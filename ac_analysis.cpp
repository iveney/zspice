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
#include <vector>
#include <set>
#include <iomanip>
#include <fstream>
#include <complex>
#include "ac_analysis.h"
#include "dc_linear.h"
#include "dc_nonlinear.h"
#include "umfpack.h"
#include "global.h"
#include "util.h"
using namespace std;

extern string g_basename;
extern double g_vin;
extern VecStr g_plot_gain_node;
extern VecStr g_plot_phase_node;
extern VecStr g_plot_vol_node;
extern double g_init_f;
extern double g_end_f;
extern double g_step_f;

/*
double compute_Cc(double Vc, POLARITY pol){
	double Cc;
	if(Vc < Fc * Vj) 
		Cc = Cjcs*pow(1-Vc/Vj,-Mj);
	else 
		Cc = Cjcs*(Mj*Vc/Vj + 1.0 - Fc*(1+Mj))/pow(1-Fc,1+Mj);
	return Cc;
}

double compute_Cbe(double Vbe, double Vbc, POLARITY pol){
	double Cbe =   Tf*Is*exp(Vbe/Vt)*(1-Vbc/VAf-Vbe/VAr)/Vt
		    - Tf*Is*(exp(Vbe/Vt)-1)/VAr;
	if(Vbe < Fc * Vj ) 
		Cbe += Cjbe*pow(1-Vbe/Vj,-Mj);
	else 
		Cbe += Cjbe*(Mj*Vbe/Vj + 1.0 - Fc*(1+Mj))/pow(1-Fc,1+Mj);
	return Cbe;
}

double compute_Cbe2(double Vbe, double Vbc, POLARITY pol){
	return -Tf*Is*(exp(Vbe/Vt)-1)/VAf;
}

double compute_Cbc(double Vbe, double Vbc, POLARITY pol){
	double Cbc = Tr*Is*exp(Vbc/Vt)/Vt;
	if(Vbc < Fc * Vj)
		Cbc += Cjbc*pow(1-Vbc/Vj,-Mj);
	else
		Cbc += Cjbc*(Mj*Vbc/Vj + 1.0 - Fc*(1+Mj))/pow(1-Fc,1+Mj);
	return Cbc;
}
*/

// open files to plot
void open_AC_plot_files(Nodelist & nodelist, 
		     vector<FILE *> & fgain,
		     vector<FILE *> & fphase){
	string name;
	// open plot gain files
	for(size_t i=0;i<g_plot_gain_node.size();i++){
		name = g_plot_gain_node[i];
		string of = g_basename + "_" + name + "_g.dat";
		cout<<"Node \""<<name<<"\" magnitude plot : "<<of<<endl;
		FILE * ofs=fopen(of.c_str(),"w");
		fgain.push_back(ofs);
	}
	// open plot gain files
	for(size_t i=0;i<g_plot_phase_node.size();i++){
		name = g_plot_phase_node[i];
		string of = g_basename + "_" + name + "_p.dat";
		cout<<"Node \""<<name<<"\" phase plot : "<<of<<endl;
		FILE* ofs=fopen(of.c_str(),"w");
		fphase.push_back(ofs);
	}
}

void close_AC_plot_files(vector<FILE *> & fgain,
		      vector<FILE *> & fphase){
	for(size_t i=0;i<fgain.size();i++) fclose(fgain[i]);
	for(size_t i=0;i<fphase.size();i++) fclose(fphase[i]);
}

void plot_gains(vector<FILE *> & fgain, Nodelist & nodelist,
		double f, double * vx, double * vz){
	for(size_t i=0;i<fgain.size();i++){
		string name = g_plot_gain_node[i];
		int id = nodelist.name2idx[name];
		complex<double> vout(vx[id],vz[id]);
		double s = abs(vout)/g_vin;
		double gain = 20*log10(s);
		//cout<<"f="<<f<<", "<<name<<"="<<vout<<" ,s="<<s<<endl;
		fprintf(fgain[i], "%.9lf %.9lf\n", f, gain);
	}
}

void plot_phase(vector<FILE *> & fphase, Nodelist & nodelist,
		double f, double * vx, double * vz){
	for(size_t i=0;i<fphase.size();i++){
		string name = g_plot_phase_node[i];
		int id = nodelist.name2idx[name];
		complex<double> vout(vx[id],vz[id]);
		double phase = arg(vout)*180.0/PI;
		fprintf(fphase[i], "%.9lf %.9lf\n", f, phase);
	}
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
		Vc = -Vc;
		Vbc = -Vbc;
		Vbe = -Vbe;
	}

	// compute 
	double Cc  = net.compute_Cc(Vc);
	double Cbe = net.compute_Cbe(Vbe, Vbc);
	double Cbe2 = net.compute_Cbe2(Vbe, Vbc);
	double Cbc = net.compute_Cbc(Vbc);

	net.B[0] =   Cbe  + Cbe2 + Cbc;
	net.B[1] = -(Cbe2 + Cbc);
	net.B[2] = -Cbe;
	
	net.C[0] = -Cbc;
	net.C[1] =  Cc + Cbc;
	net.C[2] =  0;

	net.E[0] = -(Cbe  + Cbe2);
	net.E[1] =   Cbe2;
	net.E[2] =   Cbe;
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

// Solve the complex matrix, the matrix should be stamped
void solve_ac(Triplet & t, double * vx, double * vz, 
		double * Jx, double * Jz, int n){
	// convert to column compressed form 
	int n_row = n; // do not count ground row
	int n_col = n; // do not count ground column
	int nz = t.size();
	int * Ap = new int [n_col+1];
	int * Ai = new int [nz];
	double * Ax = new double [nz];
	double * Az = new double [nz];

	int * Ti = new int[nz];
	int * Tj = new int[nz];
	double * Tx = new double[nz];
	double * Tz = new double[nz];
	vector_to_array<int>(t.Ti, Ti);
	vector_to_array<int>(t.Tj, Tj);
	vector_to_array<double>(t.Tx, Tx);
	vector_to_array<double>(t.Tz, Tz);

	int status;
	double Control [UMFPACK_CONTROL];
	umfpack_zi_defaults (Control) ;
	status = umfpack_zi_triplet_to_col(n_row, n_col, nz, Ti, Tj, 
			Tx, Tz, Ap, Ai, Ax, Az, (int *) NULL);


	if( status < 0 ) {
		umfpack_zi_report_status (Control, status) ;
		report_exit("umfpack_zi_triplet_to_col failed\n") ;
	}

	double *null = (double *) NULL;
	void *Symbolic, *Numeric;
	status = umfpack_zi_symbolic(n, n, Ap, Ai, Ax, Az, 
			&Symbolic, Control, null) ; 
	if( status < 0 ){
		umfpack_zi_report_status (Control, status) ;
		report_exit("umfpack_di_symbolic failed\n") ;
	}
	status = umfpack_zi_numeric(Ap, Ai, Ax, Az, 
			Symbolic, &Numeric, Control, null) ;
	if( status < 0 ){
		umfpack_zi_report_status (Control, status) ;
		report_exit("umfpack_di_numeric failed\n") ;
	}
	umfpack_zi_free_symbolic (&Symbolic) ;

	status = umfpack_zi_solve(UMFPACK_A, Ap, Ai, 
			Ax, Az,
			vx, vz,
			Jx, Jz,
			Numeric, Control, null) ;

	if( status < 0 ){
		umfpack_zi_report_status (Control, status) ;
		report_exit("umfpack_di_solve failed\n") ;
	}
	umfpack_zi_free_numeric (&Numeric) ;

	delete [] Az;
	delete [] Ax;
	delete [] Ai;
	delete [] Ap;

	delete [] Ti;
	delete [] Tj;
	delete [] Tx;
	delete [] Tz;
}

void stamp_BJT_AC(Netlist & netlist, Nodelist & nodelist,
		Triplet & t, double f){
	// no imaginary part?
	Net net;
	double w = 2*PI*f;
	foreach_net_in(netlist, BJT, net){
		string clct = net.nbr[0];
		string base = net.nbr[1];
		string emit = net.emit;

		// find the index of c,b,e
		int c = nodelist.name2idx[clct];
		int b = nodelist.name2idx[base];
		int e = nodelist.name2idx[emit];

		t.push(b,b,0,net.B[0] * w);
		t.push(b,c,0,net.B[1] * w);
		t.push(b,e,0,net.B[2] * w);

		t.push(c,b,0,net.C[0] * w);
		t.push(c,c,0,net.C[1] * w);
		t.push(c,e,0,net.C[2] * w);

		t.push(e,b,0,net.E[0] * w);
		t.push(e,c,0,net.E[1] * w);
		t.push(e,e,0,net.E[2] * w);
	}
}



// perform ac analysis
void ac_analysis(Netlist & netlist, Nodelist & nodelist){
	// first perform dc analysis to get the DC operating point
	dc_analysis(netlist,nodelist,false);

	// now we got Vb, Vc, Ve for each BJT
	compute_caps(netlist, nodelist);

	// initialize the matrix
	Triplet t, t_copy;
	int size = nodelist.size();
	size += netlist.netset[VSRC].size();
	size += netlist.netset[VCVS].size();
	size += netlist.netset[CCVS].size();
	double * vx = new double[size];
	double * vz = new double[size];
	double * Jx = new double[size];
	double * Jz = new double[size];
	double * Jx_copy = new double [size];
	memset((void*)vx, 0, sizeof(double)*size); // real part of sol
	memset((void*)vz, 0, sizeof(double)*size); // img part of sol
	memset((void*)Jx, 0, sizeof(double)*size);
	memset((void*)Jz, 0, sizeof(double)*size);
	memset((void*)Jx_copy, 0, sizeof(double)*size);

	// stamp the fixed part: resistor, voltage/current source
	// they are NOT related to frequency
	// NOTE: DC current is OPEN circuit
	//       DC voltage is SHORT circuit
	// backup the triplet and real part in t_copy, Jx_copy
	stamp_linear(netlist, nodelist, t_copy, Jx_copy, AC);

	// this is a trick: use Jx as a dummy J, since we do not stamp
	// J anyway (i.e., Jx will be cleared later)
	stamp_BJT_DC(netlist, nodelist, t_copy, Jx);

	vector<FILE *> fgain;
	vector<FILE *> fphase;
	open_AC_plot_files(nodelist, fgain, fphase);

	// set a frequency and stamp the matrix
	double inc = pow(10.0,1.0/g_step_f);
	//g_end_f = g_init_f;
	for(double f=g_init_f;f<=g_end_f;f*=inc){
		t = t_copy;
		memcpy((void*)Jx,(void*)Jx_copy,sizeof(double)*size);
		memset((void*)Jz, 0, sizeof(double)*size);
		memset((void*)vx, 0, sizeof(double)*size);
		memset((void*)vz, 0, sizeof(double)*size);

		// stamp the frequency-related part:
		// capacitor, inductor, bjt
		stamp_capacitor(netlist, nodelist, t, f, AC);
		stamp_inductor(netlist, nodelist, t, f, AC);
		stamp_BJT_AC(netlist, nodelist, t, f);
		Jx[0] = Jz[0] = 0.0;

		solve_ac(t, vx, vz, Jx, Jz, size);
		
		// now we got solutions, compute gain and phase
		plot_gains(fgain, nodelist, f, vx, vz);
		plot_phase(fphase, nodelist, f, vx, vz);
	}
	delete [] vx;
	delete [] vz;
	delete [] Jx;
	delete [] Jz;
	delete [] Jx_copy;
	close_AC_plot_files(fgain,fphase);
}
