// ----------------------------------------------------------------//
// Filename : main.cpp
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// Program entry.
// ----------------------------------------------------------------//
// - Zigang Xiao - Sun Sep 12 23:57:25 CDT 2010
//   * first create the file

#include <iostream>
#include "util.h"
#include "net.h"
#include "io.h"
#include "main.h"
using namespace std;

int main(int argc, char *argv[]){
	if(argc < 2){
		report_exit("Usage: zspice netlist\n");
	}
	char * filename = argv[1];
	Netlist netlist;
	read_netlist(filename, netlist);
	netlist.output_sets();
	return 0;
}
