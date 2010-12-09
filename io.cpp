// ----------------------------------------------------------------//
// Filename : io.cpp
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// implementation file of io.h
// ----------------------------------------------------------------//
// - Zigang Xiao Mon Oct 25 22:57:53 CDT 2010
//   * added function for parser, now can parse diode
//
// - Zigang Xiao - Wed Sep 15 15:48:58 CDT 2010
//   * Update the read_netlist function

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "net.h"
#include "util.h"
#include "io.h"
#include "node.h"
using namespace std;

extern ANALYSIS_TYPE g_atype;
extern double g_vin;
extern vector<string> g_plot_gain_node;
extern vector<string> g_plot_phase_node;
extern vector<string> g_plot_vol_node;
extern double g_init_f;
extern double g_end_f;
extern double g_step_f;

// improved version of reading value pairs
// read the initial (guess) values of voltage
// e.g., v(node1) = 1.02
//       i(vsrc)  = 4.50
void read_initial_values(string line,
		Netlist & netlist, Nodelist & nodelist){
	char * l = new char[line.size()+2];
	l[0]='x'; // trick: magic char
	strcpy(l+1, line.c_str());
	const char * sep = "=() \r\n";
	char * chs, t;
	string name;
	double value;
	int idx;
	strtok(l, sep); // initialize
	do{
		chs= strtok(NULL, sep); // v or i
		if( chs == NULL ) break;
		t = chs[0];
		chs = strtok(NULL, sep); // name
		name=string(chs);
		chs = strtok(NULL, sep); // value
		value = atof(chs);
		//cout<<"read "<<t<<"="<<name<<"="<<value<<endl;
		switch(t){
		case 'v':
		case 'V':
			idx = nodelist.name2idx[name];
			nodelist.nodelist[idx].v = value;
			break;
		case 'i':
		case 'I':
			netlist[name].current = value;
			break;
		default:
			break;
		}
	}while(chs!=NULL);
	delete [] l;
}

// read in output node names
void read_output_node(string line){
	char * l = new char[line.size()+1];
	strcpy(l, line.c_str());
	//for(size_t i=0;i<line.size();i++) cout<<i<<":"<<int(l[i])<<endl;
	const char * sep = "() \r\n";
	char * name;
       	name = strtok(l, sep);    // ac
	while( name != NULL ){
		name = strtok(NULL, sep); // reads a `VM' or a `VP'
		if( name == NULL ) break;
		if( strcmp(name, "VM") == 0 ){
			name = strtok(NULL, sep);
			g_plot_gain_node.push_back(string(name)); 
		}
		else if ( strcmp(name, "VP") == 0 ) {
			name = strtok(NULL, sep);
			g_plot_phase_node.push_back(string(name)); 
		}
		else report_exit("Unknown plot type!\n");
	}
	delete [] l;
}

// NOTE: this part is hard coded now
void read_plot_params(string line){
	char * l = new char[line.size()+1];
	strcpy(l, line.c_str());
	const char * sep = " ";
	char * value;
	value = strtok(l, sep);   // DEC
	value = strtok(NULL, sep); // step
	g_step_f = atoi(value);
	value = strtok(NULL, sep); // init
	g_init_f = atoi(value);
	value = strtok(NULL, sep); // end
	g_end_f = atoi(value);

	g_step_f = 100;
	g_init_f = 10E3;
	g_end_f = 100E6;
	delete [] l;
}
// read a ac/dc value from input string
VOL_TYPE input_voltage(ifstream & ifs, double & value, double & off,
		double &amp, double & freq){
	ifs.ignore(256,' ');
	char a = ifs.peek();
	if( a == '-' || a == '+' ||
	    ((a >= '0') && (a <= '9')) ){
		ifs>>value;
		return DC;
	}
	else if( a == 'a') {
		char c,eq;
		ifs>>a>>c>>eq; // "ac="
		assert( a == 'a' && c =='c' && eq == '=' );
		ifs>>value;
		return AC;
	}
	else {// sin
		string sin;
		ifs>>sin; //"sin"
		ifs>>off>>amp>>freq;
		value=0.0;
		return SIN;
	}
}

// given a filename, parse it and store the nets in netlist
// return the size of the square matrix
void read_netlist(char * filename, Netlist & netlist, Nodelist & nodelist){
	ifstream ifs(filename);
	if( ifs.fail() )
		report_exit("File not exist!\n");
	string name,line,dummy;
	int line_counter = 0;
	// read the node name
	while( ifs>>name ){
		line_counter++;
		// check declarative command
		if( name == ".end" ) break;
		if( name == ".op" ) continue;
		if( name == ".ac" ){
			g_atype = AC;
			getline(ifs, line);
			read_plot_params(line);
			continue;
		}
		if( name == ".plot" ){// perform AC analysis
			g_atype = AC;
			getline(ifs, line);
			read_output_node(line);
			continue;
		}

		// check device multiple definition
		if( netlist.netlist.find(name) != netlist.netlist.end() ){
			cerr<<"Warning: Ignoring duplicate \
				definition of device ["
				<<name<<"] at line "<<line_counter<<": ";
			// skip the whole line;
			getline(ifs, dummy);
			cerr<<name<<dummy<<endl;
			continue;
		}

		// check if it is initial guess
		if( name == ".nodeset" || name == "+" ){
			// set the initial voltage to the nodes, current to src
			getline(ifs,line);
			read_initial_values(line, netlist, nodelist);
			continue;
		}

		// start to read netlist
		string node1, node2, ctrl1, ctrl2, vyyy, emit, polarity;
		double v,off,amp,freq;
		char c = name[0];
		VOL_TYPE vtype;
		POLARITY p;

		// read two more node names and insert to node list
		ifs>>node1>>node2;
		nodelist.insert_node(Node(node1));
		nodelist.insert_node(Node(node2));
		switch(c){
			case 'r': // resistor
			case 'R':
				ifs>>v;
				netlist[name]=Net(RSTR, name, node1, node2, v);
				netlist.netset[RSTR].insert(name);
				break;
			case 'v': // independent voltage source
			case 'V':
				vtype = input_voltage(ifs,v,off,amp,freq);
				netlist[name]=Net(VSRC,name, node1, node2, v); 
				netlist[name].set_voltage(vtype,v,off,amp,freq);
				netlist.netset[VSRC].insert(name);
				if(vtype == AC) g_vin = v; //mark the vin value
				break;
			case 'i': // current source
			case 'I':
				ifs>>v;
				netlist[name]=Net(CSRC, name, node1, node2, v);
				netlist.netset[CSRC].insert(name);
				break;
			case 'g': // vccs
			case 'G':
				ifs>>ctrl1>>ctrl2>>v;
				netlist[name]=Net(VCCS, name, node1, node2, 
						ctrl1, ctrl2, v);
				netlist.netset[VCCS].insert(name);
				nodelist.insert_node(Node(ctrl1));
				nodelist.insert_node(Node(ctrl2));
				break;
			case 'e': // vcvs
			case 'E':
				ifs>>ctrl1>>ctrl2;
				vtype = input_voltage(ifs,v,off,amp,freq);
				netlist[name]=Net(VCVS, name, node1, node2, 
						ctrl1, ctrl2, v);
				netlist[name].set_voltage(vtype,v,
						          off,amp,freq);
				netlist.netset[VCVS].insert(name);
				nodelist.insert_node(Node(ctrl1));
				nodelist.insert_node(Node(ctrl2));
				break;
			case 'h': // ccvs
			case 'H':
				ifs>>vyyy;
				vtype = input_voltage(ifs,v,off,amp,freq);
				netlist[name]=Net(CCVS, name, 
						  node1, node2, vyyy, v);
				netlist[name].set_voltage(vtype,v,
						          off,amp,freq);
				netlist.netset[CCVS].insert(name);
				break;
			case 'f': // cccs
			case 'F':
				ifs>>vyyy>>v;
				netlist[name]=Net(CCCS, name, 
						node1, node2, vyyy, v);
				netlist.netset[CCCS].insert(name);
				break;
			case 'd': // diode
			case 'D':
				netlist[name]=Net(DIODE, name, 
						node1, node2, v);
				netlist.netset[DIODE].insert(name);
				break;
			case 'q': // bjt
			case 'Q':
				ifs>>emit>>polarity ;
				p = (polarity == "pnp"? PNP: NPN);
				netlist[name]=Net(BJT, name, 
						node1, node2, emit, p); 
				netlist.netset[BJT].insert(name);
				nodelist.insert_node(Node(emit));
				nodelist[emit].insert_net(name);
				break;
			case 'c': // capacitor
			case 'C':
				ifs>>v;
				netlist[name]=Net(CAPCT, name, node1, node2, v); 
				netlist.netset[CAPCT].insert(name);
				break;
			case 'l': // inductor
			case 'L':
				ifs>>v;
				netlist[name]=Net(INDCT, name, node1, node2, v); 
				netlist.netset[INDCT].insert(name);
				break;
			default: // Uknown type, report error and exit
				string error_msg(name+": Unknown type\n");
				report_exit(error_msg.c_str());
				break;
		}
		nodelist[node1].insert_net(name);
		nodelist[node2].insert_net(name);
	};
	ifs.close();
}
