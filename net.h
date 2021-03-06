// ----------------------------------------------------------------//
// Filename : net.h
// Author : Zigang Xiao <zxiao2@illinois.edu>
//
// Definition of nets
// ----------------------------------------------------------------//
// - Zigang Xiao - Sun Sep 19 17:16:42 CDT 2010
//   * added a macro for fast traversing the net list
// - Zigang Xiao - Mon Sep 13 12:54:13 CDT 2010
//   * first create the file

#ifndef __NET_H__
#define __NET_H__

#include <string>
#include <tr1/unordered_map>
#include <set>
#include <iostream>
#include "global.h"
using namespace std;
using namespace std::tr1;

class Net{
public:
	Net();
	Net(NODETYPE type, string name, string node1, string node2, double value=0.);
	Net(NODETYPE t, string n, string node1, string node2,
		string ctrl1, string ctrl2, double v);
	Net(NODETYPE t, string n, string node1, string node2, string vy, double v);
	Net(NODETYPE t, string n, string collect, string base, 
			string emit, POLARITY pol);

	void set(NODETYPE type, string name, string node1, string node2, 
			double value);

	void set_voltage(VOL_TYPE vtype=DC, double value=0., double offset=0.,
			double amplitude=0., double freq=0.);

	// auxiliary function to compute some values for BJT
	void init_BJT();
	double compute_Ic(double Vc, double Vb, double Ve);
	double compute_Ib(double Vc, double Vb, double Ve);

	double compute_Cc(double Vc);
	double compute_Cbe(double Vbe, double Vbc);
	double compute_Cbe2(double Vbe, double Vbc);
	double compute_Cbc(double Vbc);

	double compute_dCc(double Vc);
	double compute_dCbe(double Vbe, double Vbc);
	double compute_dCbe2(double Vbe, double Vbc);
	double compute_dCbc(double Vbc);

	void compute_Cc_eq(double Vc, double Vtn_c, 
			double &CIeq, double & CGeq);
	void compute_Cbe_eq(double Vbe, double Vbc,
			double Vtn_be, double Vtn_bc,
			double & BEIeq, double & BEGeq1, double & BEGeq2);
	void compute_Cbc_eq(double Vbc, double Vtn_bc, 
			double &BCIeq, double &BCGeq);

	NODETYPE type;
	string name;
	string nbr[2];     // as of BJT, they denotes collector and base
	string ctrl[2];
	string vyyy;
	double value;      // charateristic value - I don't know what it means.

	VOL_TYPE vtype;    
	double current;    // stores the voltage source branch current
	double offset,amplitude,freq;  // for sin signal

	// for BJT
	string emit;       
	POLARITY polarity; // BJT polarity
	double Ic, Ib;     
	double hc[4], hb[4];

	// BJT AC analysis
	// i.e.: B[3] = {BB, BC, BE}
	double B[3], C[3], E[3];
	// double Cc, Cbe, Cbc; // parastic capacitance
};

// functor of translating string to char*
/* tr1 now provide this
namespace __gnu_cxx{ 
	template<> struct hash< std::string >{
		size_t operator()( const std::string& x ) const {
			return hash< const char* >()( x.c_str() );
		}
	};
}
*/

class Netlist{
public:
	Net & operator [] (string name);
	friend ostream & operator <<(ostream &s, Netlist& );
	void output_branch_currents(unordered_map<string,int> & net2int, 
		double * v);

	// component sets, there 11 different types
	set<string> netset[NUM_NETTYPE];
	unordered_map<string, Net> netlist;
};

// some macros to iterate the sets
#define FORALL_CONCAT(x,y) x##y
#define FORALL_VAR(y) FORALL_CONCAT(FORALL_VAR,y)

// note that the set may be empty, hence need to check at the very beginning
#define foreach_net_in( nlist, type, net ) \
	set<string>::iterator FORALL_VAR(__LINE__); \
	for(FORALL_VAR(__LINE__) = (nlist).netset[(type)].begin(), \
	    (net) = (FORALL_VAR(__LINE__) == (nlist).netset[(type)].end())? Net(): (nlist)[*FORALL_VAR(__LINE__)] ; \
	    FORALL_VAR(__LINE__) != (nlist).netset[(type)].end() ; \
	    FORALL_VAR(__LINE__)++,  \
	    (net) = (FORALL_VAR(__LINE__) == (nlist).netset[(type)].end())? Net(): (nlist)[*FORALL_VAR(__LINE__)])

#endif

