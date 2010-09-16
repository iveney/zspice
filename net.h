// ----------------------------------------------------------------//
// Filename : net.h
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// Definition of nets
// ----------------------------------------------------------------//
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
	Net(NODETYPE type, string name, string node1, string node2, double value);
	Net(NODETYPE t, string n, string node1, string node2,
		string ctrl1, string ctrl2, double v);
	Net(NODETYPE t, string n, string node1, string node2, string vy, double v);
	Net(NODETYPE t, string n, string collect, string base, 
			string emit, POLARITY pol);

	void set(NODETYPE type, string name, string node1, string node2, double value);

	NODETYPE type;
	string name;
	string nbr[2];
	string ctrl[2];
	string vyyy;
	string emit;
	double value;
	POLARITY polarity;
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
private:
	hash_map<string, Net> netlist;
};

#endif

