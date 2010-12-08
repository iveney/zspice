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
#include "triplet.h"
using std::vector;

void dc_analysis(Netlist & netlist, Nodelist & nodelist, bool output);
bool stamp_matrix(Netlist & netlist, Nodelist & nodelist, Triplet &, 
		double *, ANALYSIS_TYPE);
void solve_dc(Triplet & t, double * v, double * J, int n);
void output_result(Netlist &, Nodelist & , double *v, int n);
void dc_core(Netlist & netlist, Nodelist & nodelist,
		double *v, double *J, int size);
void update_node_voltages(Nodelist & nodelist, double *v);
bool stamp_linear(Netlist & netlist, Nodelist & nodelist, 
		Triplet & t, double * J, ANALYSIS_TYPE atype);

bool stamp_resistor(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double * J, ANALYSIS_TYPE atype);

bool stamp_vccs(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double * J, ANALYSIS_TYPE atype);

bool stamp_vsrc(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double * J, ANALYSIS_TYPE atype, int & ct);

bool stamp_cccs(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double * J, ANALYSIS_TYPE atype);

#endif
