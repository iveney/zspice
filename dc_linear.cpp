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

	// // for vsrc, cccs, vcvs, ccvs, allocate new rows and column
	// for vsrc, vcvs, ccvs, allocate new rows and column
	size += netlist.netset[VSRC].size();
	//size += netlist.netset[CCCS].size();
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

	// IMPORTANT: remove ground equation to make matrix definite!
	Y[0][0]=1.0;
	for(int i=1;i<size;i++) Y[i][0]=Y[0][i]=0.0;
	J[0]=0.0;

	cout<<"** Information of Y (ground included) **"<<endl;
	output_matrix(Y, size);
	cout<<"** Content of J **"<<endl;
	for(int i=0;i<size;i++) cout<<"J["<<i<<"]="<<J[i]<<endl;

	// solve matrix and output
	solve_dc(Y,J,v,size);
	output_result(netlist, nodelist, v, size);
	
	// release resourse
	delete_matrix(Y, size);
	delete [] v;
	delete [] J;
}

// output the final result, which includes 
// 1) node voltages 
// 2) currents of independent voltage sources, cccs, vcvs, ccvs
void output_result(Netlist & netlist, Nodelist & nodelist, double *v, int n){
	cout<<endl<<"** Node voltages **"<<endl;
	int i;
	for(i=1; i<nodelist.size(); i++){ // do not output ground=0
		Node & nd = nodelist.nodelist[i];
		int id = nodelist.name2idx[nd.name];
		cout<<nd.name<<": "<<scientific<<right<<v[id]<<endl;
	}

	Net net;
	//const int set_types[] = {VSRC, CCCS, VCVS, CCVS};
	const int set_types[] = {VSRC, VCVS, CCVS};
	int nn= sizeof(set_types)/sizeof(int);
	for(int i=0;i<nn;i++){
		cout<<endl<<"** branch current of "
			  <<nettype_str[set_types[i]]<<" **"<<endl;
		foreach_net_in(netlist, set_types[i], net){
			int id = net2int[net.name];
			cout<<net.name<<": "<<scientific<<right<<v[id]<<endl;
		}
	}

	// for cccs, it is controlled by the current, just compute it
	cout<<endl<<"** branch current of cccs **"<<endl;
	foreach_net_in(netlist, CCCS, net){
		int id = net2int[net.vyyy];
		cout<<net.name<<": "<<scientific<<right<<net.value*v[id]<<endl;
	}
}

// count how many non-zero entries in Y, note that we ignore row 1 and column 1
int count_entry(double **Y, int n){
	int m=0;
	for(int i=0;i<n;i++)
		for(int j=0;j<n;j++)
			if( !zero(Y[i][j]) ) ++m;
	return m;
}

// Convert the matrix to triplet form, note that we ignore row 1 and column 1
void matrix_to_triplet(int *Ti, int * Tj, double * Tx, int nz, double **Y, int n){
	int k=0;
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			if( !zero(Y[i][j]) ){
				Ti[k] = i;  
				Tj[k] = j;
				Tx[k] = Y[i][j];
				k++;
			}
		}
	}
}

// Given the matrix, solve them, the result is stored in `v'
void solve_dc(double **Y, double * J, double * v, int n){
	// convert them to triplet format
	int nz=count_entry(Y,n);
	int Ti[nz];
	int Tj[nz];
	double Tx[nz];
	matrix_to_triplet(Ti,Tj,Tx,nz,Y,n);
	
	// then convert to column compressed form 
	int n_row = n; // do not count ground row
	int n_col = n; // do not count ground column
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

	/*
	for(int i=0;i<n_col+1;i++)
		cout<<"Ap["<<i<<"]="<<Ap[i]<<endl;
			
	for(int i=0;i<nz;i++)
		cout<<"Ai["<<i<<"]="<<Ai[i]<<" Ax["<<i<<"]="<<Ax[i]<<endl;
		*/
	
	double *null = (double *) NULL;
	void *Symbolic, *Numeric;
	status = umfpack_di_symbolic(n, n, Ap, Ai, Ax, &Symbolic, Control, null) ; 
	if( status < 0 ){
		umfpack_di_report_status (Control, status) ;
		report_exit("umfpack_di_symbolic failed\n") ;
	}
	status = umfpack_di_numeric(Ap, Ai, Ax, Symbolic, &Numeric, Control, null) ;
	if( status < 0 ){
		umfpack_di_report_status (Control, status) ;
		report_exit("umfpack_di_numeric failed\n") ;
	}
	umfpack_di_free_symbolic (&Symbolic) ;
	status = umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax, v, J, Numeric, Control, null) ;
	if( status < 0 ){
		umfpack_di_report_status (Control, status) ;
		report_exit("umfpack_di_solve failed\n") ;
	}
	umfpack_di_free_numeric (&Numeric) ;

	delete [] Ax;
	delete [] Ai;
	delete [] Ap;
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

		Y[k][ct] += 1.;
		Y[ct][k] += 1.;
		Y[l][ct] += -1.;
		Y[ct][l] += -1.;
		J[ct] += net.value;
	}

	// stamp cccs
	set<string> & cccs = netlist.netset[CCCS];
	for(it = cccs.begin(); it != cccs.end(); ++it){
		Net & net = netlist[*it];
		//net2int[net.name] = ct;
		int p = nodelist.name2idx[net.nbr[0]];
		int q = nodelist.name2idx[net.nbr[1]];

		// need to find the controlling node
		string ctrl = net.vyyy;

		// need to find out where the controlling node is stamped
		int ctrl_index = net2int[ctrl];
		
		//Y[k][ct] += 1.;
		//Y[ct][k] += 1.;
		//Y[l][ct] += -1.;
		//Y[ct][l] += -1.;
		Y[p][ctrl_index] +=  net.value; // alpha
		Y[q][ctrl_index] += -net.value; // alpha
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

		Y[p][ct] += 1. ;
		Y[ct][p] += 1.;
		Y[q][ct] += -1.;
		Y[ct][q] += -1.;
		Y[ct][k] += -net.value; // mu
		Y[ct][l] +=  net.value; // mu
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

		// need to find out where the controlling node is stamped
		int ctrl_index = net2int[ctrl];

		Y[p][ct] += 1.;
		Y[ct][p] += 1.;
		Y[q][ct] += -1.;
		Y[ct][q] += -1.; 
		Y[ct][ctrl_index] += -net.value;
	}
	// stamp is over!
}
