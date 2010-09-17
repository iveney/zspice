// ----------------------------------------------------------------//
// Filename : util.cpp
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// Source code for some utility functions
// ----------------------------------------------------------------//

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

// report an error string and exit the program
// Note that it is wrapped by a macro `report_exit'
// Do NOT use this directly.
void _report_exit(const char *location, const char *msg){
#ifdef DEBUG
	  fprintf(stderr,"Error at %s: %s", location, msg);
#else
	  fprintf(stderr,"%s", msg);
#endif
	  exit(1);
}

// dynamically allocate a 2D square array of size n
double ** new_square_matrix(int n){
	double **m = new double *[n];
	int i,j;
	for(i=0;i<n;i++){
		m[i] = new double [n];
		for(j=0;j<n;j++) // initialize as 0
			m[i][j]=0.0;
	}
	return m;
}

// delete the allocated space from new_square_matrix
void delete_matrix(double ** m, int n){
	for(int i=0;i<n;i++) delete m[i];
	delete [] m;
}

void output_matrix(double ** m,int n){
	int precision = 4;
	int width = precision+8;
	cout.precision(precision);
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			cout<<scientific<<setw(width)<<m[i][j];
		}
		cout<<endl;
	}
	cout<<endl;
}
