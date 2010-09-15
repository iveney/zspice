#include "node.h"

Node::Node(string n):name(n){}

void Node::insert_net(const Net & net){
	nets.push_back(net);
}

void Node::insert_node(const Node & node){
	nodelist.push_back(node);
}
