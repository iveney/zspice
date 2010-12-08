// ----------------------------------------------------------------//
// Filename : dc_nonlinear.cpp
// Author : Zigang Xiao <zxiao2@illinois.edu>
//
// Function headers for non-linear dc analysis
// ----------------------------------------------------------------//
// - Zigang Xiao - Mon Oct 25 12:45:28 CDT 2010
//   * first create the file
//
#ifndef __DC_NONLINEAR_H__
#define __DC_NONLINEAR_H__
#include "dc_linear.h"

// performs newton-raphson iteraion
double NR_iteration(Netlist & netlist, Nodelist & nodelist,
		double *v, double *J, int size, bool output);
double voltage_diff(double * Vnew, double * Vold, int n);
void copy_voltages(Nodelist & nodelist, double * vs);
bool stamp_nonlinear(Netlist & netlist, Nodelist & nodelist, 
		Triplet & t, double * J);
void update_BJT_currents(Netlist & netlist, Nodelist & nodelist);

void stamp_diode(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double * J);

void stamp_capacitor(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double f, ANALYSIS_TYPE atype);

void stamp_inductor(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double f, ANALYSIS_TYPE atype);

void stamp_BJT_AC(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double f);

void stamp_BJT_DC(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double * J);

#endif
