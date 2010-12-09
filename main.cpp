// ----------------------------------------------------------------//
// Filename : main.cpp
// Author : Zigang Xiao <zxiao2@illinois.edu>
//
// Program entry.
// ----------------------------------------------------------------//
// - Zigang Xiao - Mon Oct 25 22:56:38 CDT 2010
//   * added part of phase 2, lack of BJT
//
// - Zigang Xiao - Sun Sep 19 17:16:05 CDT 2010
//   * Finish phase 1 linear dc analysis
//   * added file existence detection
// 
// - Zigang Xiao - Sun Sep 12 23:57:25 CDT 2010
//   * first create the file

#include <iostream>
#include <set>
#include <string>
#include "util.h"
#include "net.h"
#include "node.h"
#include "io.h"
#include "dc_linear.h"
#include "ac_analysis.h"
#include "main.h"
using namespace std;

ANALYSIS_TYPE g_atype = DC; // default to DC
char * g_filename = NULL;
string g_basename;
double g_vin=-1.0;
vector<string> g_plot_gain_node;
vector<string> g_plot_phase_node;
vector<string> g_plot_vol_node;
double g_init_f=0;
double g_end_f=0;
double g_step_f=0;

void output_netlist_info(Netlist & netlist, Nodelist & nodelist){
	cout<<endl;
	cout<<"** Net information **"<<endl;
	cout<<netlist<<endl;
	cout<<"** Node information **"<<endl;
	cout<<nodelist<<endl;
}

int main(int argc, char *argv[]){
	if(argc < 2)
		report_exit("Usage: zspice netlist\n");
	g_filename = argv[1];
	g_basename = get_basename(g_filename);

	Netlist netlist;
	Nodelist nodelist;
	read_netlist(g_filename, netlist, nodelist);

	switch(g_atype){
	case DC: // DC analysis
		cout<<"** DC analysis **"<<endl<<endl;
		output_netlist_info(netlist,nodelist);
		dc_analysis(netlist,nodelist,true);
		break;
	case AC: // AC analysis
		cout<<"** AC analysis **"<<endl<<endl;
		//output_netlist_info(netlist,nodelist);
		ac_analysis(netlist,nodelist);
		break;
	case TRAN:
		cout<<"** Transient analysis **"<<endl;
		break;
	default:
		cout<<"Unknown type. ";
		break;
	}

	return 0;
}
