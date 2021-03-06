// ----------------------------------------------------------------//
// Filename : node.cpp
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// implementation file of node.h
// ----------------------------------------------------------------//
// - Zigang Xiao - Wed Sep 15 15:48:58 CDT 2010
//   * Added the class Node and Nodelist

#include <iomanip>
#include "node.h"
using namespace std;

Node::Node(string n):name(n),v(0.0){}

void Node::insert_net(const Net & net){
	nets.push_back(net.name);
}

void Node::insert_net(const string & netname){
	nets.push_back(netname);
}


// output a node
ostream & operator <<(ostream & s, Node & node){
	s<<std::left<<setw(3)<<node.name<<std::right<<": [ ";
	VecStr::iterator it;
	for(it=node.nets.begin(); it!=node.nets.end(); it++)
		s<<*it<<" ";
	cout<<"]";
	return s;
}

Nodelist::Nodelist(){
	Node ground("0");
	this->insert_node(ground);
}

bool Nodelist::insert_node(const Node & node){
	if( name2idx.find(node.name) != name2idx.end() )
		return false;

	// avoid inserting duplicate node
	int idx = nodelist.size();
	name2idx[node.name] = idx;
	idx2name[idx] = node.name;
	nodelist.push_back(node);
	return true;
}

Node & Nodelist::operator [] (string name){
	return nodelist[name2idx[name]];
}

// return the size of the Nodelist
const int Nodelist::size(){
	return nodelist.size();
}

// output the voltages of nodes
void Nodelist::output_node_voltages(){
	int i;
	int width = 7;
	// node voltages
	for(i=0;i<this->size();i++){
		Node & nd = this->nodelist[i];
		//cout<<"voltage at "<<setw(width)
		//    <<nd.name<<" = "<<scientific<<nd.v<<endl;
		printf("%s  %.5e\n",nd.name.c_str(), nd.v);
	}

}

// output a node list
ostream & operator <<(ostream & s, Nodelist & nlist){
	vector<Node>::iterator it;
	for(it=nlist.nodelist.begin();it!=nlist.nodelist.end();it++){
		s<<"("<<nlist.name2idx[(*it).name]
			<<") "<<*it<<endl;
	}
	return s;
}


