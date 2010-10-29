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
#include <ext/hash_map>
#include <set>
#include <iomanip>
#include <cassert>
#include "net.h"
using namespace std;
using namespace __gnu_cxx;

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
	//if( polarity == PNP ) Ic = -Ic;
	Ic += hc[3];
//	cout<<" to "<<Ic<<endl;
	return Ic;
}

double Net::compute_Ib(double Vc, double Vb, double Ve){
//	cout<<"update IB from "<<Ib;
	Ib = (hb[1]+hb[2]) * Vb - hb[2] * Vc - hb[1] * Ve;
	//if( polarity == PNP ) Ib = -Ib;
	Ib += hb[3];
//	cout<<" to "<<Ib<<endl;
	return Ib;
}

// opeartor [ ] overload
Net & Netlist::operator [](string name){
	return netlist[name];
}

// output branch currents
void Netlist::output_branch_currents(hash_map<string,int> & net2int, 
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
