// ----------------------------------------------------------------//
// Filename : dc_nonlinear.cpp
// Author : Zigang Xiao <zxiao2@illinois.edu>
//
// Function definition for non-linear dc analysis, mainly newton-raphson
// ----------------------------------------------------------------//
// - Zigang Xiao - Fri Oct 29 17:00:24 CDT 2010
//   * Finish writing NR iteration function
//   * resctructured file
//
// - Zigang Xiao - Mon Oct 25 12:45:28 CDT 2010
//   * first create the file
//
#include <cmath>
#include <ext/hash_map>
#include "dc_nonlinear.h"
#include "node.h"
#include "net.h"
#include "global.h"
using namespace __gnu_cxx;

extern hash_map<string,int> net2int;

// stamp the non-linear devices
// NOTE: some terms rely on the value of last time
bool stamp_nonlinear(Netlist & netlist, Nodelist & nodelist, 
		Triplet & t, double * J){
	Net net;

	bool success = true;
	// ** linearize non-linear devices: Diode **
	foreach_net_in(netlist, DIODE, net){
		int k = nodelist.name2idx[net.nbr[0]];
		int l = nodelist.name2idx[net.nbr[1]];
		Node & nd1 = nodelist[net.nbr[0]];
		Node & nd2 = nodelist[net.nbr[1]];
		double Vd = nd1.v - nd2.v;
		double e = exp(Vd/Vt);
		double Geq = Is * e / Vt;
		double Ieq = Is * (e-1.0) - Vd*Geq;
		t.push(k,k,Geq);
		t.push(l,l,Geq);
		t.push(k,l,-Geq);
		t.push(l,k,-Geq);
		J[k] += -Ieq;
		J[l] += Ieq;
	}

	// ** linearize non-linear devices: BJT **
	set<string>::iterator it;
	set<string> & bjts = netlist.netset[BJT];
	for(it=bjts.begin(); it!=bjts.end(); ++it){
		Net & net = netlist[*it];
		//cout<<"stamping "<<net.name<<" p = "<<net.polarity<<endl;

		// find the names of three terminals
		string clct = net.nbr[0];
		string base = net.nbr[1];
		string emit = net.emit;
		//cout<<"BJT c="<<clct<<" b="<<base<<" e="<<emit<<endl;

		// find the index of c,b,e
		int c = nodelist.name2idx[clct];
		int b = nodelist.name2idx[base];
		int e = nodelist.name2idx[emit];
		//cout<<"index c="<<c<<" b="<<b<<" e="<<e<<endl;

		double Vc = nodelist[clct].v;
		double Vb = nodelist[base].v;
		double Ve = nodelist[emit].v;
		//cout<<"Vol "<<clct<<"="<<Vc
		//   <<" "<<base<<"="<<Vb
		//  <<" "<<emit<<"="<<Ve<<endl;

		double Vbc = Vb - Vc;
		double Vbe = Vb - Ve;
		if( net.polarity == PNP ){
			Vbc = -Vbc;
			Vbe = -Vbe;
		}
		//cout<<"Vbc="<<Vbc<<" Vbe="<<Vbe<<endl;

		// get Ic, Ib from last iteration
		double Ib = net.Ib;
		double Ic = net.Ic;
		//cout<<"Ib="<<Ib<<" Ic="<<Ic<<endl;

		// construct h_{c1}^{k-1}
		double hc1 = 
			 Is * (1. - Vbc / VAf - Vbe / VAr) * exp(Vbe / Vt) / Vt
		       - Is * (exp(Vbe / Vt) - exp(Vbc / Vt)) / VAr;
		
		// construct h_{c2}^{k-1}
		double hc2 = 
		      - Is * (exp(Vbe / Vt) - exp(Vbc / Vt)) / VAf 
		      - Is * (1. - Vbc / VAf - Vbe / VAr) * exp(Vbc / Vt) / Vt
		      - Is * exp(Vbc / Vt) / Br / Vt;

		// construct h_{c3}^{k-1}
		double hc3 = Ic - hc1 * Vbe - hc2 * Vbc;
		//cout<<"Hc 1="<<hc1<<" 2="<<hc2<<" 3="<<hc3<<endl;

		double hb1 = Is * exp(Vbe / Vt) / Vt / Bf; // h_{b1}^{k-1}
		double hb2 = Is * exp(Vbc / Vt) / Vt / Br; // h_{b2}^{k-1}
		double hb3 = Ib - hb1 * Vbe - hb2 * Vbc;
		//cout<<"Hb 1="<<hb1<<" 2="<<hb2<<" 3="<<hb3<<endl;

		// finally, start to stamp
		double Scb = hc1 + hc2,  Scc = -hc2,       Sce = -hc1;
		double Sbb = hb1 + hb2,  Sbc = -hb2,       Sbe = -hb1;
		double Seb = -(Sbb+Scb), Sec = -(Sbc+Scc), See = -(Sbe+Sce);

		t.push(c, b, Scb); t.push(c, c, Scc); t.push(c, e, Sce);
		t.push(b, b, Sbb); t.push(b, c, Sbc); t.push(b, e, Sbe);
		t.push(e, b, Seb); t.push(e, c, Sec); t.push(e, e, See);

		if( net.polarity == NPN ){
			J[c] += -hc3;       
			J[b] += -hb3;      
			J[e] +=  hc3 + hb3; 
		}
		else{
			J[c] += hc3;       
			J[b] += hb3;      
			J[e] += -(hc3 + hb3); 
		}

		// update BJT value
		net.hb[1]=hb1; net.hb[2]=hb2; net.hb[3]=hb3;
		net.hc[1]=hc1; net.hc[2]=hc2; net.hc[3]=hc3;
		//cout<<endl;
	}
	return success;
}


// calculate the voltage difference between two voltage vectors
double voltage_diff(double * Vnew, double * Vold, int n){
	double r=0.0,tmp;
	for(int i=0;i<n;i++) {
	//	cout<<i<<" : "<<Vold[i]<<" | "<<Vnew[i]<<endl;
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

// update the BJT currents according to analytical form
void update_BJT_currents(Netlist & netlist, Nodelist & nodelist){
	set<string>::iterator it;
	set<string> & bjts = netlist.netset[BJT];
	for(it=bjts.begin(); it!=bjts.end(); ++it){
		Net & net = netlist[*it];

		// find the names of three terminals
		string clct = net.nbr[0];
		string base = net.nbr[1];
		string emit = net.emit;

		double Vc = nodelist[clct].v;
		double Vb = nodelist[base].v;
		double Ve = nodelist[emit].v;

		// new below
		double Vbc = Vb - Vc;
		double Vbe = Vb - Ve;

		if( net.polarity == PNP ){
			Vbc = -Vbc;
			Vbe = -Vbe;
		}

		net.Ic = Is * 
			(  (1 - Vbc / VAf - Vbe / VAr) * 
			   (exp(Vbe / Vt) - exp(Vbc / Vt))
			 - (exp(Vbc / Vt) - 1) / Br );


		net.Ib = Is * 
			(  (exp(Vbe / Vt) - 1) / Bf 
			 + (exp(Vbc / Vt) - 1) / Br );
	}
}

// performs Newton-Raphson iteration here
double NR_iteration(Netlist & netlist, Nodelist & nodelist,
		double *v, double *J, int size, bool output){
	if( output == true ){
		// output input node information
		cout<<endl<<"Input nodes:"<<endl;
		nodelist.output_node_voltages();
		cout<<endl<<"Iteration begins:"<<endl;
	}

	// backup the voltages
	double * Vold, *Vnew;
	Vold = new double[nodelist.size()];
	Vnew = new double[nodelist.size()];

	// initialize BJT branch currents
	update_BJT_currents(netlist, nodelist);
	
	int counter=0;
	double diff=0;
	do{
		// backup old value of voltages to Vold
		copy_voltages(nodelist, Vold);

		// linearize the non-linear device, stamp and solve
		dc_core(netlist,nodelist,v,J,size);

		// update new value of voltages
		update_node_voltages(nodelist, v);
		
		// copy new value of voltages to Vnew
		copy_voltages(nodelist, Vnew);

		// update new value of currents (Ib, Ic) for next iteration
		update_BJT_currents(netlist, nodelist);

		diff = voltage_diff(Vnew,Vold,nodelist.size());
		if( output == true ){
			cout<<"iteration: "<<++counter
			    <<", difference = "<<diff<<endl;
		}
		//nodelist.output_node_voltages();
	}while(diff>EPSILON);
	
	delete [] Vold;
	delete [] Vnew;

	if( output == true ){
		cout<<"Total number of iterations: "<<counter<<endl;
		// finally, output again
		cout<<"\nResult:"<<endl;
		nodelist.output_node_voltages();
		netlist.output_branch_currents(net2int,v);
	}

	return counter;
}
