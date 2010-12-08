// ----------------------------------------------------------------//
// Filename : net.cpp
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// implementation file of ac_analysis.h
// ----------------------------------------------------------------//
// - Zigang Xiao - Tue Dec  7 12:42:11 CST 2010
//   * first create the file
//
//
#include <cmath>
#include "ac_analysis.h"
#include "dc_linear.h"
#include "dc_nonlinear.h"
#include "global.h"

// compute Cc
double compute_Cc(double Vc, POLARITY pol){
	double Cc;

	if(Vc < Fc * Vj){
		Cc = Cjcs*pow(1-Vc/Vj,-Mj);
	}
	else{ // Vc >= Fc * Vj
		Cc = Cjcs*(Mj*Vc/Vj + 1.0 - Fc*(1+Mj))/pow(1-Fc,1+Mj);
	}
	return Cc;
}

double compute_Cbe(double Vbe, double Vbc, POLARITY pol){
	double Cbe =   Tf*Is*exp(Vbe/Vt)*(1-Vbc/VAf-Vbe/VAr)/Vt
		    - Tf*Is*(exp(Vbe/Vt)-1)/VAr;
	if(Vbe < Fc * Vj ){
		Cbe += Cjbe*pow(1-Vbe/Vj,-Mj);
	}
	else{
		Cbe += Cjbe*(Mj*Vbe/Vj + 1.0 - Fc*(1+Mj))/pow(1-Fc,1+Mj);
	}
	return Cbe;
}

double compute_Cbc(double Vbe, double Vbc, POLARITY pol){
	double Cbc = Tr*Is*exp(Vbc/Vt)/Vt;
	if(Vbc < Fc * Vj){
		Cbc += Cjbc*pow(1-Vbc/Vj,-Mj);
	}
	else{
		Cbc += Cjbc*(Mj*Vbc/Vj + 1.0 - Fc*(1+Mj))/pow(1-Fc,1+Mj);
	}
	return Cbc;
}

void compute_BJT_cap(Net & net, Nodelist & nodelist){
	// find the names of three terminals
	string clct = net.nbr[0];
	string base = net.nbr[1];
	string emit = net.emit;

	double Vc = nodelist[clct].v;
	double Vb = nodelist[base].v;
	double Ve = nodelist[emit].v;

	double Vbc = Vb - Vc;
	double Vbe = Vb - Ve;
	if( net.polarity == PNP ){
		Vbc = -Vbc;
		Vbe = -Vbe;
	}
	net.Cc  = compute_Cc(Vc, net.polarity);
	net.Cbe = compute_Cbe(Vbe, Vbc, net.polarity);
	net.Cbc = compute_Cbc(Vbe, Vbc, net.polarity);

	/*
	if( net.polarity == PNP ){
		net.Cc  = -net.Cc;
		net.Cbe = -net.Cbe;
		net.Cbc = -net.Cbc;
	}
	*/
}

// after obtaining the DC operating point, compute capacitance of BJT
void compute_caps(Netlist & netlist, Nodelist & nodelist){
	set<string>::iterator it;
	set<string> & bjts = netlist.netset[BJT];
	for(it=bjts.begin(); it!=bjts.end(); ++it){
		Net & net = netlist[*it];
		compute_BJT_cap(net,nodelist);
	}
}

// perform ac analysis
void ac_analysis(Netlist & netlist, Nodelist & nodelist){
	// first perform dc analysis to get the DC operating point
	dc_analysis(netlist,nodelist,false);

	// now we got Vb, Vc, Ve for each BJT
	compute_caps(netlist, nodelist);

	// set a frequency and stamp the matrix
	double step = 10.0;
	double inc = pow(10.0,1.0/step);
	double init = 10E3, final = 100E6;
	for(double f=init;f<=final;f*=inc){
		// stamp the fixed part: resistor, voltage source)

		// stamp the frequency-related part:
		// capacitor, inductor, bjt
	}
}
