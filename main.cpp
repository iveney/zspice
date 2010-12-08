// ----------------------------------------------------------------//
// Filename : main.cpp
// Author : Zigang Xiao <zxiao2@illinois.edu>
//
// Program entry.
// ----------------------------------------------------------------//
// - Zigang Xiao - Mon Oct 25 22:56:38 CDT 2010
//   * added part of phase 2, lack of BJT
//
// - Zigang Xiao - Sun Sep 19 17:16:05 CDT 2010
//   * Finish phase 1 linear dc analysis
//   * added file existence detection
// 
// - Zigang Xiao - Sun Sep 12 23:57:25 CDT 2010
//   * first create the file

#include <iostream>
#include "util.h"
#include "net.h"
#include "node.h"
#include "io.h"
#include "dc_linear.h"
#include "ac_analysis.h"
#include "main.h"
using namespace std;

const static char *str_help="Supported types:\n 1: DC\n 2: AC";

void output_netlist_info(Netlist & netlist, Nodelist & nodelist){
	cout<<endl;
	cout<<"** Net information **"<<endl;
	cout<<netlist<<endl;
	cout<<"** Node information **"<<endl;
	cout<<nodelist<<endl;
}

int main(int argc, char *argv[]){
	if(argc < 3)
		report_exit("Usage: zspice netlist type\n");
	char * filename = argv[1];
	int type = atoi(argv[2]);

	Netlist netlist;
	Nodelist nodelist;
	read_netlist(filename, netlist, nodelist);

	switch(type){
	case 1: // DC analysis
		cout<<"** DC analysis **"<<endl;
		output_netlist_info(netlist,nodelist);
		dc_analysis(netlist,nodelist,true);
		break;
	case 2: // AC analysis
		cout<<"** AC analysis **"<<endl;
		output_netlist_info(netlist,nodelist);
		ac_analysis(netlist,nodelist);
		break;
	default:
		cout<<"Unknown type. ";
		cout<<str_help<<endl;
		break;
	}

	return 0;
}
