#include <string>
#include <iostream>
#include "net.h"
#include "global.h"
using std::string;
using std::cout;

Net::Net():type(UNDEF){
}

// constructor for resistor, vsrc, csrc, cap, ind, diode
Net::Net(NODETYPE t, string n, string node1, string node2, double v=0.0):
	type(t), name(n), value(v){
	// for diode, do not need to provide parameter v
	nbr[0]=node1;
       	nbr[1]=node2;
}

// constructor for vccs, vcvs
Net::Net(NODETYPE t, string n, string node1, string node2,
		string ctrl1, string ctrl2, double v):
	type(t), name(n), value(v){
	nbr[0]=node1;
       	nbr[1]=node2;
	ctrl[0]=ctrl1;
	ctrl[1]=ctrl2;
}

// constructor for ccvs, cccs
Net::Net(NODETYPE t, string n, string node1, string node2,
		string vy, double v): type(t), name(n), vyyy(vy), value(v){
	nbr[0]=node1;
       	nbr[1]=node2;
}

// constructor for bjt
Net::Net(NODETYPE t, string n, string collect, string base, string em, POLARITY pol):
	type(t), name(n), emit(em), polarity(pol){
	nbr[0]=collect;
       	nbr[1]=base;
}

void Net::set(NODETYPE type, string name, string node1, string node2, double value){
	this->type = type;
	this->name = name;
	nbr[0]=node1;
	nbr[1]=node2;
	this->value = value;
}

Net & Netlist::operator [](string name){
	return netlist[name];
}

void Netlist::output(){
	hash_map<string, Net>::iterator iter;
	for(iter=netlist.begin();iter!=netlist.end();iter++){
		cout<<(*iter).first<<endl;
	}
}
