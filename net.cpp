// ----------------------------------------------------------------//
// Filename : net.cpp
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// implementation file of net.h
// ----------------------------------------------------------------//
// - Zigang Xiao - Wed Sep 15 15:48:58 CDT 2010
//   * change output to standard c++ stream output

#include <string>
#include <iostream>
#include <tr1/unordered_map>
#include <set>
#include <iomanip>
#include <cassert>
#include <cmath>
#include "net.h"
using namespace std;
using namespace std::tr1;

extern double g_step_tran;

// initialize the type to be UNDEF
Net::Net():type(UNDEF){
}


void Net::set_voltage(VOL_TYPE vtype, double value, double offset,
		double amplitude, double freq){
	this->vtype = vtype;
	this->value = value;
	this->offset = offset;
	this->amplitude = amplitude;
	this->freq = freq;
}

// constructor for resistor, vsrc, csrc, cap, ind, diode
Net::Net(NODETYPE t, string n, string node1, string node2, double v):
	type(t), name(n), value(v), current(0.0){
	// for diode, do not need to provide parameter v
	nbr[0]=node1;
       	nbr[1]=node2;
}

// constructor for vccs, vcvs
Net::Net(NODETYPE t, string n, string node1, string node2,
		string ctrl1, string ctrl2, double v):
	type(t), name(n), value(v), current(0.0){
	nbr[0]=node1;
       	nbr[1]=node2;
	ctrl[0]=ctrl1;
	ctrl[1]=ctrl2;
}

// constructor for ccvs, cccs
Net::Net(NODETYPE t, string n, string node1, string node2, string vy, double v): 
	type(t), name(n), vyyy(vy), value(v), current(0.0){
	nbr[0]=node1;
       	nbr[1]=node2;
}

// constructor for bjt
Net::Net(NODETYPE t, string n, string collect, string base, 
		string em, POLARITY pol):
	type(t), name(n), emit(em), polarity(pol){
	nbr[0]=collect;
       	nbr[1]=base;
	this->init_BJT();
}

void Net::set(NODETYPE type, string name, string node1, string node2, double value){
	this->type = type;
	this->name = name;
	nbr[0]=node1;
	nbr[1]=node2;
	this->value = value;
}

void Net::init_BJT(){
	Ic = Ib = 0.0;
	for(int i=1;i<=3;i++)
		hc[i]=hb[i] = 0.0;
}

double Net::compute_Ic(double Vc, double Vb, double Ve){
//	cout<<"update IC from "<< this->Ic;
	Ic = (hc[1]+hc[2]) * Vb - hc[2] * Vc - hc[1] * Ve;
	Ic += hc[3];
//	cout<<" to "<<Ic<<endl;
	return Ic;
}

double Net::compute_Ib(double Vc, double Vb, double Ve){
//	cout<<"update IB from "<<Ib;
	Ib = (hb[1]+hb[2]) * Vb - hb[2] * Vc - hb[1] * Ve;
	Ib += hb[3];
//	cout<<" to "<<Ib<<endl;
	return Ib;
}

double Net::compute_Cc(double Vc){
	double Cc;
	if(Vc < Fc * Vj) 
		Cc = Cjcs*pow(1-Vc/Vj,-Mj);
	else 
		Cc = Cjcs*(Mj*Vc/Vj + 1.0 - Fc*(1+Mj))/pow(1-Fc,1+Mj);
	return Cc;
}

double Net::compute_Cbe(double Vbe, double Vbc){
	double Cbe =   Tf*Is*exp(Vbe/Vt)*(1-Vbc/VAf-Vbe/VAr)/Vt
		    - Tf*Is*(exp(Vbe/Vt)-1)/VAr;
	if(Vbe < Fc * Vj ) 
		Cbe += Cjbe*pow(1-Vbe/Vj,-Mj);
	else 
		Cbe += Cjbe*(Mj*Vbe/Vj + 1.0 - Fc*(1+Mj))/pow(1-Fc,1+Mj);
	return Cbe;
}

double Net::compute_Cbe2(double Vbe, double Vbc){
	return -Tf*Is*(exp(Vbe/Vt)-1)/VAf;
}

double Net::compute_Cbc(double Vbc){
	double Cbc = Tr*Is*exp(Vbc/Vt)/Vt;
	if(Vbc < Fc * Vj)
		Cbc += Cjbc*pow(1-Vbc/Vj,-Mj);
	else
		Cbc += Cjbc*(Mj*Vbc/Vj + 1.0 - Fc*(1+Mj))/pow(1-Fc,1+Mj);
	return Cbc;
}

double Net::compute_dCc(double Vc){
	double q;
	if( Vc < Fc * Vj )
		q = Mj * Cjcs * pow(1-Vc/Vj, -Mj-1);
	else
		q = Mj * Cjcs / Vj / pow(1-Fc, 1+Mj);
	return q;
}

double Net::compute_dCbe(double Vbe, double Vbc){
	double q = - 2 * Tf * Is * exp(Vbe/Vt) / VAr / Vt
		   + Tf * Is * exp(Vbe/Vt) * (1- Vbc/VAf - Vbe/VAr) / Vt / Vt;
	if( Vbe < Fc * Vj)
		q += Mj * Cjbe * pow(1 - Vbe/Vj, -Mj-1) / Vj;
	else
		q += Mj * Cjbe / Vj / pow(1-Fc,1+Mj);
	return q;
}

double Net::compute_dCbe2(double Vbe, double Vbc){
	return - Tf * Is * exp(Vbe/Vt) / VAf / Vt;
}

double Net::compute_dCbc(double Vbc){
	double q = Tr * Is * exp(Vbc/Vt) / Vt / Vt;
	if( Vbc < Fc * Vj )
		q += Mj * Cjbc * pow(1-Vbc/Vj, -Mj-1) / Vj;
	else
		q += Mj * Cjbc / Vj / pow(1-Fc, 1+Mj);
	return q;
}

void Net::compute_Cc_eq(double Vc, double Vtn_c, 
		double &CIeq, double & CGeq){
	double Cc, dCc;
	Cc = compute_Cc(Vc);
	dCc = compute_dCc(Vc);
	//printf("name = %s, Cc = %25.15e dCc = %25.15e\n", name.c_str(), Cc, dCc);
	//printf("Vc=%25.15e timeVc=%25.15e\n", Vc, Vtn_c);
	CIeq = - Vtn_c / g_step_tran * (Cc - dCc * Vc)
	       - Vc * Vc / g_step_tran * dCc;
	CGeq = - Vtn_c / g_step_tran * dCc
	       + 1 / g_step_tran * (dCc * Vc + Cc);
	//printf("name = %s, CIeq = %25.15e CGeq = %25.15e\n", name.c_str(), CIeq, CGeq);
}

void Net::compute_Cbe_eq(double Vbe, double Vbc,
		double Vtn_be, double Vtn_bc,
		double & BEIeq, double & BEGeq1, double & BEGeq2){
	double Cbe1, dCbe1, dCbe2;
	Cbe1 = compute_Cbe(Vbe, Vbc);
	dCbe1 = compute_dCbe(Vbe, Vbc);
	dCbe2 = compute_dCbe2(Vbe, Vbc);
	//printf("Cbe1 = %25.15e\n dcbe1=%25.15e\n dcbe2=%25.15e\n", Cbe1, dCbe1, dCbe2);
	BEIeq = - Vtn_be / g_step_tran * (Cbe1 + dCbe1 * Vbe + dCbe2 * Vbc)
		- Vbe * Vbe / g_step_tran * dCbe1 - Vbe * Vbc / g_step_tran * dCbe2;
	BEGeq1 = - Vtn_be / g_step_tran * dCbe1
		 + 1 / g_step_tran * (dCbe1 * Vbe + Cbe1);
	BEGeq2 = - Vtn_be / g_step_tran * dCbe2 
		 + Vbe / g_step_tran * dCbe2;
}

void Net::compute_Cbc_eq(double Vbc, double Vtn_bc, 
		double &BCIeq, double &BCGeq){
	double Cbc, dCbc;
	Cbc = compute_Cbc(Vbc);
	dCbc = compute_dCbc(Vbc);
	BCIeq = - Vtn_bc / g_step_tran * (Cbc - dCbc * Vbc)
		- Vbc * Vbc / g_step_tran * dCbc;
	BCGeq = - Vtn_bc / g_step_tran * dCbc
		+ 1 / g_step_tran * (dCbc * Vbc + Cbc);
	//printf("Cbc = %25.15e\ndCbc=%25.15e\n", Cbc, dCbc);
	//printf("BCIeq = %25.15e\nBCGeq=%25.15e\n", BCIeq, BCGeq);
}

// opeartor [ ] overload
Net & Netlist::operator [](string name){
	return netlist[name];
}

// output branch currents
void Netlist::output_branch_currents(unordered_map<string,int> & net2int, 
		double * v){
	// output branch currents of Voltage source, VCVS and CCVS
	Net net;
	const int set_types[] = {VSRC, VCVS, CCVS};
	int nn= sizeof(set_types)/sizeof(int);
	for(int i=0;i<nn;i++){
		foreach_net_in(*this, set_types[i], net){
			int id = net2int[net.name];
			cout<<"current throught source "<<net.name<<" = "
				<<scientific<<v[id]<<endl;
		}
	}

	// for CCCS, it is controlled by some vyyy, then just compute it
	foreach_net_in(*this, CCCS, net){
		int id = net2int[net.vyyy];
		cout<<"current throught source "<<net.name<<" = "
			<<scientific<<net.value*v[id]<<endl;
	}
	cout<<endl;
}


// categorized output
ostream & operator <<(ostream & s, Netlist & l){
	set<string>::iterator it;
	for(int i=0;i<NUM_NETTYPE;i++){
		s<<setw(15)<<std::left<<nettype_str[i]<<": ["<<std::right;
		for(it=l.netset[i].begin();it!=l.netset[i].end();it++)
			s<<" "<<*it;
		s<<" ]"<<endl;
	}
	return s;
}
