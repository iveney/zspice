// ----------------------------------------------------------------//
// Filename : dc_nonlinear.cpp
// Author : Zigang Xiao <zxiao2@illinois.edu>
//
// Function headers for non-linear dc analysis
// ----------------------------------------------------------------//
// - Zigang Xiao - Mon Oct 25 12:45:28 CDT 2010
//   * first create the file
//
#include "dc_linear.h"

// performs newton-raphson iteraion
void NR_iteration(Netlist & netlist, Nodelist & nodelist,
		double *v, double *J, int size);
double voltage_diff(double * Vnew, double * Vold, int n);
void copy_voltages(Nodelist & nodelist, double * vs);
