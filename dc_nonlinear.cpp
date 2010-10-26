// ----------------------------------------------------------------//
// Filename : dc_nonlinear.cpp
// Author : Zigang Xiao <zxiao2@illinois.edu>
//
// Function definition for non-linear dc analysis, mainly newton-raphson
// ----------------------------------------------------------------//
// - Zigang Xiao - Mon Oct 25 12:45:28 CDT 2010
//   * first create the file
//
#include <cmath>
#include "dc_nonlinear.h"
#include "node.h"
#include "net.h"
#include "global.h"

// calculate the difference
double voltage_diff(double * Vnew, double * Vold, int n){
	double r=0.0,tmp;
	int i;
	for(i=0;i<n;i++) {
		tmp = Vnew[i] - Vold[i];
		r+=tmp*tmp;
	}
	return sqrt(r);
}

void copy_voltages(Nodelist & nodelist, double * vs){
	for(int i=0;i<nodelist.size();i++)
		vs[i] = nodelist.nodelist[i].v;
}

void NR_iteration(Netlist & netlist, Nodelist & nodelist,
		double *J, double *v, int size){
	// output input node information
	cout<<endl<<"Input nodes:"<<endl;
	nodelist.output_node_voltages();
	cout<<endl<<"Iteration begins:"<<endl;

	double * Vold, *Vnew;
	Vold = new double[nodelist.size()];
	Vnew = new double[nodelist.size()];
	
	int counter=0;
	double diff=0;
	do{
		copy_voltages(nodelist, Vold);
		// linearize the non-linear device and stamp and solve
		linear_dc(netlist,nodelist,J,v,size);
		update_node_voltages(nodelist,v);
		copy_voltages(nodelist, Vnew);
		diff = voltage_diff(Vnew,Vold,nodelist.size());
		cout<<"iteration: "<<++counter<<", difference="<<diff<<endl;
	}while(diff>EPSILON);
	
	cout<<"Total number of iterations: "<<counter<<endl<<endl;
	// finally, output again
	delete [] Vold;
	delete [] Vnew;

	cout<<"Result:"<<endl;
	nodelist.output_node_voltages();
}
