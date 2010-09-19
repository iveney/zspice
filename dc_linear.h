// ----------------------------------------------------------------//
// Filename : dc_linear.h
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// header: function for linear dc analysis
// ----------------------------------------------------------------//
// - Zigang Xiao - Wed Sep 15 16:31:56 CDT 2010
//   * created file

#ifndef __DC_LINEAR__
#define __DC_LINEAR__
#include <vector>
#include "node.h"
#include "net.h"
using std::vector;

class Triplet{
public:
	Triplet();
	inline void push(int i,int j,double x);;
	int size();
	vector<int> Ti;
	vector<int> Tj;
	vector<double> Tx;
};

void Triplet::push(int i, int j, double x){
	if( i==0 || j == 0 ) return;
	Ti.push_back(i);
	Tj.push_back(j);
	Tx.push_back(x);
}

void linear_dc_analysis(Netlist & netlist, Nodelist & nodelist);
bool stamp_matrix(Netlist & netlist, Nodelist & nodelist, Triplet &, double *);
void solve_dc(Triplet & t, double * J, double * v, int n);
void output_result(Netlist &, Nodelist & , double *v, int n);

template<class T>
void vector_to_array(vector<T> v, T * arr);

#endif
