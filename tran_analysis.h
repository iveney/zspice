// ----------------------------------------------------------------//
// Filename : tran_analysis.h
// Author : Zigang Xiao <zxiao2@illinois.edu>
//
// Header file of transient analysis
// ----------------------------------------------------------------//
// - Zigang Xiao - Fri Dec 10 00:16:37 CST 2010
//   * First create the file

#ifndef __TRAN_ANALYSIS_H__
#define __TRAN_ANALYSIS_H__
#include "node.h"
#include "net.h"

void transient_analysis(Netlist & netlist,Nodelist &nodelist);
#endif
