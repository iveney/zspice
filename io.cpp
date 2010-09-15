#include <iostream>
#include <fstream>
#include <cstdio>
#include "net.h"
#include "util.h"
#include "io.h"
using namespace std;

// given a filename, parse it and store the nets in netlist
// return the size of the square matrix
void read_netlist(char * filename, Netlist & netlist){
	ifstream ifs(filename);
	string name;
	// read the node name
	while( ifs>>name ){
		// check errors
		if( name == ".eof" || name == ".op" ) break;

		string node1, node2, ctrl1, ctrl2, vyyy, emit;
		double v;
		char c = name[0];

		// read two more node names
		ifs>>node1>>node2;
		switch(c){
		case 'r': // resistor
			ifs>>v;
			netlist[name]=Net(RSTR, name, node1, node2, v); 
			netlist.netset[RSTR].insert(name);
			break;
		case 'v': // independent voltage source
			ifs>>v;
			netlist[name]=Net(VSRC, name, node1, node2, v); 
			netlist.netset[VSRC].insert(name);
			break;
		case 'i': // current source
			ifs>>v;
			netlist[name]=Net(CSRC, name, node1, node2, v); 
			netlist.netset[CSRC].insert(name);
			break;
		case 'g': // vccs
			ifs>>ctrl1>>ctrl2>>v;
			netlist[name]=Net(VCCS, name, node1, node2, 
					ctrl1, ctrl2, v);
			netlist.netset[VCCS].insert(name);
			break;
		case 'e': // vcvs
			ifs>>ctrl1>>ctrl2>>v;
			netlist[name]=Net(VCVS, name, node1, node2, 
					ctrl1, ctrl2, v);
			netlist.netset[VCVS].insert(name);
			break;
		case 'h': // ccvs
			ifs>>vyyy>>v;
			netlist[name]=Net(CCVS, name, node1, node2, vyyy, v);
			netlist.netset[CCVS].insert(name);
			break;
		case 'f': // cccs
			ifs>>vyyy>>v;
			netlist[name]=Net(CCCS, name, node1, node2, vyyy, v);
			netlist.netset[CCCS].insert(name);
			break;
		case 'd': // diode
			netlist[name]=Net(DIODE, name, node1, node2, v);
			netlist.netset[DIODE].insert(name);
			break;
		case 'q': // bjt
			ifs>>emit>>v;
			netlist[name]=Net(BJT, name, node1, node2, emit, v); 
			netlist.netset[BJT].insert(name);
			break;
		case 'c': // capacitor
			ifs>>v;
			netlist[name]=Net(CAPCT, name, node1, node2, v); 
			netlist.netset[CAPCT].insert(name);
			break;
		case 'l': // inductor
			ifs>>v;
			netlist[name]=Net(INDCT, name, node1, node2, v); 
			netlist.netset[INDCT].insert(name);
			break;
		default: // Uknown type, report error and exit
			string error_msg(name+": Unknown type");
			report_exit(error_msg.c_str());
			break;
		}
	};
	ifs.close();
}
