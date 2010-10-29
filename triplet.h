// ----------------------------------------------------------------//
// Filename : triplet.h
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// header file of Triplet class, this is used for solving sparse 
// matrix
// ----------------------------------------------------------------//
// - Zigang Xiao - Fri Oct 29 16:44:11 CDT 2010
//   * created file

#ifndef __TRIPLET_H__
#define __TRIPLET_H__

#include <vector>
using std::vector;

class Triplet{
public:
	Triplet();
	void merge();
	void push(int i,int j,double x);
	int size();
	vector<int> Ti;
	vector<int> Tj;
	vector<double> Tx;
};

#endif
