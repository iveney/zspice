#include <iostream>
#include <fstream>
#include <cstdio>
#include "net.h"
#include "util.h"
#include "io.h"
#include "global.h"
using namespace std;

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
		case 'r':
			ifs>>v;
			netlist[name]=Net(RSTR, name, node1, node2, v); 
			break;
		case 'v':
			ifs>>v;
			netlist[name]=Net(VSRC, name, node1, node2, v); 
			break;
		case 'i':
			ifs>>v;
			netlist[name]=Net(CSRC, name, node1, node2, v); 
			break;
		case 'g': // read two more nodes
			ifs>>ctrl1>>ctrl2>>v;
			netlist[name]=Net(VCCS, name, node1, node2, 
					ctrl1, ctrl2, v);
			break;
		case 'e':
			ifs>>ctrl1>>ctrl2>>v;
			netlist[name]=Net(VCVS, name, node1, node2, 
					ctrl1, ctrl2, v);
			break;
		case 'h':
			ifs>>vyyy>>v;
			netlist[name]=Net(CCVS, name, node1, node2, vyyy, v);
			break;
		case 'f':
			ifs>>vyyy>>v;
			netlist[name]=Net(CCCS, name, node1, node2, vyyy, v);
			break;
		case 'd':
			netlist[name]=Net(DIODE, name, node1, node2, v);
			break;
		case 'q':
			ifs>>emit>>v;
			netlist[name]=Net(BJT, name, node1, node2, emit, v); 
			break;
		case 'c':
			ifs>>v;
			netlist[name]=Net(CAPCT, name, node1, node2, v); 
			break;
		case 'l':
			ifs>>v;
			netlist[name]=Net(INDCT, name, node1, node2, v); 
			break;
		default:
			string error_msg(name+": Unknown type");
			report_exit(error_msg.c_str());
			break;
		}
	};
	ifs.close();
}
