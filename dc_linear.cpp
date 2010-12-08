// ----------------------------------------------------------------//
// Filename : dc_linear.cpp
// Author : Zigang Xiao <zxiao2@illinois.edu>
//
// define function for linear dc analysis
// ----------------------------------------------------------------//
// - Zigang Xiao - Fri Oct 29 16:41:52 CDT 2010
//   * Finish writing newton iteration part
//   * added non-linear dc analysis : bjt 
//   * separate triplet to a new cpp/h
//
// - Zigang Xiao - Mon Oct 25 22:53:38 CDT 2010
//   * rewrite dc_analysis part, incorporate NR-iteration
//   * added non-linear dc analysis : diode
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
#include "triplet.h"
#include "util.h"
#include "dc_linear.h"
#include "dc_nonlinear.h"
#include "umfpack.h"
using namespace std;
using namespace __gnu_cxx;

// for vsrc, vcvs, ccvs, map the net name to integer index in the matrix
hash_map<string, int> net2int;

// Given netlist and nodelist, perform dc analysis
void dc_analysis(Netlist & netlist, Nodelist & nodelist, bool output){
	// create matrix, note that 0 row and column is for ground
	int size = nodelist.size();

	// for vsrc, vcvs, ccvs, allocate new rows and column
	size += netlist.netset[VSRC].size();
	size += netlist.netset[VCVS].size();
	size += netlist.netset[CCVS].size();

	// use triplet form here
	double * v = new double[size];
	double * J = new double[size];
	memset((void*)J, 0, sizeof(double)*size);
	memset((void*)v, 0, sizeof(double)*size);

	if( netlist.netset[DIODE].size() + netlist.netset[BJT].size() == 0) {
		// No non-linear device: linear dc analysis
		dc_core(netlist,nodelist,v,J,size);
		//output_result(netlist, nodelist, v, size);
		update_node_voltages(nodelist, v);
		if( output == true){
			nodelist.output_node_voltages();
			netlist.output_branch_currents(net2int, v);
		}
	}
	else{ // non-linear dc analysis
		NR_iteration(netlist,nodelist,v,J,size,output);
	}

	// release resourse
	delete [] v;
	delete [] J;
}

// perform dc analysis
// can be used as a sub-routine in NR-iteration
void dc_core(Netlist & netlist, Nodelist & nodelist,
		double *v, double *J, int size){
	// stamp the matrix
	//cout<<"Stamping matrix..."<<endl;
	Triplet tri;
	memset((void*)J, 0, sizeof(double)*size);
	memset((void*)v, 0, sizeof(double)*size);
	bool ret = stamp_matrix(netlist, nodelist, tri, J, DC);
	if( ret == false ) report_exit("**** job aborted ****\n");

	// this is important: cross out the ground (reference) node 
	J[0]=0.0;

	// DEBUG: output J;
	// for(int i=0;i<size;i++) cout<<i<<" "<<J[i]<<endl;

	// solve matrix and output
	//cout<<"Solving the sparse matrix..."<<endl;
	solve_dc(tri, v, J, size);
}

// after dc_analysis, remember to call this function to update node voltages
// Note: damp here
void update_node_voltages(Nodelist & nodelist, double *v){
	for(int i=1;i<nodelist.size();i++){
		Node & nd = nodelist.nodelist[i];
		int id = nodelist.name2idx[nd.name];
		/*
		if( abs(nd.v - v[id]) > DAMPEN ){
			if( nd.v - v[id] > 0 )
				nd.v -= DAMPEN;
			else 
				nd.v += DAMPEN;
		}
		else{
			nd.v = v[id];
		}
		*/
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

	// output branch currents of Voltage source, VCVS and CCVS
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

	// for CCCS, it is controlled by some vyyy, then just compute it
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

// Given the matrix, solve them, the result is stored in `v'
void solve_dc(Triplet & t, double * v, double * J, int n){
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

	// DEBUG: output triplet
	//t.merge();
	//for(int i=0;i<t.size();i++){
		//cout<<t.Ti[i]<<" "<<t.Tj[i]<<" : "<<t.Tx[i]<<endl;
	//}

	int status;
	double Control [UMFPACK_CONTROL];
	umfpack_di_defaults (Control) ;
	status = umfpack_di_triplet_to_col(n_row, n_col, nz, Ti, Tj, Tx, 
			Ap, Ai, Ax, (int *) NULL);

	//status = umfpack_di_triplet_to_col(n_row, n_col, nz, Ti, Tj, Tx, 
	//		Ap, Ai, Ax, (int *) NULL);

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

bool stamp_resistor(Netlist & netlist, Nodelist & nodelist,
		Triplet & t){
	Net net;
	// stamp resistor
	foreach_net_in(netlist, RSTR, net){
		int i = nodelist.name2idx[net.nbr[0]];
		int j = nodelist.name2idx[net.nbr[1]];
		// nothing to do with imaginary part and J
		double G = 1./net.value;
		t.push(i,i, G);
		t.push(j,j, G);
		t.push(i,j,-G);
		t.push(j,i,-G);
	}
	return true;
}

bool stamp_csrc(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double * J, ANALYSIS_TYPE atype){
	Net net;
	// stamp current source
	foreach_net_in(netlist, CSRC, net){
		double value = net.value;
		int i = nodelist.name2idx[net.nbr[0]];
		int j = nodelist.name2idx[net.nbr[1]];
		if( atype == DC ){ 
			J[i] += -value;
			J[j] +=  value;
		}
		// for AC, open circuit, i.e., I=0
	}
	return true;
}

bool stamp_vccs(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double * J, ANALYSIS_TYPE atype){
	// stamp vccs
	Net net;
	foreach_net_in(netlist, VCCS, net){
		// for AC, open circuit, i.e., I=0, do nothing
		if( atype == AC ) continue;
		int p = nodelist.name2idx[net.nbr[0]];
		int q = nodelist.name2idx[net.nbr[1]];
		int k = nodelist.name2idx[net.ctrl[0]];
		int l = nodelist.name2idx[net.ctrl[1]];

		t.push(p,k,net.value);
		t.push(q,l,net.value);
		t.push(p,l,-net.value);
		t.push(q,k,-net.value);
		// nothing to do w/ J
	}
	return true;
}

bool stamp_vsrc(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double * J, 
		ANALYSIS_TYPE atype, int & ct){
	Net net;
	// stamp voltage source, NOTE the counter
	foreach_net_in(netlist, VSRC, net){
		// we need to identify the analysis type
		double value = net.value;
		if(net.vtype == AC && atype == DC ||
		   net.vtype == DC && atype == AC) {
			//    AC voltage source in DC analysis 
			// or DC voltage source in AC analysis 
			value = 0.0;
		}
		net2int[net.name] = ct;
		int k = nodelist.name2idx[net.nbr[0]];
		int l = nodelist.name2idx[net.nbr[1]];

		// NOTE: stamp initial value
		t.push(k,ct,1.);
		t.push(ct,k,1.);
		t.push(l,ct,-1.);
		t.push(ct,l,-1.);
		//cout<<"stamping v "<<net.value<<" to "<<ct<<endl;
		J[ct] += value;  // Vkl
		++ct;
	}
	return true;
}


bool stamp_cccs(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double * J, ANALYSIS_TYPE atype){
	bool success = true;
	Net net;
	// stamp cccs
	foreach_net_in(netlist, CCCS, net){
		if( atype == AC ) continue;
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
	return success;
}

bool stamp_vcvs(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double * J, ANALYSIS_TYPE atype, int & ct){
	Net net;
	// stamp vcvs, need counter
	foreach_net_in(netlist, VCVS, net){
		double value = net.value;
		if(net.vtype == AC && atype == DC ||
		   net.vtype == DC && atype == AC) {
			//    AC voltage source in DC analysis 
			// or DC voltage source in AC analysis 
			value = 0.0;
		}
		net2int[net.name] = ct;
		int p = nodelist.name2idx[net.nbr[0]];
		int q = nodelist.name2idx[net.nbr[1]];
		int k = nodelist.name2idx[net.ctrl[0]];
		int l = nodelist.name2idx[net.ctrl[1]];

		// NOTE: stamp initial value
		t.push(p,ct, 1.);
		t.push(ct,p, 1.);
		t.push(q,ct,-1.);
		t.push(ct,q,-1.);
		t.push(ct,k,-value); // mu
		t.push(ct,l, value); // mu
		ct++;
	}
	return true;
}

bool stamp_ccvs(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double * J, ANALYSIS_TYPE atype, int & ct){
	bool success=true;
	Net net;
	// stamp ccvs, add only one row and column, NEED counter
	foreach_net_in(netlist, CCVS, net){
		double value = net.value;
		if(net.vtype == AC && atype == DC ||
		   net.vtype == DC && atype == AC) {
			//    AC voltage source in DC analysis 
			// or DC voltage source in AC analysis 
			value = 0.0;
		}

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

		// NOTE: stamp initial value
		t.push(p,ct, 1.);
		t.push(ct,p, 1.);
		t.push(q,ct,-1.);
		t.push(ct,q,-1.);
		t.push(ct,ctrl_index,-value);
		++ct;
	}
	return success;
}

// stamp linear devices
bool stamp_linear(Netlist & netlist, Nodelist & nodelist, 
		Triplet & t, double * J, ANALYSIS_TYPE atype){
	bool success = true;
	Net net;

	stamp_resistor(netlist, nodelist, t);
	stamp_csrc(netlist, nodelist, t, J, atype);
	stamp_vccs(netlist, nodelist, t, J, atype);
	
	// the following nets will use extra space in augmented Y
	int ct = nodelist.size();  // counter

	stamp_vsrc(netlist, nodelist, t, J, atype, ct);
	success &= stamp_cccs(netlist, nodelist, t, J, atype);
	stamp_vcvs(netlist, nodelist, t, J, atype, ct);
	success &= stamp_ccvs(netlist, nodelist, t, J, atype, ct);

	return success;
}

// stamp the matrix according to the elements (nets)
// the index of nodes in the matrix are the same as order in nodelist
// NOTE: error will be checked here
bool stamp_matrix(Netlist & netlist, Nodelist & nodelist, 
		Triplet & t, double * J, ANALYSIS_TYPE atype){
	bool success = stamp_linear(netlist, nodelist, t, J, atype);
	if(!success) return false;
	stamp_nonlinear(netlist, nodelist, t, J);
	return success;
}
