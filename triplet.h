// ----------------------------------------------------------------//
// Filename : triplet.h
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// header file of Triplet class, this is used for solving sparse 
// matrix
// ----------------------------------------------------------------//
// - Zigang Xiao - Fri Oct 29 16:44:11 CDT 2010
//   * created file
// - Zigang Xiao - Tue Dec  7 21:57:13 CST 2010
//   * modified to adapt complex number

#ifndef __TRIPLET_H__
#define __TRIPLET_H__

#include <vector>
using std::vector;

class Triplet{
public:
	Triplet();
	void merge();
	void push(int i,int j,double x,double z=.0);
	int size();
	void output();

	vector<int> Ti;
	vector<int> Tj;
	vector<double> Tx;
	vector<double> Tz;
};

#endif
