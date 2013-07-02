/* PaniniGeneral.c		15Jan2010 TKS

Copyright (c) 2010, Thomas K Sharpless
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This is the reference implementation of the General Pannini
Projection, an elaboration of the basic Pannini projection 
discovered by Bruno Postle and Thomas Sharpless in December 
2008 in paintings by Gian Paolo Pannini (1691-1765).

*/
#include "PaniniGeneral.h"

#include <math.h>

#define PI 3.1415926535897932384626433832795
#define D2R( x ) ((x) * PI / 180 )
#define R2D( x ) ((x) * 180 / PI )

int panini_general_toSphere	( double* lon, double* lat, 
							  double h,  double v, 
							  double d, double top, double bot
							 )
{ 
	double S, cl, q, t;

	if( d < 0 ) return 0;
	q = v < 0 ? top : bot;
	if( h == 0 ){
        *lon = 0;
		S = 1;
		cl = 1;
	} else {
	/* solve quadratic for cosine of azimuth angle */
        double k, kk, dd, del;
        k = fabs(h) / (d + 1);
        kk = k * k;
        dd = d * d;
        del = kk * kk * dd - (kk + 1) * (kk * dd - 1);
        if( del < 0 ) 
            return 0;
        cl = (-kk * d + sqrt( del )) / (kk + 1);
	/* use that to compute S, and angle */
        S = (d + cl)/(d + 1);
		*lon = atan2( S * h, cl );
    }
	*lat = atan(S * v);

  /* squeeze */
	if( q > 0 ){
	/* hard squeeze */
		t = atan( v * cl );
		t = q * (t - *lat);
		*lat += t;
	} else if( q < 0 ){
	/* soft squeeze version 2 */
		double cc = cos(0.92 * *lon) - 1;
		double ss = 2 * d / (d + 1);
		t = v / (1 + ss * q * cc );
		*lat = atan( S * t );
	}

	return 1;
}
int panini_general_toPlane	( double lon, double lat, 
							  double*  h,  double*  v, 
							  double d, double top, double bot
							 )
{
	double S, q, t;

	if( d < 0 ) return 0;

	S = (d + 1) / (d + cos(lon));
	*h = sin(lon) * S;
	*v = tan(lat) * S;
    
  /* squeeze */
	q = lat < 0 ? top : bot;
	if( q < 0 ){
	/* soft squeeze version 2 */
		double cc = cos(0.92 * lon) - 1;
		double ss = 2 * d / (d + 1);
		*v *= 1 + ss * q * cc;
	} else if( q > 0 ){
	/* hard squeeze */
		t = cos(lon);
		if( t > 0.01 ) 
			t = *v / t;
		*v += q * (t - *v);
	}


	return 1;
}

int panini_general_maxVAs	( double d, 
							  double maxProj,
							  double * maxView
							 )
{	double a, s;

	if( d < 0 ) return 0;

/* hFOV... */
  // theoretical max angle (infeasible for d < 1.1 or so)
	if( d > 1. )
		s =  -1/d;
	else 
		s = -d;  
	a = acos( s );
  // actual limit may be max projection angle...
	s = asin(d * sin(maxProj)) + maxProj ; 
	if( a > s ){	// clip to projection angle limit
		a = s;	
	} 
    maxView[0] = a;

/* vFOV... */
	maxView[1] = maxProj;

	return 1;
}
