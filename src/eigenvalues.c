//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: walus@atips.ca                                //
// WEB: http://www.atips.ca/projects/qcadesigner/       //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way.                        //
// Any independent files you generate format as you wish//
//////////////////////////////////////////////////////////
// Please use complete names in variables and fucntions //
// This will reduce ramp up time for new people trying  //
// to contribute to the project.                        //
//////////////////////////////////////////////////////////

#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "nrutil.h"
#define ROTATE(a,i,j,k,l) g=a[i][j]; h=a[k][l]; a[i][j]=g-s*(h+g*tau); a[k][l]=h+s*(g-h*tau);


/*
Computes all eigenvalues and eigenvectors of a real symmetric matrix a[0..n-1][0..n-1]. On
output, elements of a above the diagonal are destroyed. d[0..n-1] returns the eigenvalues of a.
v[0..n-1][0..n-1] is a matrix whose columns contain, on output, the normalized eigenvectors of
a. nrot returns the number of Jacobi rotations that were required.
*/

void jacobi(float **a, int n, float *d, float **v, int *nrot){
	
	int j,iq,ip,i;
	float tresh,theta,tau,t,sm,s,h,g,c,*b,*z;

	b=vector(0,n-1);
	z=vector(0,n-1);

	assert(v!=NULL);
	// Initialize to the identity matrix.
	for (ip = 0; ip < n; ip++){ 
		for (iq = 0; iq < n; iq++)v[ip][iq]=0.0; 
		v[ip][ip]=1.0;
		}

	//Initialize b and d to the diagonal of a. 
	for (ip = 0; ip < n; ip++) { 
		b[ip]=d[ip]=a[ip][ip];
		z[ip]=0.0; 
		}

	*nrot=0;

	for (i=1;i<=50;i++) {
		sm=0.0;
		
		for (ip=0;ip<n-1;ip++) {
			for (iq=ip+1;iq<n;iq++)
				sm += fabs(a[ip][iq]);
		}

		if (sm == 0.0) {
			free_vector(z,0,n-1);
			free_vector(b,0,n-1);
			return;
		}

		if (i < 4)
			tresh=0.2*sm/(n*n);
		else
			tresh=0.0;

		for (ip=0;ip<n-1;ip++) {
			for (iq=ip+1;iq<n;iq++) {
				g=100.0*fabs(a[ip][iq]);
				
				if (i > 4 && (float)(fabs(d[ip])+g) == (float)fabs(d[ip]) && (float)(fabs(d[iq])+g) == (float)fabs(d[iq]))
					a[ip][iq]=0.0;
				else if (fabs(a[ip][iq]) > tresh) {
					h=d[iq]-d[ip];
				
				if ((float)(fabs(h)+g) == (float)fabs(h)) t=(a[ip][iq])/h;
				
				else {
					theta=0.5*h/(a[ip][iq]);
					t=1.0/(fabs(theta)+sqrt(1.0+theta*theta));
					if (theta < 0.0) t = -t;
				}
				
				c=1.0/sqrt(1+t*t);
				s=t*c;
				tau=s/(1.0+c);
				h=t*a[ip][iq];
				z[ip] -= h;
				z[iq] += h;
				d[ip] -= h;
				d[iq] += h;
				a[ip][iq]=0.0;

				for (j=0;j<ip-1;j++) {
					ROTATE(a,j,ip,j,iq)
				}
				
				for (j=ip+1;j<iq-1;j++) {
					ROTATE(a,ip,j,j,iq)
				}

				for (j=iq+1;j<n;j++) {
					ROTATE(a,ip,j,iq,j)
				}
				
				for (j=0;j<n;j++) {
					ROTATE(v,j,ip,j,iq)
				}
				++(*nrot);
				}
			}
		}
		for (ip=0;ip<n;ip++) {
			b[ip] += z[ip];
			d[ip]=b[ip];
			z[ip]=0.0;
		}
	}

	nrerror("Too many iterations in routine jacobi");
}
