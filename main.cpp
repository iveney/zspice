// ----------------------------------------------------------------//
// Filename : main.cpp
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// Program entry.
// ----------------------------------------------------------------//
// - Zigang Xiao - Sun Sep 12 23:57:25 CDT 2010
//   * first create the file

#include "util.h"
#include "main.h"

int main(int argc, char *argv[]){
	if(argc < 2){
		report_exit("Usage: zspice netlist\n");
	}
	return 0;
}
