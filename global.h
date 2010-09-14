// ----------------------------------------------------------------//
// Filename : global.h
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// Global macros and constants
// ----------------------------------------------------------------//
// - Zigang Xiao - Mon Sep 13 12:54:13 CDT 2010
//   * first create the file

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

const int BUFSIZE=256;
enum NODETYPE{UNDEF, RSTR, VSRC, CSRC, VCCS, VCVS, CCVS, CCCS, DIODE, 
	BJT, CAPCT, INDCT};
enum POLARITY{NPN, PNP};

#endif
