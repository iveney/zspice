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
#include <set>
#include <iomanip>
#include <cassert>
#include "net.h"
using namespace std;

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
Net::Net(NODETYPE t, string n, string collect, string base, string em, POLARITY pol):
	type(t), name(n), emit(em), polarity(pol){
	nbr[0]=collect;
       	nbr[1]=base;
	this->BJT_init();
}

void Net::set(NODETYPE type, string name, string node1, string node2, double value){
	this->type = type;
	this->name = name;
	nbr[0]=node1;
	nbr[1]=node2;
	this->value = value;
}

void Net::BJT_init(){
	Ic = Ib = 0.0;
	for(int i=1;i<=3;i++)
		hc[i]=hb[i] = 0.0;
}

double Net::compute_Ic(double Vc, double Vb, double Ve){
	return this->Ic = (hc[1]+hc[2]) * Vb - hc[2] * Vc - hc[1] * Ve + hc[3];
}

double Net::compute_Ib(double Vc, double Vb, double Ve){
	return this->Ib = (hb[1]+hb[2]) * Vb - hb[2] * Vc - hb[1] * Ve + hb[3];
}

// opeartor [] overload
Net & Netlist::operator [](string name){
	return netlist[name];
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
