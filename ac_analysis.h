// ----------------------------------------------------------------//
// Filename : net.cpp
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// header file of ac_analysis.cpp 
// ----------------------------------------------------------------//
// - Zigang Xiao - Tue Dec  7 12:42:11 CST 2010
//   * first create the file

#ifndef __AC_ANALYSIS_H
#define __AC_ANALYSIS_H

#include "net.h"
#include "node.h"
double compute_Cc(double Vc, POLARITY pol);
double compute_Cbe(double Vbe, POLARITY pol);
double compute_Cbc(double Vbc, POLARITY pol);
void compute_BJT_cap(Net & net, Nodelist & nodelist);
void compute_caps(Netlist & netlist, Nodelist & nodelist);
void ac_analysis(Netlist & netlist, Nodelist & nodelist);
#endif
