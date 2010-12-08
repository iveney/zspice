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
double NR_iteration(Netlist & netlist, Nodelist & nodelist,
		double *v, double *J, int size, bool output);
double voltage_diff(double * Vnew, double * Vold, int n);
void copy_voltages(Nodelist & nodelist, double * vs);
bool stamp_nonlinear(Netlist & netlist, Nodelist & nodelist, 
		Triplet & t, double * J);
void update_BJT_currents(Netlist & netlist, Nodelist & nodelist);
