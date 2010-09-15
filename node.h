// ----------------------------------------------------------------//
// Filename : node.h
// Author : Xiao Zigang <zxiao2@illinois.edu>
//
// Definition of nodes
// ----------------------------------------------------------------//
// - Zigang Xiao - Wed Sep 15 11:46:48 CDT 2010
//   * first create the file

#ifndef __NODE_H__
#define __NODE_H__

#include <vector>
#include "net.h"
using std::vector;

class Node{
public:
	Node(string name);
	void insert_net(const Net & net);
private:
	vector<string> nets;
	string name;
};

class Nodelist{
public:
	void insert_node(const Node & node);
private:
	vector<Node> nodelist;
};

#endif
