// ----------------------------------------------------------------//
// Filename : global.h
// Author : Zigang Xiao <zxiao2@illinois.edu>
//
// Global macros and constants
// ----------------------------------------------------------------//
// - Zigang Xiao - Mon Sep 13 12:54:13 CDT 2010
//   * first create the file

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

static const int BUFSIZE=256;
enum NODETYPE{RSTR, VSRC, CSRC, VCCS, VCVS, CCVS, CCCS, DIODE, 
	BJT, CAPCT, INDCT, UNDEF}; // the last type is undef
const char * const nettype_str[]={
	"resistor", 
	"voltage source", 
	"current source",
	"vccs", "vcvs",
	"ccvs", "cccs",
	"diode", "bjt",
	"capacitor",
	"inductor"
};
// types of nets
static const int NUM_NETTYPE=sizeof(nettype_str)/sizeof(char*);
enum POLARITY{NPN, PNP};
static const double EPSILON = 1E-9;

#define abs(a) ((a)>0?(a):-(a))
#define zero(a) (abs(a)<EPSILON)

#endif
