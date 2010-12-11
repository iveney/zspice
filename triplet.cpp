// ----------------------------------------------------------------//
// Filename : dc_linear.cpp
// Author : Zigang Xiao <zxiao2@illinois.edu>
//
// class definition of triplet
// ----------------------------------------------------------------//
// - Zigang Xiao - Fri Oct 29 16:43:17 CDT 2010
//   * created file
#include <iostream>
#include <iomanip>
#include "triplet.h"
using namespace std;

// Y[0][0] is always 1, hence add this when initializing
Triplet::Triplet(){
	Ti.push_back(0);
	Tj.push_back(0);
	Tx.push_back(1.0);
	Tz.push_back(0.0);
}

void Triplet::output(){
	for(int i=0;i<size();i++){
		printf("[%d, %d] %25.15lf\n", Ti[i], Tj[i], Tx[i]);
	}
}

// stupidly merge elements at the same position
void Triplet::merge(){
	for(int k=0;k<size();k++){
		for(int l=k+1;l<size();l++){
			if( Ti[k] == Ti[l] && Tj[k] == Tj[l] ){
				/*
				if( Tj[k]==5 && Tj[l] == 5){
					cout<<"merge ";
					cout<<k<<" "<<l<<endl;
				}
				*/
				Tx[k] += Tx[l];
				Tz[k] += Tz[l];
				Ti.erase(Ti.begin()+l);
				Tj.erase(Tj.begin()+l);
				Tx.erase(Tx.begin()+l);
				Tz.erase(Tz.begin()+l);
			}
		}
	}
}

// insert a triplet 
void Triplet::push(int i, int j, double x, double z){
	// NOTE: cross-out the zero-th row and column
	// 	 do not stamp them
	if( i == 0 || j == 0 ) return; 
	//if( i == 1 && j == 1 )
	//	printf("Pushing to [1, 1] by: %25.15lf\n", x);
	Ti.push_back(i);
	Tj.push_back(j);
	Tx.push_back(x);
	Tz.push_back(z);
}

// return the number of elements in a triplet instance
int Triplet::size(){return Ti.size();}

////////////////////////////////////////////////////////////////////////////

