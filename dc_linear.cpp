// ----------------------------------------------------------------//
// Filename : dc_linear.cpp
// Author : Zigang Xiao <zxiao2@illinois.edu>
//
// define function for linear dc analysis
// ----------------------------------------------------------------//
// - Zigang Xiao - Wed Sep 15 16:31:56 CDT 2010
//   * created file

#include <ext/hash_map>
#include <cstdlib>
#include "global.h"
#include "util.h"
#include "dc_linear.h"
#include "umfpack.h"
using namespace std;
using namespace __gnu_cxx;

hash_map<string, int> net2int;

// Given netlist and nodelist, perform dc analysis
void linear_dc_analysis(Netlist & netlist, Nodelist & nodelist){
	// create matrix, note that 0 row and column is for ground
	int size = nodelist.size();

	// for vsrc, cccs, vcvs, ccvs, allocate new rows and column
	size += netlist.netset[VSRC].size();
	size += netlist.netset[CCCS].size();
	size += netlist.netset[VCVS].size();
	size += netlist.netset[CCVS].size();
#ifdef 	DEBUG
	cout<<"matrix size = "<<size<<endl;
#endif
	double ** Y = new_square_matrix(size);
	double * J = new double[size];
	double * v = new double[size];
	memset(J, 0, sizeof(double)*size);

	// stamp the matrix
	stamp_matrix(netlist, nodelist, Y, J);
	cout<<"Information of Y and J (ground included)"<<endl;
	output_matrix(Y, size);
	for(int i=0;i<size;i++) cout<<"J["<<i<<"]="<<J[i]<<endl;

	// solve matrix
	solve_dc(Y,J,v,size);
	
	// release resourse
	delete_matrix(Y, size);
	delete [] v;
	delete [] J;
}

// count how many non-zero entries in Y, note that we ignore row 1 and column 1
int count_entry(double **Y, int n){
	int m=0;
	for(int i=1;i<n;i++){
		for(int j=1;j<n;j++){
			if( !zero(Y[i][j]) ) ++m;
		}
	}
	return m;
}

// Convert the matrix to triplet form, note that we ignore row 1 and column 1
void matrix_to_triplet(int *Ti, int * Tj, double * Tx, int nz, double **Y, int n){
	int k=0;
	for(int i=1;i<n;i++){
		for(int j=1;j<n;j++){
			if( !zero(Y[i][j]) ){
				Ti[k] = i-1;  // move the index one step forward
				Tj[k] = j-1;
				Tx[k] = Y[i][j];
				k++;
			}
		}
	}
}

// Given the matrix, solve them, the result is stored in `v'
void solve_dc(double **Y, double * J, double * v, int n){
	// first need to convert them to triplet format
	int nz=count_entry(Y,n);
	int Ti[nz];
	int Tj[nz];
	double Tx[nz];
	matrix_to_triplet(Ti,Tj,Tx,nz,Y,n);
	
	// then convert to column compressed form 
	int nn = n-1;    // new size
	int n_row = nn; // do not count ground row
	int n_col = nn; // do not count ground column
	int * Ap = new int [n_col+1];
	int * Ai = new int [nz];
	double * Ax = new double [nz];
	int status;
	double Control [UMFPACK_CONTROL];
	umfpack_di_defaults (Control) ;
	status = umfpack_di_triplet_to_col(n_row, n_col, nz, Ti, Tj, Tx, 
			Ap, Ai, Ax, (int *) NULL);

	if( status < 0 ) {
		umfpack_di_report_status (Control, status) ;
		report_exit("umfpack_zi_triplet_to_col failed\n") ;
	}

	// for J, we also need to remove the 1st element
	double * JJ = new double[nn];
	// start from J[1]
	memcpy((void*)JJ,(void*)(J+1),sizeof(double)*(nn));

	/*
	for(int i=0;i<n_col+1;i++)
		cout<<"Ap["<<i<<"]="<<Ap[i]<<endl;
			
	for(int i=0;i<nz;i++)
		cout<<"Ai["<<i<<"]="<<Ai[i]<<" Ax["<<i<<"]="<<Ax[i]<<endl;
		*/

	
	double *null = (double *) NULL;
	void *Symbolic, *Numeric;
	status = umfpack_di_symbolic (nn, nn, Ap, Ai, Ax, &Symbolic, Control, null) ; 
	if( status < 0 ){
		umfpack_di_report_status (Control, status) ;
		report_exit("umfpack_di_symbolic failed\n") ;
	}
	status= umfpack_di_numeric (Ap, Ai, Ax, Symbolic, &Numeric, Control, null) ;
	if( status < 0 ){
		umfpack_di_report_status (Control, status) ;
		report_exit("umfpack_di_numeric failed\n") ;
	}
	umfpack_di_free_symbolic (&Symbolic) ;
	(void) umfpack_di_solve (UMFPACK_A, Ap, Ai, Ax, v, JJ, Numeric, Control, null) ;
	if( status < 0 ){
		umfpack_di_report_status (Control, status) ;
		report_exit("umfpack_di_solve failed\n") ;
	}
	umfpack_di_free_numeric (&Numeric) ;

	for(int i=0;i<nn;i++)
		cout<<"v["<<i<<"] = "<<v[i]<<endl;

	delete [] Ax;
	delete [] Ai;
	delete [] Ap;
	delete [] JJ;
}

// stamp the matrix according to the elements (nets)
// the index of nodes in the matrix are the same as their stored order in nodelist
void stamp_matrix(Netlist & netlist, Nodelist & nodelist, double ** Y, double * J){
	set<string>::iterator it;

	// stamp resistor
	set<string> & rset = netlist.netset[RSTR];
	for(it = rset.begin(); it != rset.end(); ++it){
		Net & net = netlist[*it];
		int i = nodelist.name2idx[net.nbr[0]];
		int j = nodelist.name2idx[net.nbr[1]];
		double G = 1./net.value;
		Y[i][i] +=  G;
		Y[j][j] +=  G;
		Y[i][j] += -G;
		Y[j][i] += -G;
	}

	// stamp current source
	set<string> & cset = netlist.netset[CSRC];
	for(it = cset.begin(); it != cset.end(); ++it){
		Net & net = netlist[*it];
		int i = nodelist.name2idx[net.nbr[0]];
		int j = nodelist.name2idx[net.nbr[1]];
		J[i] += -net.value;
		J[j] +=  net.value;
	}
	
	// stamp vccs
	set<string> & vccs = netlist.netset[VCCS];
	for(it = vccs.begin(); it != vccs.end(); ++it){
		Net & net = netlist[*it];
		int p = nodelist.name2idx[net.nbr[0]];
		int q = nodelist.name2idx[net.nbr[1]];
		int k = nodelist.name2idx[net.ctrl[0]];
		int l = nodelist.name2idx[net.ctrl[1]];
		Y[p][k] +=  net.value;
		Y[q][l] +=  net.value;
		Y[p][l] += -net.value;
		Y[q][k] += -net.value;
	}

	// the following nets will use extra space in augmented Y
	int ct = nodelist.size();  // counter

	// stamp voltage source
	set<string> & vsrc = netlist.netset[VSRC];
	for(it = vsrc.begin(); it != vsrc.end(); ++it, ++ct){
		Net & net = netlist[*it];
		net2int[net.name] = ct;
		int k = nodelist.name2idx[net.nbr[0]];
		int l = nodelist.name2idx[net.nbr[1]];
		Y[k][ct] = Y[ct][k] =  1.;
		Y[l][ct] = Y[ct][l] = -1.;
		J[ct] = net.value;
	}

	// stamp cccs
	set<string> & cccs = netlist.netset[CCCS];
	for(it = cccs.begin(); it != cccs.end(); ++it, ++ct){
		Net & net = netlist[*it];
		net2int[net.name] = ct;
		int p = nodelist.name2idx[net.nbr[0]];
		int q = nodelist.name2idx[net.nbr[1]];
		// need to find the controlling node
		string ctrl = net.vyyy;
		string ctrl1 = netlist[ctrl].nbr[0];
		string ctrl2 = netlist[ctrl].nbr[1];
		int k = nodelist.name2idx[ctrl1];
		int l = nodelist.name2idx[ctrl2];

		Y[k][ct] = Y[ct][k] =  1.;
		Y[l][ct] = Y[ct][l] = -1.;
		Y[p][ct] =   net.value; // alpha
		Y[q][ct] = - net.value; // alpha
		//J[ct] = 0;
	}

	// stamp vcvs
	set<string> & vcvs = netlist.netset[VCVS];
	for(it = vcvs.begin(); it != vcvs.end(); ++it, ++ct){
		Net & net = netlist[*it];
		net2int[net.name] = ct;
		int p = nodelist.name2idx[net.nbr[0]];
		int q = nodelist.name2idx[net.nbr[1]];
		int k = nodelist.name2idx[net.ctrl[0]];
		int l = nodelist.name2idx[net.ctrl[1]];

		Y[p][ct] = Y[ct][p] =  1.;
		Y[q][ct] = Y[ct][q] = -1.;
		Y[ct][k] = -net.value; // mu
		Y[ct][l] =  net.value; // mu
		//J[ct] = 0;
	}

	// stamp ccvs, add only one row and column!
	set<string> & ccvs = netlist.netset[CCVS];
	for(it = ccvs.begin(); it != ccvs.end(); ++it, ++ct){
		Net & net = netlist[*it];
		net2int[net.name] = ct;
		int p = nodelist.name2idx[net.nbr[0]];
		int q = nodelist.name2idx[net.nbr[1]];
		
		// need to find the controlling node
		string ctrl = net.vyyy;
		string ctrl1 = netlist[ctrl].nbr[0];
		string ctrl2 = netlist[ctrl].nbr[1];

		// need to find out where the controlling node is stamped
		int ctrl_index = net2int[ctrl];

		//Y[k][ct] = Y[ct][k] = Y[l][ct] = Y[ct][l] = 0; 
		Y[p][ct] = Y[ct][p] =  1.;
		Y[q][ct] = Y[ct][q] = -1.; 
		Y[ct][ctrl_index] = -net.value;
	}
	// stamp is over!
}
