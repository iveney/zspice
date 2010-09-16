// ----------------------------------------------------------------//
// Filename : dc_linear.cpp
// Author : Xiao Zigang <zxiao2@illinois.edu>
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
	output_matrix(Y, size);

	// solve matrix
	solve_dc(Y,J,v,size);
	
	// release resourse
	delete_matrix(Y, size);
	delete [] v;
	delete [] J;
}

void solve_dc(double **Y, double * J, double * v, int n){
	double *null = (double *) NULL;
	void *Symbolic, *Numeric;
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
		int k = nodelist.name2idx[ctrl1];
		int l = nodelist.name2idx[ctrl2];

		// need to find out where the controlling node is stamped
		int ctrl_index = net2int[ctrl];

		//Y[k][ct] = Y[ct][k] = Y[l][ct] = Y[ct][l] = 0; 
		Y[p][ct] = Y[ct][p] =  1.;
		Y[q][ct] = Y[ct][q] = -1.; 
		Y[ct][ctrl_index] = -net.value;
	}
	// stamp is over!
}
