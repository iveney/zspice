// ----------------------------------------------------------------//
// Filename : io.cpp
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// implementation file of io.h
// ----------------------------------------------------------------//
// - Zigang Xiao - Wed Sep 15 15:48:58 CDT 2010
//   * Update the read_netlist function

#include <iostream>
#include <fstream>
#include "net.h"
#include "util.h"
#include "io.h"
#include "node.h"
using namespace std;

string read_bracket(ifstream & ifs){
	char lb,c;
	string name;
	ifs>>lb;
	do{
		ifs>>c;
		if(c!=')')
			name.push_back(c);
		else
			break;
	}while(1);
	return name;
}

void read_name_value_pair(ifstream & ifs, string & name, double & value){
	char eq;
	name = read_bracket(ifs);
	ifs>>eq>>value;
}

// read the initial values
// e.g., v(node1)=1.02
//       i(vsrc) =4.50
void read_initial_values(ifstream & ifs, Nodelist & nodelist){
	char d;
	string name;
	int idx;
	double guess;
	bool stop = false;
	ifs>>noskipws;
	do{
		ifs>>d;
		switch(d){
		case 'v':
			read_name_value_pair(ifs,name,guess);
			idx = nodelist.name2idx[name];
			nodelist.nodelist[idx].v = guess; // set the value!
			//cout<<"nodeset: v "<<name<<" "<<guess<<endl;
			break;
		case 'i':
			read_name_value_pair(ifs,name,guess);
			// TODO: set vsrc initial value
			//cout<<"nodeset: i "<<name<<" "<<guess<<endl;
			break;
		case ' ' : // ignore space
			break;
		case '\n':
			stop = true;
			break;
		}
	}while(!stop);
	ifs>>skipws;
}

// given a filename, parse it and store the nets in netlist
// return the size of the square matrix
void read_netlist(char * filename, Netlist & netlist, Nodelist & nodelist){
	ifstream ifs(filename);
	if( ifs.fail() )
		report_exit("File not exist!\n");
	string name;
	int line_counter = 0;
	// read the node name
	while( ifs>>name ){
		// check format
		if( name == ".end" || name == ".op" ) break;
		line_counter++;

		// check device multiple definition
		if( netlist.netlist.find(name) != netlist.netlist.end() ){
			cerr<<"Warning: Ignoring duplicate \
				definition of device ["
				<<name<<"] at line "<<line_counter<<": ";
			// skip the whole line;
			string dummy;
			getline(ifs, dummy);
			cerr<<name<<dummy<<endl;
			continue;
		}

		// check if it is initial guess
		if( name == ".nodeset" || name == "+" ){
			// set the initial voltage to the nodes, current to src
			read_initial_values(ifs, nodelist);
			continue;
		}

		// start to read netlist
		string node1, node2, ctrl1, ctrl2, vyyy, emit;
		double v;
		char c = name[0];

		// read two more node names and insert to node list
		ifs>>node1>>node2;
		nodelist.insert_node(Node(node1));
		nodelist.insert_node(Node(node2));
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
				nodelist.insert_node(Node(ctrl1));
				nodelist.insert_node(Node(ctrl2));
				break;
			case 'e': // vcvs
				ifs>>ctrl1>>ctrl2>>v;
				netlist[name]=Net(VCVS, name, node1, node2, 
						ctrl1, ctrl2, v);
				netlist.netset[VCVS].insert(name);
				nodelist.insert_node(Node(ctrl1));
				nodelist.insert_node(Node(ctrl2));
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
				nodelist.insert_node(Node(emit));
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
		nodelist[node1].insert_net(name);
		nodelist[node2].insert_net(name);
	};
	ifs.close();
}
