// ----------------------------------------------------------------//
// Filename : dc_linear.cpp
// Author : Zigang Xiao <zxiao2@illinois.edu>
//
// define function for linear dc analysis
// ----------------------------------------------------------------//
// - Zigang Xiao - Mon Oct 25 22:53:38 CDT 2010
//   * rewrite dc_analysis part, incorporate NR-iteration
//   * added non-linear dc analysis : diode and bjt 
//
// - Zigang Xiao - Wed Sep 15 16:31:56 CDT 2010
//   * created file

#include <ext/hash_map>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <cassert>
#include "global.h"
#include "util.h"
#include "dc_linear.h"
#include "dc_nonlinear.h"
#include "umfpack.h"
using namespace std;
using namespace __gnu_cxx;

///////////////////////////////////////////////////////////////////////
// class definition of triplet

// Y[0][0] is always 1, hence add this when initializing
Triplet::Triplet(){
	Ti.push_back(0);
	Tj.push_back(0);
	Tx.push_back(1.0);
}

// insert a triplet 
void Triplet::push(int i, int j, double x){
	if( i==0 || j == 0 ) return;
	Ti.push_back(i);
	Tj.push_back(j);
	Tx.push_back(x);
}

// return the number of elements in a triplet instance
int Triplet::size(){return Ti.size();}

///////////////////////////////////////////////////////////////////////

// for vsrc, vcvs, ccvs, map the net name to integer index in the matrix
hash_map<string, int> net2int;

// Given netlist and nodelist, perform dc analysis
void dc_analysis(Netlist & netlist, Nodelist & nodelist, DC_TYPE dc_type){
	// create matrix, note that 0 row and column is for ground
	int size = nodelist.size();

	// for vsrc, vcvs, ccvs, allocate new rows and column
	size += netlist.netset[VSRC].size();
	size += netlist.netset[VCVS].size();
	size += netlist.netset[CCVS].size();

	// use triplet form here
	double * v = new double[size];
	double * J = new double[size];

	assert(dc_type == LINEAR || dc_type == NON_LINEAR);
	// linear dc analysis
	if( dc_type == LINEAR ){
		linear_dc(netlist,nodelist,J,v,size);
		output_result(netlist, nodelist, v, size);
	}
	// non-linear dc analysis
	else// (dc_type == NON_LINEAR)
		NR_iteration(netlist,nodelist,J,v,size);

	// release resourse
	delete [] v;
	delete [] J;
}

// perform linear dc analysis
// can be used as a sub-routine in NR-iteration
void linear_dc(Netlist & netlist, Nodelist & nodelist,
		double *J, double *v, int size){
	// stamp the matrix
	//cout<<"Stamping matrix..."<<endl;
	Triplet tri;
	memset((void*)J, 0, sizeof(double)*size);
	bool ret = stamp_matrix(netlist, nodelist, tri, J);
	if( ret == false ){
		report_exit("**** job aborted ****\n");
	}

	// this is important: cross out the ground (reference) node 
	J[0]=0.0;

	// solve matrix and output
	//cout<<"Solving the sparse matrix..."<<endl;
	solve_dc(tri, J, v, size);
}

// after dc_analysis, remember to call this function to update node voltages
void update_node_voltages(Nodelist & nodelist, double *v){
	for(int i=1;i<nodelist.size();i++){
		Node & nd = nodelist.nodelist[i];
		int id = nodelist.name2idx[nd.name];
		nd.v = v[id];
	}
}

// output the final result, which includes 
// 1) node voltages 
// 2) currents of independent voltage sources, cccs, vcvs, ccvs
void output_result(Netlist & netlist, Nodelist & nodelist, double *v, int n){
	// output node voltages
	cout<<endl<<"** Node voltages **"<<endl;
	int i;
	for(i=1; i<nodelist.size(); i++){ // do not output ground=0
		Node & nd = nodelist.nodelist[i];
		int id = nodelist.name2idx[nd.name];
#ifndef PHASE1OUTPUT
		cout<<nd.name<<": "<<scientific<<right<<v[id]<<endl;
#else
		cout<<"voltage at node "<<nd.name<<" = "<<scientific<<v[id]<<endl;
#endif
	}

	// output branch currents
	Net net;
	const int set_types[] = {VSRC, VCVS, CCVS};
	int nn= sizeof(set_types)/sizeof(int);
	for(i=0;i<nn;i++){
#ifndef PHASE1OUTPUT
		cout<<endl<<"** branch current of "
			<<nettype_str[set_types[i]]<<" **"<<endl;
#endif
		foreach_net_in(netlist, set_types[i], net){
			int id = net2int[net.name];
#ifndef PHASE1OUTPUT	
			cout<<net.name<<": "<<scientific<<right<<v[id]<<endl;
#else
			cout<<"current throught source "<<net.name<<" = "
				<<scientific<<v[id]<<endl;
#endif
		}
	}

	// for cccs, it is controlled by the current, just compute it
#ifndef PHASE1OUTPUT
	cout<<endl<<"** branch current of cccs **"<<endl;
#endif
	foreach_net_in(netlist, CCCS, net){
		int id = net2int[net.vyyy];
#ifndef PHASE1OUTPUT
		cout<<net.name<<": "<<scientific<<right<<net.value*v[id]<<endl;
#else
		cout<<"current throught source "<<net.name<<" = "
			<<scientific<<net.value*v[id]<<endl;
#endif
	}
	cout<<endl;
}

// given a vector, copy its element to a basic array
template<class T>
void vector_to_array(vector<T> v, T * arr){
	copy(v.begin(), v.end(), arr);
}

// Given the matrix, solve them, the result is stored in `v'
void solve_dc(Triplet & t, double * J, double * v, int n){
	// convert to column compressed form 
	int n_row = n; // do not count ground row
	int n_col = n; // do not count ground column
	int nz = t.size();
	int * Ap = new int [n_col+1];
	int * Ai = new int [nz];
	double * Ax = new double [nz];

	int * Ti = new int[nz];
	int * Tj = new int[nz];
	double * Tx = new double[nz];
	vector_to_array<int>(t.Ti, Ti);
	vector_to_array<int>(t.Tj, Tj);
	vector_to_array<double>(t.Tx, Tx);

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
	   cout<<"nz, n ="<<nz<<" "<<n<<endl;
	   for(int i=0;i<n;i++)
	   cout<<"J["<<i<<"]="<<J[i]<<endl;

	   for(int i=0;i<n_col+1;i++)
	   cout<<"Ap["<<i<<"]="<<Ap[i]<<endl;

	   for(int i=0;i<nz;i++)
	   cout<<"Ai["<<setw(2)<<i<<"]="<<setw(2)<<Ai[i]
	   <<" Ax["<<setw(2)<<i<<"]="<<setw(2)<<Ax[i]<<endl;
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

	clock_t t1,t2;
	t1=clock();
	// mearuse the run time
	status = umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax, v, J, Numeric, Control, null) ;
	t2=clock();
	//cout<<"Runtime = "<<1.0*(t2-t1)/CLOCKS_PER_SEC<<endl;
	if( status < 0 ){
		umfpack_di_report_status (Control, status) ;
		report_exit("umfpack_di_solve failed\n") ;
	}
	umfpack_di_free_numeric (&Numeric) ;

	delete [] Ax;
	delete [] Ai;
	delete [] Ap;
	delete [] Ti;
	delete [] Tj;
	delete [] Tx;
}

// stamp linear devices
bool stamp_linear(Netlist & netlist, Nodelist & nodelist, 
		Triplet & t, double * J){
	bool success = true;
	Net net;

	// stamp resistor
	foreach_net_in(netlist, RSTR, net){
		int i = nodelist.name2idx[net.nbr[0]];
		int j = nodelist.name2idx[net.nbr[1]];
		double G = 1./net.value;
		t.push(i,i,G);
		t.push(j,j,G);
		t.push(i,j,-G);
		t.push(j,i,-G);

	}

	// stamp current source
	foreach_net_in(netlist, CSRC, net){
		int i = nodelist.name2idx[net.nbr[0]];
		int j = nodelist.name2idx[net.nbr[1]];
		J[i] += -net.value;
		J[j] +=  net.value;
	}

	// stamp vccs
	foreach_net_in(netlist, VCCS, net){
		int p = nodelist.name2idx[net.nbr[0]];
		int q = nodelist.name2idx[net.nbr[1]];
		int k = nodelist.name2idx[net.ctrl[0]];
		int l = nodelist.name2idx[net.ctrl[1]];

		t.push(p,k,net.value);
		t.push(q,l,net.value);
		t.push(p,l,-net.value);
		t.push(q,k,-net.value);
	}

	// the following nets will use extra space in augmented Y
	int ct = nodelist.size();  // counter

	// stamp voltage source, NOTE the counter
	foreach_net_in(netlist, VSRC, net){
		net2int[net.name] = ct;
		int k = nodelist.name2idx[net.nbr[0]];
		int l = nodelist.name2idx[net.nbr[1]];

		t.push(k,ct,1.);
		t.push(ct,k,1.);
		t.push(l,ct,1.);
		t.push(ct,l,1.);
		J[ct] += net.value;
		++ct;
	}

	// stamp cccs
	foreach_net_in(netlist, CCCS, net){
		int p = nodelist.name2idx[net.nbr[0]];
		int q = nodelist.name2idx[net.nbr[1]];

		// need to find the controlling node
		string ctrl = net.vyyy;
		// check if it exists
		if( netlist.netlist.find(ctrl) == netlist.netlist.end() ){
			cerr<<"Error: dependent voltage source ["<<ctrl
				<<"] of cccs ["<<net.name<<"] not present!"<<endl;
			success = false;
			continue;
		}

		// need to find out where the controlling node is stamped
		int ctrl_index = net2int[ctrl];

		t.push(p,ctrl_index, net.value); // alpha
		t.push(q,ctrl_index,-net.value); // alpha
	}

	// stamp vcvs, need counter
	foreach_net_in(netlist, VCVS, net){
		net2int[net.name] = ct;
		int p = nodelist.name2idx[net.nbr[0]];
		int q = nodelist.name2idx[net.nbr[1]];
		int k = nodelist.name2idx[net.ctrl[0]];
		int l = nodelist.name2idx[net.ctrl[1]];

		t.push(p,ct, 1.);
		t.push(ct,p, 1.);
		t.push(q,ct,-1.);
		t.push(ct,q,-1.);
		t.push(ct,k,-net.value); // mu
		t.push(ct,l, net.value); // mu
		ct++;
	}

	// stamp ccvs, add only one row and column, NEED counter
	foreach_net_in(netlist, CCVS, net){
		net2int[net.name] = ct;
		int p = nodelist.name2idx[net.nbr[0]];
		int q = nodelist.name2idx[net.nbr[1]];

		// need to find the controlling node
		string ctrl = net.vyyy;
		// check if it exists
		if( netlist.netlist.find(ctrl) == netlist.netlist.end() ){
			cerr<<"Error: dependent voltage source ["<<ctrl
				<<"] of ccvs ["<<net.name<<"] not present!"<<endl;
			success = false;
			continue;
		}

		// need to find out where the controlling node is stamped
		int ctrl_index = net2int[ctrl];

		t.push(p,ct, 1.);
		t.push(ct,p, 1.);
		t.push(q,ct,-1.);
		t.push(ct,q,-1.);
		t.push(ct,ctrl_index,-net.value);
		++ct;
	}
	return success;
}

// stamp the non-linear devices
bool stamp_nonlinear(Netlist & netlist, Nodelist & nodelist, 
		Triplet & t, double * J){
	Net net;

	bool success = true;
	// ** linearize non-linear devices: Diode **
	foreach_net_in(netlist, DIODE, net){
		int k = nodelist.name2idx[net.nbr[0]];
		int l = nodelist.name2idx[net.nbr[1]];
		Node & nd1 = nodelist[net.nbr[0]];
		Node & nd2 = nodelist[net.nbr[1]];
		double Vd = nd1.v - nd2.v;
		double e = exp(Vd/Vt);
		double Geq = Is * e / Vt;
		double Ieq = Is * (e-1.0) - Vd*Geq;
		t.push(k,k,Geq);
		t.push(l,l,Geq);
		t.push(k,l,-Geq);
		t.push(l,k,-Geq);
		J[k] += -Ieq;
		J[l] += Ieq;
	}

	// ** linearize non-linear devices: Diode **
	//foreach_net_in(netlist, DIODE, net){
	//}
	return success;
}

// stamp the matrix according to the elements (nets)
// the index of nodes in the matrix are the same as order in nodelist
// NOTE: error will be checked here
bool stamp_matrix(Netlist & netlist, Nodelist & nodelist, 
		Triplet & t, double * J){
	bool success = stamp_linear(netlist, nodelist, t, J);
	if(!success) return false;
	stamp_nonlinear(netlist, nodelist, t, J);
	return success;
}
