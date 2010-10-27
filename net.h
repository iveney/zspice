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
#include <ext/hash_map>
#include <set>
#include <iostream>
#include "global.h"
using namespace std;
using namespace __gnu_cxx;

class Net{
public:
	Net();
	Net(NODETYPE type, string name, string node1, string node2, double value=0.);
	Net(NODETYPE t, string n, string node1, string node2,
		string ctrl1, string ctrl2, double v);
	Net(NODETYPE t, string n, string node1, string node2, string vy, double v);
	Net(NODETYPE t, string n, string collect, string base, 
			string emit, POLARITY pol);

	void set(NODETYPE type, string name, string node1, string node2, double value);

	void set_voltage(VOL_TYPE vtype=DC, double value=0., double offset=0.,
			double amplitude=0., double freq=0.);

	NODETYPE type;
	string name;
	string nbr[2]; // for BJT, they denotes collector and base
	string ctrl[2];
	string vyyy;
	string emit;
	double value; // charateristic value - I don't know what it means.
	POLARITY polarity;
	VOL_TYPE vtype;

	double current;
	double offset,amplitude,freq;
};

// functor of translating string to char *
namespace __gnu_cxx{ 
	template<> struct hash< std::string >{
		size_t operator()( const std::string& x ) const {
			return hash< const char* >()( x.c_str() );
		}
	};
}

class Netlist{
public:
	Net & operator [] (string name);
	friend ostream & operator <<(ostream &s, Netlist& );

	// component sets, there 11 different types
	set<string> netset[NUM_NETTYPE];
	hash_map<string, Net> netlist;
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

