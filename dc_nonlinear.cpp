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

// calculate the voltage difference between two voltage vectors
double voltage_diff(double * Vnew, double * Vold, int n){
	double r=0.0,tmp;
	for(int i=0;i<n;i++) {
		tmp = Vnew[i] - Vold[i];
		r+=tmp*tmp;
	}
	return sqrt(r);
}

// given a nodelist, copy the voltage values to a vector vs
void copy_voltages(Nodelist & nodelist, double * vs){
	for(int i=0;i<nodelist.size();i++)
		vs[i] = nodelist.nodelist[i].v;
}

double * Jold; // used to keep the old currents
// performs Newton-Raphson iteration here
void NR_iteration(Netlist & netlist, Nodelist & nodelist,
		double *v, double *J, int size){
	// output input node information
	cout<<endl<<"Input nodes:"<<endl;
	nodelist.output_node_voltages();
	cout<<endl<<"Iteration begins:"<<endl;

	// backup the voltages
	double * Vold, *Vnew;
	Vold = new double[nodelist.size()];
	Vnew = new double[nodelist.size()];

	// backup the currents
	Jold = new double[size];
	memset((void*)J, 0, sizeof(double)*size);
	
	int counter=0;
	double diff=0;
	do{
		// copy old value of voltages and currents
		copy_voltages(nodelist, Vold);
		memcpy(Jold, J, sizeof(double)*size); // J[i]=0 at 1st 

		// linearize the non-linear device, stamp and solve
		dc_core(netlist,nodelist,v,J,size);

		// update new value of voltages and currents
		update_node_voltages(nodelist,v);

		// copy new value of voltages
		copy_voltages(nodelist, Vnew);

		diff = voltage_diff(Vnew,Vold,nodelist.size());
		cout<<"iteration: "<<++counter<<", difference="<<diff<<endl;
	}while(diff>EPSILON);
	
	cout<<"Total number of iterations: "<<counter<<endl<<endl;
	delete [] Vold;
	delete [] Vnew;
	delete [] Jold;

	// finally, output again
	cout<<"Result:"<<endl;
	nodelist.output_node_voltages();
}
