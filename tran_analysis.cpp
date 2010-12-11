// ----------------------------------------------------------------//
// Filename : tran_analysis.h
// Author : Zigang Xiao <zxiao2@illinois.edu>
//
// Implentation file of tran_analysis.h
// ----------------------------------------------------------------//
// - Zigang Xiao - Fri Dec 10 00:16:37 CST 2010
//   * First create the file

#include "global.h"
#include "util.h"
#include "dc_linear.h"
#include "dc_nonlinear.h"
#include "tran_analysis.h"

extern string g_basename;
extern double g_step_tran;
extern double g_end_tran ;
extern VecStr g_plot_tran_node;

// open files to plot transient
void open_tran_plot_files(Nodelist & nodelist, 
		     vector<FILE *> & fplot){
	string name;
	// open plot transient files
	for(size_t i=0;i<g_plot_tran_node.size();i++){
		name = g_plot_tran_node[i];
		string of = g_basename + "_" + name + "_t.dat";
		cout<<"Node \""<<name<<"\" transient plot : "<<of<<endl;
		FILE * ofs=fopen(of.c_str(),"w");
		fplot.push_back(ofs);
	}
}

void close_tran_plot_files(vector<FILE *> & fplot){
	for(size_t i=0;i<fplot.size();i++) fclose(fplot[i]);
}

void plot_transient(vector<FILE *> & fplot, Nodelist & nodelist, 
		double time){
	for(size_t i=0;i<fplot.size();i++){
		string name = g_plot_tran_node[i];
		int id = nodelist.name2idx[name];
		double vol = nodelist.nodelist[id].v;
		fprintf(fplot[i], "%.9lf %.9lf\n", time, vol);
	}
}

// idea:
// use DC value for initial voltage value of V(t_n+1) in k-th (0-th) iteration
// (that is, the same as V(t_n), and we already have V(t_n) )
// compute Ieq, Geq and stamp
void stamp_BJT_TRAN(Netlist & netlist, Nodelist & nodelist,
		double time, double * Vk_1, double * Vtn,
		Triplet & t, double * J){
	// compute Ceq, dC, Ieq and Geq
	set<string>::iterator it;
	set<string> & bjts = netlist.netset[BJT];
	for(it=bjts.begin(); it!=bjts.end(); ++it){
		Net & net = netlist[*it];
		string clct = net.nbr[0];
		string base = net.nbr[1];
		string emit = net.emit;

		int c = nodelist.name2idx[clct];
		int b = nodelist.name2idx[base];
		int e = nodelist.name2idx[emit];

		// get V(t_n)
		double Vtn_c = Vtn[c];
		double Vtn_b = Vtn[b];
		double Vtn_e = Vtn[e];
		double Vtn_bc = Vtn_b - Vtn_c;
		double Vtn_be = Vtn_b - Vtn_e;

		// get V^(k-1)(t_n+1)
		double Vc = Vk_1[c];
		double Vb = Vk_1[b];
		double Ve = Vk_1[e];
		double Vbc = Vb - Vc;
		double Vbe = Vb - Ve;

		double CIeq, CGeq, BEIeq, BEGeq1, BEGeq2, BCIeq, BCGeq;
		net.compute_Cc_eq(Vc, Vtn_c, CIeq, CGeq);
		net.compute_Cbe_eq(Vbe, Vbc, Vtn_be, Vtn_bc, BEIeq, BEGeq1, BEGeq2);
		net.compute_Cbc_eq(Vbc, Vtn_bc, BCIeq, BCGeq);
		/*
		printf("Stamping transient %s\n", net.name.c_str());
	printf("\
	    CIeq  = %25.15e\n\
	    CGeq  = %25.15e\n\
	    BEIeq = %25.15e\n\
	    BEGeq1= %25.15e\n\
	    BEGeq2= %25.15e\n\
	    BCIeq = %25.15e\n\
	    BCGeq = %25.15e\n", CIeq, CGeq, BEIeq, BEGeq1, BEGeq2, BCIeq, BCGeq);
	    */
		double Sbb = BEGeq1 + BEGeq2 + BCGeq, 
		       Sbc = -BEGeq2 - BCGeq,
		       Sbe = BEGeq1,
		       Scb = -BCGeq,
		       Scc = CGeq + BCGeq,
		       Sce = 0.0,
		       Seb = -BEGeq1 - BEGeq2,
		       Sec = BEGeq2,
		       See = BEGeq1;

		double o[]={Scb, Scc, Sce,
			    Sbb, Sbc, Sbe,
			    Seb, Sec, See};

		/*
		for(int i=0;i<9;i++){
			o[i] *= 1E-12;
		}
		*/

		t.push(c, b, o[0]); t.push(c, c, o[1]); t.push(c, e, o[2]);
		t.push(b, b, o[3]); t.push(b, c, o[4]); t.push(b, e, o[5]);
		t.push(e, b, o[6]); t.push(e, c, o[7]); t.push(e, e, o[8]);
		/*
		printf("\nStamping %s\n", net.name.c_str());
		//printf("Vc = %lf, Vbe = %lf Vbc = %lf\n", Vc, Vbe, Vbc);
		//printf("Vc = %lf, Vbe = %lf Vbc = %lf\n", Vtn_c, Vtn_be, Vtn_bc);
		for(int i=0;i<9;i++){
			if(i%3==0 && i>0) printf("\n");
			printf("%15.8e ", o[i]);
		}
		cout<<endl;
		*/

		double Ic = (- CIeq + BCIeq);
		double Ib = (- BEIeq - BCIeq);
		double Ie = BEIeq;
		//printf("Ic=%25.15e\nIb=%25.15e\nIe=%25.15e\n", Ic,Ib,Ie);
		//printf("Jc=%25.15e\nJb=%25.15e\nJe=%25.15e\n", J[c],J[b],J[e]);
		J[b] += Ib;
		J[c] += Ic;
		J[e] += Ie;
	}
}

void get_node_voltages(Nodelist & nodelist, double * Vsol, double *vs){
	for(int i=1;i<nodelist.size();i++){
		Node & nd = nodelist.nodelist[i];
		int id = nodelist.name2idx[nd.name];
		vs[id]=Vsol[id];
	}
}

// iteratively solve transient analysis
void solve_transient(Netlist & netlist, Nodelist & nodelist, double time){
	// first perform dc analysis to get the DC operating point
	// this is the initial value of V^k_(t_n+1), i.e., V^0_(t_n+1) = V(tn)
	dc_analysis(netlist,nodelist,false);

	int size = nodelist.size();
	size += netlist.netset[VSRC].size();
	size += netlist.netset[VCVS].size();
	size += netlist.netset[CCVS].size();
	double * v = new double[size];
	double * J = new double[size];
	memset((void*)v, 0, sizeof(double)*size);

	double * Vold, // stores V^(k-1)_t(n+1)
	       * Vnew, // stores V^k_t(n+1)
	       * Vtn;  // stores V(tn)
	Vold = new double[size];
	Vnew = new double[size];
	Vtn = new double[size];
	memset((void*)Vold, 0, sizeof(double)*size);
	memset((void*)Vnew, 0, sizeof(double)*size);
	memset((void*)Vtn, 0, sizeof(double)*size);

	update_BJT_currents(netlist, nodelist);
	copy_voltages(nodelist, Vtn); // save Vtn
	int counter = 0;
	double diff = 0.0;
	do{// NR-iteration
		counter++;
		copy_voltages(nodelist, Vold); // get V^(k-1)_t(n+1)

		Triplet t;
		memset((void*)J, 0, sizeof(double)*size);
		memset((void*)v, 0, sizeof(double)*size);
		stamp_resistor(netlist, nodelist, t);
		int ct = nodelist.size();
		stamp_vsrc(netlist, nodelist, t, J, TRAN, ct, time);

		stamp_BJT_DC(netlist, nodelist, t, J);
		/*
		J[0] = 0;
		cout<<" === before === "<<endl;
		for(int i=0;i<size;i++)
			printf("%d: %15.8e\n", i, J[i]);
			*/
		stamp_BJT_TRAN(netlist, nodelist, time, Vold, Vtn, t, J);
		J[0] = 0;

		/*
		t.merge();t.merge();
		t.output();
		cout<<" === after === "<<endl;
		for(int i=0;i<size;i++)
			printf("%d: %25.15e\n", i, J[i]);
		cout<<endl;
		*/
		solve_dc(t, v, J, size);

		get_node_voltages(nodelist, v, Vnew); 
		update_node_voltages(nodelist, v); // update V^k_t(n+1)
		update_BJT_currents(netlist, nodelist);
		//nodelist.output_node_voltages();
		diff = voltage_diff(Vnew,Vold,size);
//		cout<<"Iteration "<<counter<<" diff="<<diff<<endl;
	}while(diff > EPSILON && counter < MAX_ITERATION);

	if( diff > EPSILON && counter >= MAX_ITERATION)
		report_exit("Exceeding max iteration time, not converging!\n");

	delete [] v;
	delete [] J;
	delete [] Vold;
	delete [] Vnew;
	delete [] Vtn;
}

void transient_analysis(Netlist & netlist,Nodelist &nodelist){
	cout<<" ** Transient analysis begins ** "<<endl;
	vector<FILE *> fplot;
	open_tran_plot_files(nodelist,fplot);
	//g_end_tran = g_step_tran;
	for(double time = g_step_tran; time <= g_end_tran; time+=g_step_tran){
		//cout<<"time="<<time<<endl;
		solve_transient(netlist, nodelist, time);
		plot_transient(fplot, nodelist, time);
	}
	close_tran_plot_files(fplot);
}
