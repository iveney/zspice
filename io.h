// ----------------------------------------------------------------//
// Filename : io.h
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// I/O stuff puts here
// ----------------------------------------------------------------//
// - Zigang Xiao - Wed Sep 15 15:48:58 CDT 2010
//   * Added comment

#ifndef __IO_H__
#define __IO_H__

#include <string>
#include "net.h"
#include "node.h"
using namespace std;

string read_bracket(ifstream & ifs);
void read_name_value_pair(ifstream & ifs, string & name, double & value);
void read_netlist(char * filename, Netlist & netlist, Nodelist & nodelist);
void read_initial_values(ifstream & ifs, Netlist & netlist, Nodelist & nodelist);

#endif
