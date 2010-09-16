#include <stdio.h>
#include "dc_linear.h"
#include "umfpack.h"
#include "util.h"

const int    n = 3 ;

/*
double  Y[][]={
	{0.2, -0.2, 1},
	{-0.2,-0.2,1},
	{-1,1,0}
};
*/
double J[]={0,0,1};
double v[3];


int main (void) {
	double **Y = new_square_matrix(n);
	Y[0][0] = 1; Y[0][1] = 0; Y[0][2] = 0; 
	Y[1][0] = 0; Y[1][1] = 0.2; Y[1][2] = 1; 
	Y[2][0] = 0; Y[2][1] = 1; Y[2][2] = 0; 
	int nz = count_entry(Y,n);
	int Ti[nz];
	int Tj[nz];
	double Tx[nz];
	matrix_to_triplet(Ti,Tj,Tx,nz,Y,n);

	int n_row = n;
	int n_col = n;
	int * Ap = new int [n_col+1];
	int * Ai = new int [nz];
	double * Ax = new double [nz];

	int status;
	double Control [UMFPACK_CONTROL];
	umfpack_di_defaults (Control) ;
	status = umfpack_di_triplet_to_col(n_row, n_col, nz, Ti, Tj, Tx, 
			Ap, Ai, Ax, (int *) NULL);
	
	if( status < 0 ) {
		umfpack_di_report_status (Control, status) ;
		report_exit("umfpack_zi_triplet_to_col failed\n") ;
	}


	double *null = (double *) NULL ;
	int i ;
	void *Symbolic, *Numeric ;
	(void) umfpack_di_symbolic (n, n, Ap, Ai, Ax, &Symbolic, null, null) ;
	(void) umfpack_di_numeric (Ap, Ai, Ax, Symbolic, &Numeric, null, null) ;
	umfpack_di_free_symbolic (&Symbolic) ;
	(void) umfpack_di_solve (UMFPACK_A, Ap, Ai, Ax, v, J, Numeric, null, null) ;
	umfpack_di_free_numeric (&Numeric) ;
	for (i = 0 ; i < n ; i++) printf ("v [%d] = %lf\n", i, v[i]) ;

	delete [] Ap;
	delete [] Ai;
	delete [] Ax;
	return (0) ;
}
