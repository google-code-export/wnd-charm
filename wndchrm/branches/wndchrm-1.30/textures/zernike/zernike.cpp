/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                              mb_Znl.c                                    */
/*                                                                          */
/*                                                                          */
/*                           Michael Boland                                 */
/*                            09 Dec 1998                                   */
/*                                                                          */     
/*  Revisions:                                                              */
/*  9-1-04 Tom Macura <tmacura@nih.gov> modified to make the code ANSI C    */
/*         and work with included complex arithmetic library from           */
/*         Numerical Recepies in C instead of using the system's C++ STL    */
/*         Libraries.                                                       */
/*                                                                          */
/*  1-29-06 Lior Shamir <shamirl (-at-) mail.nih.gov> modified "factorial"  */
/*  to a loop, replaced input structure with ImageMatrix class.             */
/*  2011-04-25 Ilya Goldberg. Optimization due to this function accounting  */
/*    for 35% of the total wndchrm run-time.  Now ~4x faster.               */
/*                                                                          */
/****************************************************************************/


//---------------------------------------------------------------------------

#ifdef WIN32
#pragma hdrstop
#endif

#include <complex>
#include <cmath>
#include <cfloat> // Has definition of DBL_EPSILON
#include <assert.h>
#include <stdio.h>
#include "gsl/specfunc.h"

#include "../../cmatrix.h"
#include "complex.h"

#include "zernike.h"
#define PI 3.14159265358979323846264338328

// This sets the maximum D parameter (15)
// The D parameter has to match MAX_D. See mb_Znl() below.
#define MAX_D 15
// This is based on the maximum D parameter being 15, which sets the number of returned values.
#define MAX_Z 72
// This is also based on the maximum D parameter - contains pre-computed factorials
#define MAX_LUT 240


/* mb_Znl
  Zernike moment generating function.  The moment of degree n and
  angular dependence l for the pixels defined by coordinate vectors
  X and Y and intensity vector P.  X, Y, and P must have the same
  length
*/

void mb_Znl(double *X, double *Y, double *P, int size, double D, double m10_m00, double m01_m00, double R, double psum, double *zvalues, long *output_size) {
	static double LUT[MAX_LUT];
	static int n_s[MAX_Z], l_s[MAX_Z];
	static char init_lut=0;

	double x, y, p ;   /* individual values of X, Y, P */
	int i,m, theZ, theLUT, numZ=0;
	int n=0,l=0;
	using namespace std;

	complex<double> sum [MAX_Z];
	complex<double> Vnl;


// The LUT indexes don't work unless D == MAX_D
// To make it more flexible, store the LUT by [m][n][l].  Needs [(D+1)/2][D+1][D+1] of storage.
// Other hard-coded D values should just need changing MAX_D, MAX_Z and MAX_LUT above.
	assert (D == MAX_D);

	if (!init_lut) {
		theZ=0;
		theLUT=0;
		for (n = 0; n <= MAX_D; n++) {
			for (l = 0; l <= n; l++) {
				if ( (n-l) % 2 == 0 ) {
					for (m = 0; m <= (n-l)/2; m++) {
						LUT[theLUT] = pow((double)-1.0,(double)m) * ( (long double) gsl_sf_fact(n-m) / ( (long double)gsl_sf_fact(m) * (long double)gsl_sf_fact((n - 2.0*m + l) / 2.0) *
							(long double)gsl_sf_fact((n - 2.0*m - l) / 2.0) ) );
						theLUT++;
					}
					n_s[theZ] = n;
					l_s[theZ] = l;
					theZ++;
				}
			}
		}
		init_lut = 1;
	}

// Get the number of Z values, and clear the sums.
	for (n = 0; n <= D; n++) {
		for (l = 0; l <= n; l++) {
			if ( (n-l) % 2 == 0 ) {
				sum [numZ] = complex<double>(0.0,0.0);
				numZ++;
			}
		}
	}

	for(i = 0 ; i < size ; i++) {
		x = (X[i] - m10_m00)/R;
		y = (Y[i] - m01_m00)/R;
		double sqr_x2y2 = sqrt (x*x + y*y);
		if (sqr_x2y2 > 1.0) continue;

		p = P[i] / psum;
		double atan2yx = atan2(y,x);
		theLUT = 0;
		for (theZ = 0; theZ < numZ; theZ++) {
			n = n_s[theZ];
			l = l_s[theZ];
			Vnl = complex<double>(0.0,0.0);
			for( m = 0; m <= (n-l)/2; m++ ) {
				Vnl += ( polar (1.0, l*atan2yx) * LUT[theLUT] * pow( sqr_x2y2, (double)(n - 2*m)) );
				theLUT++;
			}
			sum [theZ] += (conj(Vnl) * p);
		}
	}

	double preal, pimag;
	for (theZ = 0; theZ < numZ; theZ++) {
		sum [theZ] *= ((n_s[theZ]+1)/PI);
		preal = real ( sum [theZ] );
		pimag = imag ( sum [theZ] );
		zvalues[theZ] = fabs(sqrt(preal*preal+pimag*pimag));
	}

	*output_size = numZ;
}


void mb_zernike2D(ImageMatrix *I, double D, double R, double *zvalues, long *output_size) {
	double *Y,*X,*P,psum;
	double intensity;
	int x,y,size;

	*output_size=0;
	int rows = I->height,cols = I->width;
	if (D<=0) D=15;
	if (R<=0) R=rows/2;

	Y=new double[rows*cols];
	X=new double[rows*cols];
	P=new double[rows*cols];

   /* Find all non-zero pixel coordinates and values */
	size=0;
	psum=0;
	double moment10 = 0.0, moment00 = 0.0, moment01 = 0.0;
	for (y=0;y<rows;y++)
		for (x=0;x<cols;x++) {
			intensity = I->pixel(x,y,0).intensity;
			if (intensity != 0) {
				Y[size] = y+1;
				X[size] = x+1;
				P[size] = intensity;
				psum   += intensity;
				size++;
			}
		// moments
			moment10 += (x+1) * intensity;
			moment00 += intensity;
			moment01 += (y+1) * intensity;
		}

   /* Normalize the coordinates to the center of mass and normalize
      pixel distances using the maximum radius argument (R) */

	double m10_m00 = moment10/moment00;
	double m01_m00 = moment01/moment00;
	mb_Znl (X,Y,P,size,D,m10_m00,m01_m00,R,psum,zvalues,output_size);

	delete Y;
	delete X;
	delete P;

}




#ifdef WIN32
#pragma package(smart_init)
#endif


