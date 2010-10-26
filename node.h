// ----------------------------------------------------------------//
// Filename : node.h
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// Definition of nodes
// ----------------------------------------------------------------//
// - Zigang Xiao - Wed Sep 15 15:48:58 CDT 2010
//   * Added the classes Node and Nodelist
// - Zigang Xiao - Wed Sep 15 11:46:48 CDT 2010
//   * first create the file

#ifndef __NODE_H__
#define __NODE_H__

#include <vector>
#include <ext/hash_map>
#include <iostream>
#include "net.h"
using namespace std;
using namespace __gnu_cxx;

class Node{
public:
	Node(string name);
	void insert_net(const Net & net);
	void insert_net(const string & netname);
	friend ostream & operator <<(ostream & s, Node & node);
	string name;
	vector<string> nets;
	double v; // voltage
};

class Nodelist{
public:
	Nodelist();
	bool insert_node(const Node & node);
	hash_map<string, int> name2idx;
	hash_map<int, string> idx2name;
	friend ostream & operator <<(ostream & s, Nodelist & nlist);
	Node & operator [] (string name);
	const int size();
	vector<Node> nodelist;
	void output_node_voltages();
};

#endif
