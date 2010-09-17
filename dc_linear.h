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
#include "node.h"
#include "net.h"

void linear_dc_analysis(Netlist & netlist, Nodelist & nodelist);
void stamp_matrix(Netlist & netlist, Nodelist & nodelist, double **, double *);
void solve_dc(double **Y, double * J, double * v, int n);
int count_entry(double **Y, int n);
void matrix_to_triplet(int *Ti, int * Tj, double * Tx, int nz, double **Y, int n);
void output_result(Netlist &, Nodelist & , double *v, int n);

#endif
