// ----------------------------------------------------------------//
// Filename : util.cpp
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// Source code for some utility functions
// ----------------------------------------------------------------//

#include <stdlib.h>
#include <stdio.h>

// report an error string and exit the program
// Note that it is wrapped by a macro `report_exit'
// Do NOT use this directly.
void _report_exit(const char *location, const char *msg){
#ifdef DEBUG
	  fprintf(stderr,"Error at %s: %s", location, msg);
#else
	  fprintf(stderr,"%s", msg);
#endif
	  exit(1);
}

