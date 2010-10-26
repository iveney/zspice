// ----------------------------------------------------------------//
// Filename : main.cpp
// Author : Zigang Xiao <zxiao2@illinois.edu>
//
// Program entry.
// ----------------------------------------------------------------//
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
#include "main.h"
using namespace std;

int main(int argc, char *argv[]){
	if(argc < 2)
		report_exit("Usage: zspice netlist\n");
	char * filename = argv[1];

	Netlist netlist;
	Nodelist nodelist;
	read_netlist(filename, netlist, nodelist);

	cout<<endl;
	cout<<"** Net information **"<<endl;
	cout<<netlist<<endl;
	cout<<"** Node information **"<<endl;
	cout<<nodelist<<endl;

	// linear_dc_analysis
	//cout<<"Start to analyze the linear dc circuit ... "<<endl;
	//dc_analysis(netlist,nodelist,LINEAR);

	// non_linear dc analysis
	cout<<"Start to analyze the non-linear dc circuit ... "<<endl;
	dc_analysis(netlist,nodelist,NON_LINEAR);

	return 0;
}
