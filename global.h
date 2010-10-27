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

enum NODETYPE{RSTR, VSRC, CSRC, VCCS, VCVS, CCVS, CCCS, DIODE, 
	BJT, CAPCT, INDCT, UNDEF}; // the last type is undef
enum POLARITY{NPN, PNP};
enum VOL_TYPE{DC,AC,SIN};
enum DC_TYPE{LINEAR,NON_LINEAR};
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

static const int BUFSIZE=256;
// types of nets
static const int NUM_NETTYPE=sizeof(nettype_str)/sizeof(char*);
// tolerable difference
static const double EPSILON = 1E-10;

// constants for non-linear devices
static const double Vt = 0.02585126075417;
static const double Is = 10E-15;
static const double TAO = 2E10-11;
static const double Cj = 10E-14;
static const double Vj = 0.8;
static const double Fc = 0.5;
static const double Mj = 0.5;

#define abs(a) ((a)>0?(a):-(a))
#define zero(a) (abs(a)<EPSILON)

#endif
