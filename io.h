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

#include "net.h"
#include "node.h"

void read_netlist(char * filename, Netlist & netlist, Nodelist & nodelist);

#endif
