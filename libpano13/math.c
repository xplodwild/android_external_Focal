/* Panorama_Tools       -       Generate, Edit and Convert Panoramic Images
   Copyright (C) 1998,1999 - Helmut Dersch  der@fh-furtwangen.de
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/*------------------------------------------------------------*/


#include "filter.h"
#include <float.h>
#include "f2c.h"

#include "PaniniGeneral.h"

#define R_EPS  1.0e-6   
#define MAXITER 100

#include <assert.h>


#ifndef abs
#define abs(a) ( (a) >= 0 ? (a) : -(a) )
#endif

#ifdef _MSC_VER
#define isnan(a) _isnan(a)
#define isinf(a) (_fpclass(a) == _FPCLASS_NINF || _fpclass(a) == _FPCLASS_PINF)
#endif

void    matrix_matrix_mult      ( double m1[3][3],double m2[3][3],double result[3][3]);
int     polzeros_();

void cubeZero( double *a, int *n, double *root );
void squareZero( double *a, int *n, double *root );
double cubeRoot( double x );


//------------------------- Some auxilliary math functions --------------------------------------------

// atanh is not available on MSVC. Use the atanh routine from gsl
#ifdef _MSC_VER

#define GSL_DBL_EPSILON        2.2204460492503131e-16
#define GSL_SQRT_DBL_EPSILON   1.4901161193847656e-08 

static double
log1p (const double x)
{
  volatile double y;
  y = 1 + x;
  return log(y) - ((y-1)-x)/y ;  /* cancels errors with IEEE arithmetic */
} 

static double
atanh (const double x)
{
  double a = fabs (x);
  double s = (x < 0) ? -1 : 1;

  if (a > 1)
    {
      //return NAN;
      return 0;
    }
  else if (a == 1)
    {
      //return (x < 0) ? GSL_NEGINF : GSL_POSINF; 
      return (x < 0) ? -1e300 : 1e300;
    }
  else if (a >= 0.5)
    {
      return s * 0.5 * log1p (2 * a / (1 - a));
    }
  else if (a > GSL_DBL_EPSILON)
    {
      return s * 0.5 * log1p (2 * a + 2 * a * a / (1 - a));
    }
  else
    {
      return x;
    }
} 
#endif

void matrix_mult( double m[3][3], double vector[3] )
{
        register int i;
        register double v0 = vector[0];
        register double v1 = vector[1];
        register double v2 = vector[2];
        
        
        for(i=0; i<3; i++)
        {
                vector[i] = m[i][0] * v0 + m[i][1] * v1 + m[i][2] * v2;
        }
}
                
void matrix_inv_mult( double m[3][3], double vector[3] )
{
        register int i;
        register double v0 = vector[0];
        register double v1 = vector[1];
        register double v2 = vector[2];
        
        for(i=0; i<3; i++)
        {
                vector[i] = m[0][i] * v0 + m[1][i] * v1 + m[2][i] * v2;
        }
}
                
void matrix_matrix_mult( double m1[3][3],double m2[3][3],double result[3][3])
{
        register int i,k;
        
        for(i=0;i<3;i++)
        {
                for(k=0; k<3; k++)
                {
                        result[i][k] = m1[i][0] * m2[0][k] + m1[i][1] * m2[1][k] + m1[i][2] * m2[2][k];
                }
        }
}

// Set matrix elements based on Euler angles a, b, c

void SetMatrix( double a, double b, double c , double m[3][3], int cl )
{
        double mx[3][3], my[3][3], mz[3][3], dummy[3][3];
        

        // Calculate Matrices;

        mx[0][0] = 1.0 ;                                mx[0][1] = 0.0 ;                                mx[0][2] = 0.0;
        mx[1][0] = 0.0 ;                                mx[1][1] = cos(a) ;                     mx[1][2] = sin(a);
        mx[2][0] = 0.0 ;                                mx[2][1] =-mx[1][2] ;                   mx[2][2] = mx[1][1];
        
        my[0][0] = cos(b);                              my[0][1] = 0.0 ;                                my[0][2] =-sin(b);
        my[1][0] = 0.0 ;                                my[1][1] = 1.0 ;                                my[1][2] = 0.0;
        my[2][0] = -my[0][2];                   my[2][1] = 0.0 ;                                my[2][2] = my[0][0];
        
        mz[0][0] = cos(c) ;                     mz[0][1] = sin(c) ;                     mz[0][2] = 0.0;
        mz[1][0] =-mz[0][1] ;                   mz[1][1] = mz[0][0] ;                   mz[1][2] = 0.0;
        mz[2][0] = 0.0 ;                                mz[2][1] = 0.0 ;                                mz[2][2] = 1.0;

        if( cl )
                matrix_matrix_mult( mz, mx,     dummy);
        else
                matrix_matrix_mult( mx, mz,     dummy);
        matrix_matrix_mult( dummy, my, m);
}


// Do 3D-coordinate Transformation

void doCoordinateTransform( CoordInfo *ci, tMatrix *t )
{
        double m[3][3],a,b,c;
        int i;
        double mx[3][3], my[3][3], mz[3][3], dummy[3][3];
        

        // Calculate Matrices;
        a = DEG_TO_RAD( t->alpha );
        b = DEG_TO_RAD( t->beta  );
        c = DEG_TO_RAD( t->gamma );

        mx[0][0] = 1.0 ;                                mx[0][1] = 0.0 ;                                mx[0][2] = 0.0;
        mx[1][0] = 0.0 ;                                mx[1][1] = cos(a) ;                     mx[1][2] = sin(a);
        mx[2][0] = 0.0 ;                                mx[2][1] =-mx[1][2] ;                   mx[2][2] = mx[1][1];
        
        my[0][0] = cos(b);                              my[0][1] = 0.0 ;                                my[0][2] =-sin(b);
        my[1][0] = 0.0 ;                                my[1][1] = 1.0 ;                                my[1][2] = 0.0;
        my[2][0] = -my[0][2];                   my[2][1] = 0.0 ;                                my[2][2] = my[0][0];
        
        mz[0][0] = cos(c) ;                     mz[0][1] = sin(c) ;                     mz[0][2] = 0.0;
        mz[1][0] =-mz[0][1] ;                   mz[1][1] = mz[0][0] ;                   mz[1][2] = 0.0;
        mz[2][0] = 0.0 ;                                mz[2][1] = 0.0 ;                                mz[2][2] = 1.0;

        matrix_matrix_mult( my, mz,     dummy);
        matrix_matrix_mult( mx, dummy,  m);
        
        // Scale 
        
        for(i=0; i<3; i++)
                ci->x[i] *= t->scale;
        
        // Do shift
        
        for(i=0; i<3; i++)
                ci->x[i] += t->x_shift[i];
        
        // Do rotation
#if 0   
        SetMatrix( DEG_TO_RAD( t->alpha ) , 
                           DEG_TO_RAD( t->beta  ) ,
                           DEG_TO_RAD( t->gamma ) ,
                           m, 0 );

#endif
        matrix_inv_mult( m, ci->x );

}

void SettMatrixDefaults( tMatrix *t )
{
        int i;
        
        t->alpha = t->beta = t->gamma = 0.0;
        for(i=0; i<3; i++)
                t->x_shift[i] = 0.0;
        
        t->scale = 1.0;
}


        
        
        
        

//------------------------------- Transformation functions --------------------------------------------


#define         distanceparam   (*((double*)params))
#define         shift           (*((double*)params))
#define         var0            ((double*)params)[0]
#define         var1            ((double*)params)[1]
#define         var2            ((double*)params)[2]
#define         var3            ((double*)params)[3]
#define         mp              ((struct MakeParams*)params)

// execute a stack of functions stored in stack

void execute_stack              ( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
        register double                 xd = x_dest, 
                                                        yd = y_dest;
        register struct fDesc*  stack = (struct fDesc *) params;;
        
                
        while( (stack->func) != NULL )
        {

                (stack->func) ( xd, yd, x_src, y_src, stack->param );
                xd = *x_src;    
                yd = *y_src;
                stack++;
        }
}

        
int execute_stack_new( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
    register double xd = x_dest,
                    yd = y_dest;
    register struct fDesc*  stack = (struct fDesc *) params;

    while( (stack->func) != NULL )
    {
        if ( (stack->func) ( xd, yd, x_src, y_src, stack->param ) ) {
            //      printf("Execute stack %f %f %f %f\n", xd, yd, *x_src, *y_src);
            xd = *x_src;
            yd = *y_src;
            stack++;
        } else {
            return 0;
        }
    }
    return 1;
}


// Rotate equirectangular image

int rotate_erect( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
        // params: double 180degree_turn(screenpoints), double turn(screenpoints);

                *x_src = x_dest + var1;

                while( *x_src < - var0 )
                        *x_src += 2 *  var0;

                while( *x_src >  var0 )
                        *x_src -= 2 *  var0;

                *y_src = y_dest ;
    return 1;
}



// Calculate inverse 4th order polynomial correction using Newton
// Don't use on large image (slow)!


int inv_radial( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
        // params: double coefficients[4]

        register double rs, rd, f, scale;
        int iter = 0;

        rd              = (sqrt( x_dest*x_dest + y_dest*y_dest )) / ((double*)params)[4]; // Normalized 

        rs      = rd;                           
        f       = (((((double*)params)[3] * rs + ((double*)params)[2]) * rs + 
                                ((double*)params)[1]) * rs + ((double*)params)[0]) * rs;

        while( abs(f - rd) > R_EPS && iter++ < MAXITER )
        {
                rs = rs - (f - rd) / ((( 4 * ((double*)params)[3] * rs + 3 * ((double*)params)[2]) * rs  + 
                                                  2 * ((double*)params)[1]) * rs + ((double*)params)[0]);

                f       = (((((double*)params)[3] * rs + ((double*)params)[2]) * rs + 
                                ((double*)params)[1]) * rs + ((double*)params)[0]) * rs;
        }

        scale = (rd!=0.0) ? rs / rd : 1.0f;
//      printf("scale = %lg iter = %d\n", scale,iter);  
        
        *x_src = x_dest * scale  ;
        *y_src = y_dest * scale  ;
    return 1;
}

int inv_vertical( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
        // params: double coefficients[4]

        register double rs, rd, f, scale;
        int iter = 0;

        rd              = abs( y_dest ) / ((double*)params)[4]; // Normalized 

        rs      = rd;                           
        f       = (((((double*)params)[3] * rs + ((double*)params)[2]) * rs + 
                                ((double*)params)[1]) * rs + ((double*)params)[0]) * rs;

        while( abs(f - rd) > R_EPS && iter++ < MAXITER )
        {
                rs = rs - (f - rd) / ((( 4 * ((double*)params)[3] * rs + 3 * ((double*)params)[2]) * rs  + 
                                                  2 * ((double*)params)[1]) * rs + ((double*)params)[0]);

                f       = (((((double*)params)[3] * rs + ((double*)params)[2]) * rs + 
                                ((double*)params)[1]) * rs + ((double*)params)[0]) * rs;
        }

        scale = rs / rd;
//      printf("scale = %lg iter = %d\n", scale,iter);  
        
        *x_src = x_dest  ;
        *y_src = y_dest * scale  ;
    return 1;
}



int resize( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
        // params: double scale_horizontal, double scale_vertical;

                *x_src = x_dest * var0;
                *y_src = y_dest * var1;
    return 1;
}

int tiltInverse( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
        
  // 
  // This is really the inverse transformation
  
  //	printf( "Entered invtilt function \n");
  
  
    double theta = mp->tilt[0];            // use the tilt angle specified by the 'L' parameter
    double sigma = mp->tilt[1];            // use the tilt angle specified by the 'L' parameter
    double phi   = mp->tilt[2];
    double scale = mp->tilt[3];
    double v[3];                                            // 3D projective coordinate vector
    double m_tilt[3][3];                            // tilt matrix
    double m_rotate[3][3];                            // tilt matrix
    double xmax = mp->im->width/2;          // maximum y value is image width divided by 2
    double z0;
    double FOV = DEG_TO_RAD(mp->im->hfov/scale);                  
    double m_slant[3][3];                           // slant matrix
    
    //        printf("Tilt Inverse %5.2f %5.2f %5.2f, %5.2f\n", x_dest, y_dest, theta, sigma);
    
    // These operations are based on the typical projection of a point to a camera.
    // But, we want the projection to happen without translation, so we need to subtract
    // the coordinates of 0,0 projected... that is why the [1][2], [2][2] components
    // of the matrix are 0 or 1 in the tilt-x and tilt-y
    
    
    // Tilt (around X) INVERSE. The matrix does not require the [1][2], [2][2]
    // because it is tilted in its center
    m_tilt[0][0] = 1;       m_tilt[0][1] = 0;                               m_tilt[0][2] = 0;
    m_tilt[1][0] = 0;       m_tilt[1][1] = cos(theta);                      m_tilt[1][2] = 0; // sin(theta); 
    m_tilt[2][0] = 0;       m_tilt[2][1] = -sin(theta);                     m_tilt[2][2] = 1; //cos(theta);
    
    // Tilt (around Y) INVERSE.       
    // [0][2] and [2][2] are changed to tilt on center of image. See above
    m_slant[0][0] = cos(sigma);	m_slant[0][1] = 0;		m_slant[0][2] = 0; //-sin(sigma);
    m_slant[1][0] = 0;		m_slant[1][1] = 1;		m_slant[1][2] = 0;
    m_slant[2][0] = sin(sigma);	m_slant[2][1] = 0;		m_slant[2][2] = 1; //cos(sigma);
    
    // Slant (around z)
    
    // Tilt (around z) INVERSE 
    m_rotate[0][0] = cos(phi);	m_rotate[0][1] = sin(phi);		m_rotate[0][2] = 0;
    m_rotate[1][0] = -sin(phi);	m_rotate[1][1] = cos(phi);		m_rotate[1][2] = 0;
    m_rotate[2][0] = 0;	        m_rotate[2][1] = 0;		        m_rotate[2][2]=  1;
    
    z0 = xmax/tan(FOV/2);           // FOV is full angle FOV in radians
    //    printf("Values Forward %5.2f %5.2f %5.2f, %5.2f\n", x_dest, y_dest, distanceparam, z0);
    
    // z0 is distance to image from center of projection                                            
    
    //printf("z0 is %f \n", z0);
    // Now you have [x_dest, y_dest, z1]'.  Multiply that vector by M, the adjusted tilt matrix
    v[0] = x_dest;
    v[1] = y_dest;
    v[2] = z0;
    
    matrix_mult(m_rotate,v);
    matrix_mult(m_slant,v);
    matrix_mult(m_tilt,v);

    
    // Now project into xy plane
    *x_src =  v[0]* z0 /v[2];
    *y_src =  v[1]* z0 /v[2];

// THIS IS THE OPTIMIZED VERSION with no matrices multiplications. ..
#ifdef OPTIMIZED
    double x = x_dest;
    double y = y_dest;
    double above = (x * cos(phi) + y * sin(phi)) * cos (sigma);
    double below1  = (x * cos(phi) + y * sin(phi)) * sin(sigma);
    double below2 = - sin(theta) * (y * cos(phi) - x * sin(phi));
    double x4 = z0*  above / (below1 + below2 + z0);

    above  = (-x * sin (phi) + y * cos(phi)) * cos(theta);
    below1 = (x * cos(phi) + y * sin(phi)) * sin (sigma);
    below2 = (-x * sin(phi) + y * cos(phi)) * sin(theta);
    
    double y4 = z0 * above / (below1 - below2 + z0);
#endif

//    printf("x4 %6.2f x_c %6.2f\n", x4, *x_src);
//    printf("x4 %6.2f x_c %6.2f\n", y4, *y_src);

    
    return 1;
}

// Dev: inverse tilt function.  Substitute correct matrix for "un-tilting"
int tiltForward( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
  // INVERSE is a misnomer --panotools ##$&^
  // This is really the forward transformation

  //	printf( "Entered invtilt function \n");

	double phi = mp->tilt[0];		// use the tilt angle specified by the 'L' parameter (already in radians)
	double phi2 = mp->tilt[1];		// use the tilt angle specified by the 'L' parameter (already in radians)
        double phi3  = mp->tilt[2];
        double scale  = mp->tilt[3];

	double v[3];						// 3D projective coordinate vector
	double m_tilt[3][3];				// tilt matrix
	double xmax = mp->im->width/2;		// maximum y value is image width divided by 2
	double z0, z1, s;
	double FOV = DEG_TO_RAD(mp->im->hfov/scale);
        double m_slant[3][3];                           // slant matrix
        double m_rotate[3][3];                          // tilt matrix

//        printf("Tilt Forward %5.2f %5.2f angles %5.2f, %5.2f, %5.2f\n", x_dest, y_dest, phi, phi2, phi3);

	// TILT TO STRAIGHT matrix (tilt2() in MATLAB).  Really and UN-TILTING matrix
        // FORWARD
	m_tilt[0][0] = 1;	m_tilt[0][1] = 0;		m_tilt[0][2] = 0;
	m_tilt[1][0] = 0;	m_tilt[1][1] = 1/cos(phi);	m_tilt[1][2] = 0;
	m_tilt[2][0] = 0;	m_tilt[2][1] = tan(phi);	m_tilt[2][2] = 1;
								

	m_slant[0][0] = 1/cos(phi2);	m_slant[0][1] = 0;		m_slant[0][2] = 0;
	m_slant[1][0] = 0;		m_slant[1][1] = 1;		m_slant[1][2] = 0;
	m_slant[2][0] = -sin(phi2)/cos(phi2);	m_slant[2][1] = 0;		m_slant[2][2] = 1;

        // FORWARD
        m_rotate[0][0] = cos(phi3);	m_rotate[0][1] = -sin(phi3);		m_rotate[0][2] = 0;
        m_rotate[1][0] = sin(phi3);	m_rotate[1][1] = cos(phi3);		m_rotate[1][2] = 0;
        m_rotate[2][0] = 0;	        m_rotate[2][1] = 0;		        m_rotate[2][2]=  1;



	z0 = xmax/tan(FOV/2); 		// FOV is full angle FOV in radians
								// z0 is distance to image from center of projection						
        // First step, undo the projection of the point
        z1 = (z0 * z0) /(x_dest * (-sin(phi2)/cos(phi2))  + y_dest * tan(phi) + z0);
        
        s = z1 / z0;
        s = z0 /(x_dest * (-sin(phi2)/cos(phi2))  + y_dest * sin(phi)/cos(phi) + z0);
        z1 = (z0* z0) /(x_dest * (-sin(phi2)/cos(phi2))  + y_dest * sin(phi)/cos(phi) + z0);

        //(y_dest * tan(phi) + x_dest * tan(phi2) + z0);

	//s = z0 / (y_dest * tan(phi) + z0);		// s is a scaling factor ...
        
	//printf("z0 is %f \n", z0);
	// Now you have [x_dest, y_dest, z1]'.  Multiply that vector by M, the adjusted tilt matrix
	v[0] =  s* x_dest;
	v[1] =  s* y_dest;
	v[2] =  z1;

        //	matrix_mult(m_tilt,v);
        matrix_mult(m_tilt,v);
        matrix_mult(m_slant,v);
	matrix_mult(m_rotate,v);

	*x_src =   v[0] ;			// convert back to cartesian coordinates
	*y_src =   v[1];

//        printf( "Entered tiltforward function tilt z0,z1,v[21] (%f,%f, %f)\n", z0, z1, v[2]);

        //	printf( "Entered invtiltb function in (%6.2f %6.2f) out (%6.2f %6.2f)\n", x_dest, y_dest, *x_src, *y_src);

	// Uncommenting code below causes tilt function to do nothing:
	//		*x_src	= x_dest;	
	//		*y_src  = y_dest;
    return 1;
}

int shear( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double shear_horizontal, double shear_vertical;
  //	printf( "Entered shear function \n");


		*x_src  = x_dest + var0 * y_dest;
		*y_src  = y_dest + var1 * x_dest;
    return 1;
}

int shearInv( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double shear_horizontal, double shear_vertical;
  //	printf( "Entered shear inv function \n");


    *y_src  = (y_dest - var1 * x_dest) / (1 - var1 * var0);
    *x_src  = (x_dest - var0 * *y_src);
    return 1;
}

int horiz( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double horizontal shift

		*x_src	= x_dest + shift;	
		*y_src  = y_dest;
    return 1;
}

int vert( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double vertical shift

		*x_src	= x_dest;	
		*y_src  = y_dest + shift;		
    return 1;
}

	
int radial( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double coefficients[4], scale, correction_radius

	register double r, scale;

	r 		= (sqrt( x_dest*x_dest + y_dest*y_dest )) / ((double*)params)[4];
	if( r < ((double*)params)[5] )
	{
		scale 	= ((((double*)params)[3] * r + ((double*)params)[2]) * r + 
				((double*)params)[1]) * r + ((double*)params)[0];
	}
	else
		scale = 1000.0;
	
	*x_src = x_dest * scale  ;
	*y_src = y_dest * scale  ;
    return 1;
}

int vertical( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double coefficients[4]

	register double r, scale;

	r 		= y_dest / ((double*)params)[4];

	if( r < 0.0 ) r = -r;

	scale 	= ((((double*)params)[3] * r + ((double*)params)[2]) * r + 
				((double*)params)[1]) * r + ((double*)params)[0];
	
	*x_src = x_dest;
	*y_src = y_dest * scale  ;
    return 1;
}

int deregister( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double coefficients[4]

	register double r, scale;

	r 		= y_dest / ((double*)params)[4];

	if( r < 0.0 ) r = -r;

	scale 	= (((double*)params)[3] * r + ((double*)params)[2]) * r + 
				((double*)params)[1] ;
	
	*x_src = x_dest + abs( y_dest ) * scale;
	*y_src = y_dest ;
    return 1;
}



	
int persp_sphere( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params :  double Matrix[3][3], double distanceparam

	register double theta,s,r;
	double v[3];

#if 0	// old 
	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / *((double*) ((void**)params)[1]);
	phi   = atan2( y_dest , x_dest );

	v[0] = *((double*) ((void**)params)[1]) * sin( theta ) * cos( phi );
	v[1] = *((double*) ((void**)params)[1]) * sin( theta ) * sin( phi );
	v[2] = *((double*) ((void**)params)[1]) * cos( theta );
	
	matrix_inv_mult( (double(*)[3]) ((void**)params)[0], v );

	theta = atan2( sqrt( v[0]*v[0] + v[1]*v[1] ), v[2] );
	phi   = atan2( v[1], v[0] );
	*x_src = *((double*) ((void**)params)[1]) * theta * cos( phi );
	*y_src = *((double*) ((void**)params)[1]) * theta * sin( phi );
#endif

	r 		= sqrt( x_dest * x_dest + y_dest * y_dest );
	theta 	= r / *((double*) ((void**)params)[1]);
	if( r == 0.0 )
		s = 0.0;
	else
		s = sin( theta ) / r;

	v[0] =  s * x_dest ;
	v[1] =  s * y_dest ;
	v[2] =  cos( theta );

	matrix_inv_mult( (double(*)[3]) ((void**)params)[0], v );

	r 		= sqrt( v[0]*v[0] + v[1]*v[1] );
	if( r == 0.0 )
		theta = 0.0;
	else
		theta 	= *((double*) ((void**)params)[1]) * atan2( r, v[2] ) / r;
	*x_src 	= theta * v[0];
	*y_src 	= theta * v[1];

    return 1;
}	

int persp_rect( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params :  double Matrix[3][3], double distanceparam, double x-offset, double y-offset

	double v[3];
	
	v[0] = x_dest + *((double*) ((void**)params)[2]);
	v[1] = y_dest + *((double*) ((void**)params)[3]);
	v[2] = *((double*) ((void**)params)[1]);
	
	matrix_inv_mult( (double(*)[3]) ((void**)params)[0], v );
	
	*x_src = v[0] * *((double*) ((void**)params)[1]) / v[2] ;
	*y_src = v[1] * *((double*) ((void**)params)[1]) / v[2] ;
    return 1;
}



int rect_pano( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{									
								
	*x_src = distanceparam * tan( x_dest / distanceparam ) ;
	*y_src = y_dest / cos( x_dest / distanceparam );
    return 1;
}

int pano_rect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{	
	*x_src = distanceparam * atan ( x_dest / distanceparam );
	*y_src = y_dest * cos( *x_src / distanceparam );
    return 1;
}

int rect_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{	
	// params: double distanceparam

	register double  phi, theta;

	phi 	= x_dest / distanceparam;
	theta 	=  - y_dest / distanceparam  + PI / 2.0;
	if(theta < 0)
	{
		theta = - theta;
		phi += PI;
	}
	if(theta > PI)
	{
		theta = PI - (theta - PI);
		phi += PI;
	}

#if 0
	v[2] = *((double*)params) * sin( theta ) * cos( phi );   //  x' -> z
	v[0] = *((double*)params) * sin( theta ) * sin( phi );	//  y' -> x
	v[1] = *((double*)params) * cos( theta );				//  z' -> y
	
	phi   = atan2( v[1], v[0] );
//  old:	
//	theta = atan2( sqrt( v[0]*v[0] + v[1]*v[1] ) , v[2] );
//	rho = *((double*)params) * tan( theta );
//  new:
	rho = *((double*)params) * sqrt( v[0]*v[0] + v[1]*v[1] ) / v[2];
	*x_src = rho * cos( phi );
	*y_src = rho * sin( phi );
#endif
#if 1
	*x_src = distanceparam * tan(phi);
	*y_src = distanceparam / (tan( theta ) * cos(phi));
#endif
	// normalize phi to be in the -PI, PI range
	while(phi <= -PI)
		phi += 2*PI;
	while(phi > PI)
		phi -= 2*PI;

	// check if the point is "in front" of the camera
	if (phi < -PI/2.0 || phi > PI/2.0) {
		// behind, transform considered invalid
		return 0;
	} else
		return 1;
	// normalize phi to be in the -PI, PI range
	while(phi <= -PI)
		phi += 2*PI;
	while(phi > PI)
		phi -= 2*PI;

	// check if the point is "in front" of the camera
	if (phi < -PI/2.0 || phi > PI/2.0) {
		// behind, transform considered invalid
		return 0;
	} else
		return 1;

}
// This is the cylindrical projection
int pano_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{	
	// params: double distanceparam

	*x_src = x_dest;
	*y_src = distanceparam * tan( y_dest / distanceparam);
    return 1;
}

int erect_pano( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{	
	// params: double distanceparam

	*x_src = x_dest;
	*y_src = distanceparam * atan( y_dest / distanceparam);
    return 1;
}

/** convert from erect to lambert azimuthal */
int lambertazimuthal_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    double phi, lambda,k1;
    lambda = x_dest/distanceparam;
    phi = y_dest/distanceparam;

    if (abs(cos(phi) * cos(lambda) + 1.0) <= EPSLN) {
      *x_src = distanceparam * 2 ;
      *y_src = 0;
      return 0;
    }

    k1 = sqrt(2.0 / (1 + cos(phi) * cos(lambda)));

    *x_src = distanceparam * k1 * cos(phi) * sin (lambda);
    *y_src = distanceparam * k1 * sin(phi);

    return 1;
}

/** convert from lambert azimuthal to erect */
int erect_lambertazimuthal( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{

    double x, y, ro,c;

    x = x_dest/distanceparam;
    y = y_dest/distanceparam;

    assert(! isnan(x));
    assert(! isnan(y));

    if (fabs(x) > PI || fabs(y) > PI) {
        *y_src = 0;
        *x_src = 0;
	return 0;
    }

    ro = hypot(x, y);

    if (fabs(ro) <= EPSLN)
    {
        *y_src = 0;
        *x_src = 0;
        return 1;
    }

    c = 2 * asin(ro / 2.0);

    *y_src = distanceparam * asin( (y * sin(c)) / ro);


    if (fabs(ro * cos(c)) <= EPSLN ) {
      *x_src = 0;
      return 1;
    }

    *x_src = distanceparam * atan2( x * sin(c), (ro * cos(c)));

    return 1;
}

/** convert from erect to hammer */
int hammer_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    if(lambertazimuthal_erect(x_dest/2.0, y_dest, x_src, y_src, params))
    {
        *x_src *= 2.0;
        return 1;
    }
    else
    {
        *x_src=0;
        *y_src=0;
        return 0;
    };
}

/** convert from hammer to erect */
int erect_hammer( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    double x, y, z;
    x = x_dest/distanceparam;
    y = y_dest/distanceparam;
    z = 1.0 - (x * x / 16.0) - (y * y / 4.0);
    if(z<0)
    {
        *x_src=0;
        *y_src=0;
        return 0;
    };
    z = sqrt(z);
    *x_src = 2.0 * atan2(z*x, 2.0*(2.0*z*z-1.0));
    *y_src = asin(y*z);
    if(fabs(*x_src) > PI || fabs(*y_src) > HALF_PI)
    {
        *x_src=0;
        *y_src=0;
        return 0;
    };
    *x_src *= distanceparam;
    *y_src *= distanceparam;
    return 1;
}

/** convert from erect to mercator FORWARD */
int mercator_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    *x_src = x_dest;
    *y_src = distanceparam*log(tan(y_dest/distanceparam)+1/cos(y_dest/distanceparam));
    return 1;
}

/** convert from mercator to erect INVERSE */
int erect_mercator( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    *x_src = x_dest;
    *y_src = distanceparam*atan(sinh(y_dest/distanceparam));
    return 1;
}


/** convert from erect to miller */
int millercylindrical_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    double phi, tanPhi;

    *x_src = x_dest;
    phi = y_dest/distanceparam;
    tanPhi = tan(PI/4 +0.4 * phi);
    if (tanPhi < 0) {
        *x_src = 0;
        *y_src = 0;
        return 0;
    };    
    *y_src = distanceparam*log(tanPhi)/0.8;
    return 1;
}

/** convert from erect to miller */
int arch_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    if (y_dest < 0) {
        return millercylindrical_erect(x_dest, y_dest, x_src, y_src, params);
    } else {
        return lambert_erect(x_dest, y_dest, x_src, y_src, params);
    }
}

/** convert from erect to miller */
int erect_arch( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    if (y_dest < 0) {
        return erect_millercylindrical(x_dest, y_dest, x_src, y_src, params);
    } else {
        return erect_lambert(x_dest, y_dest, x_src, y_src, params);
    }
}



/** convert from miller cylindrical to erect */
int erect_millercylindrical( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    double y;

    *x_src = x_dest;
    y = y_dest/distanceparam;
    y = 1.25 * atan(sinh(4 * y /5.0));
    if ( fabs(y) > HALF_PI) {
        *x_src = 0;
        *y_src = 0;
        return 0;
    };
    *y_src = distanceparam * y;
    return 1;
}


/** convert from erect to panini */
int panini_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
  // this is the inverse

    double phi, lambdaHalf, temp,y,x;

    phi = y_dest/distanceparam;
    lambdaHalf = x_dest/ (distanceparam*2);
    x = 2 * tan (lambdaHalf);

    // Conver from central cylindrical
    phi = tan(phi);

    *x_src = distanceparam * x;
    temp  = cos(lambdaHalf);
    
    y = tan(phi) / (temp * temp);
    
    // At this point we have mapped to central cylindrical, now to equirectangular
      
    *y_src = distanceparam *  y;

    return 1;
}

/** General Pannini Projection

  setup_panini_general(&MakeParams) selects the Image struct
  corresponding to the pannini_general image and returns its
  address, or a NULL pointer for failure.

  If the selected Image has an invalid precomputedCount, it
  posts the distanceparam corresponding to min( max feasible
  hFOV, requested hFOV) and puts working parameter values in 
  precomputeValue[] in the selected Image.

  SetMakeParams (adjust.c) calls this function in lieu of setting
  distanceparam.

  The user-visible projection params, described in queryfeature.c, 
  are scaled to accomodate integer-valued control sliders in a GUI.  
  unscaleParams_panini_general() sets working values as follows:
    cmpr 0:100:150 <-> d = 0:1:->infinity NOTE very nonlinear
    tops, bots -100:100 <-> sqz -1:1 linear
				< 0 gives soft squeeze
				> 0 give transverse straightening squeeze
  CAUTION these ranges are assumed, not read from queryfeature.c

  maxFOVs_panini_general() calculates the maximum feasible FOVs
  for a given scaled parameter set.   Those also depends on a 
  projection angle limit, that is hard coded here.  FOVs in degrees.

**/
#define MAX_PROJ_ANGLE 80	

int unscaleParams_panini_general( 
				double * gui_params,	// cmpr, tops, bots
				double * wrk_params		// d, t, b
							    )
{
	double t;

   /* check for legal values */
    if(    gui_params[0] < 0
		|| gui_params[0] > 150
	  ) return 0;
    if(    gui_params[1] < -100
		|| gui_params[1] > 100
	  ) return 0;
    if(    gui_params[2] < -100
		|| gui_params[2] > 100
	  ) return 0;

    /* post working param values */
    t = (150 - gui_params[0]) / 50;	/* 0:150 => 3:0 */
    wrk_params[0] = 1.5 / (t + 0.0001) - 1.5/3.0001;
    wrk_params[1] = gui_params[1] / 100;
    wrk_params[2] = gui_params[2] / 100;

	return 1;
}

int maxFOVs_panini_general	( double *params, double *fovs ){
	double wparams[3], mfovs[2];
  /* translate user params to working values */
	if( !unscaleParams_panini_general( params, wparams )
	  )
	  return 0;
  /* compute max half-fovs in radians */
	if( !panini_general_maxVAs( wparams[0], 
							  DEG_TO_RAD(MAX_PROJ_ANGLE),
							  mfovs
							 )
	  )
	  return 0;

 /* return full fovs in degrees */
	fovs[0] = 2 * RAD_TO_DEG(mfovs[0]);
	fovs[1] = 2 * RAD_TO_DEG(mfovs[1]);

	return 1;
}

Image * setup_panini_general(struct MakeParams* pmp)
{	int  i; 
    double s,t,d,v, vl[2];
    Image * ppg = NULL;

    // Only act if it is panini_general 
    if( pmp->im->format == _panini_general )  // input panini --is it supported?
        ppg = pmp->im;
    else if( pmp->pn->format == _panini_general ) // output panini
        ppg = pmp->pn;
    else 
        return NULL;

    /* check number of precomputed param values */
    if( ppg->precomputedCount == 7 )
		return ppg;		// OK
	
    /* default unspecified values to 0, giving 
		stereographic compresssion (d = 1)
		and no squeezes.
	*/
    for( i = ppg->formatParamCount; i < 3; i++ ) 
        ppg->formatParam[i] = 0;

  /* translate user params to working values */
	if( !unscaleParams_panini_general( ppg->formatParam, ppg->precomputedValue )
	  )
	  return NULL;
	d = ppg->precomputedValue[0];

  /* get max feasible half-FOVs */
	if( !panini_general_maxVAs( d,
								DEG_TO_RAD( 80 ),	// max projection angle
								vl					// max view angles h, v
							  )
	  )
	  return 0;

  // angle and coordinate limits
	s = (d + 1) / (d + cos(vl[0]));
	ppg->precomputedValue[3] = vl[0];	// max lambda
	ppg->precomputedValue[4] = s * sin( vl[0] );	// max x 
	ppg->precomputedValue[5] = vl[1];	// max phi
	ppg->precomputedValue[6] = s * tan( vl[1] );	// max y 

  // clip hFOV to feasible limit
	v = 0.5 * DEG_TO_RAD( ppg->hfov );
	if( v > vl[0] )
		v = vl[0];

  // set distance param
	t = sin(v) * (d+1) / (d + cos(v));
	pmp->distance = 0.5 * ppg->width / t;
       
	ppg->precomputedCount = 7; 
    return ppg;
}

/** convert from panini_general to erect **/
int erect_panini_general( double x_dest,double  y_dest, double* lambda_src, double* phi_src, void* params)
{  /* params -> MakeParams */
    double x, y, lambda, phi, d, distance;
    
    Image * ppg = setup_panini_general(mp);
    if( !ppg ) 
        return FALSE;

    d = ppg->precomputedValue[0];

    distance = mp->distance;
    y = y_dest/distance;
    x = x_dest/distance;

  // fail if outside max image
/*	if(  fabs(x) > ppg->precomputedValue[4] 
	  || fabs(y) > ppg->precomputedValue[6]
	  )
	  return 0;	
*/
  // call mapping fn
	if( !panini_general_toSphere( &lambda, &phi, x, y,
								 ppg->precomputedValue[0],
								 ppg->precomputedValue[1],
								 ppg->precomputedValue[2])
	  )
	  return 0;
	    
 	*lambda_src = lambda * distance;
	*phi_src = phi * distance;
    
	return TRUE;
}


	/** convert from erect to panini_general **/
int panini_general_erect( double lambda_dest,double  phi_dest, double* x_src, double* y_src, void* params)
{
	/* params -> MakeParams */

	double phi, lambda, y,x;
	double d;  // >= 0
	double distance;

	Image * ppg = setup_panini_general(mp);
	if( !ppg ) 
		return 0;

	d = ppg->precomputedValue[0];
    
	distance = mp->distance;
	lambda = lambda_dest/distance;
	phi = phi_dest/distance;

  // fail if outside feasible FOV
/*	if(  fabs(lambda) > ppg->precomputedValue[3] 
	  || fabs(phi) > ppg->precomputedValue[5]
	  )
	  return 0;	
*/
  // call mapping fn
	if( !panini_general_toPlane( lambda, phi, &x, &y,
								 ppg->precomputedValue[0],
								 ppg->precomputedValue[1],
								 ppg->precomputedValue[2])
	  )
	  return 0;

  // fail if coords outside max image (needed for squeeze)
/*	if(  fabs(x) > ppg->precomputedValue[4] 
	  || fabs(y) > ppg->precomputedValue[6]
	  )
	  return 0;	
*/
	*y_src = distance * y;
    *x_src = distance * x;
    return 1;
}

/** convert from panini to erect */
int erect_panini( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    double y;
    double x;
    double temp;

    double  lambda;

    double phi;
    y = y_dest/distanceparam;
    x = x_dest/distanceparam;
    lambda = atan(x/2);
 
    // then project from opposite edge of the cylinder
    temp = cos(lambda);
    phi = y * temp * temp;

    // First convert to central cylindrical
    phi  = atan( phi);

    *x_src = 2 * lambda * distanceparam;
    *y_src = atan(phi) * distanceparam;

    return 1;
}




/** convert from erect to equi panini */
int equipanini_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
  // this is the inverse

    double phi, lambdaHalf, temp,y,x;

    phi = y_dest/distanceparam;
    lambdaHalf = x_dest/ (distanceparam*2);
    x = 2 * tan (lambdaHalf);

    *x_src = distanceparam * x;
    temp  = cos(lambdaHalf);
    
    y = tan(phi) / (temp * temp);

    // At this point we have mapped to central cylindrical, now to equirectangular
      
    *y_src = distanceparam *  y;

    return 1;
}


/** convert from equi panini to erect */
int erect_equipanini( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    double y;
    double x;
    double temp;

    double  lambda;

    double phi;
    y = y_dest/distanceparam;
    x = x_dest/distanceparam;
    lambda = atan(x/2);
 
    // then project from opposite edge of the cylinder
    temp = cos(lambda);
    phi = y * temp * temp;

    *x_src = 2 * lambda * distanceparam;
    *y_src = atan(phi) * distanceparam;

    return 1;
}





/** convert from erect to lambert */
int lambert_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    *x_src = x_dest;
    *y_src = distanceparam*sin(y_dest/distanceparam);
    return 1;
}

/** convert from lambert to erect */
int erect_lambert( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    double y;
    *x_src = x_dest;
    y = y_dest / distanceparam;
    if (fabs(y) > 1) {
        *x_src = 0;
        *y_src = 0;
        return 0;
    };
    *y_src = distanceparam*asin(y);
    return 1;
}


/** convert from erect to transverse mercator */
int transmercator_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    // params: distanceparam
    double B;
    x_dest /= distanceparam;
    y_dest /= distanceparam;
    B = cos(y_dest)*sin(x_dest);
    *x_src = distanceparam * atanh(B);
    *y_src = distanceparam * atan2(tan(y_dest), cos(x_dest));

    if (isinf(*x_src)) {
      return 0;
    }

    return 1;
}

/** convert from erect to transverse mercator */
int erect_transmercator( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    x_dest /= distanceparam;
    y_dest /= distanceparam;

    if (fabs(y_dest) > PI ) {
        *y_src = 0;
        *x_src = 0;
	return 0;
    }


    *x_src = distanceparam * atan2(sinh(x_dest),cos(y_dest));
    *y_src = distanceparam * asin(sin(y_dest)/cosh(x_dest));
    return 1;
}

/** convert from erect to sinusoidal */
int sinusoidal_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    // params: distanceparam

    *x_src = distanceparam * (x_dest/distanceparam*cos(y_dest/distanceparam));
    *y_src = y_dest;
    return 1;
}

/** convert from sinusoidal to erect */
int erect_sinusoidal( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    // params: distanceparam

    *y_src = y_dest;
    *x_src = x_dest/cos(y_dest/distanceparam);
    if (*x_src/distanceparam < -PI || *x_src/distanceparam > PI)
	return 0; 
    return 1;
}

/** convert from erect to stereographic */
int stereographic_erect_old( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    // params: distanceparam
    double lon = x_dest / distanceparam;
    double lat = y_dest / distanceparam;

    // use: R = 1
    double k=2.0/(1+cos(lat)*cos(lon));
    *x_src = distanceparam * k*cos(lat)*sin(lon);
    *y_src = distanceparam * k*sin(lat);
    return 1;
}

int stereographic_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    double lon, lat;
    double sinphi, cosphi, coslon;
    double g,ksp;

    lon = x_dest / distanceparam;
    lat = y_dest / distanceparam;

    sinphi = sin(lat);
    cosphi = cos(lat);
    coslon = cos(lon);

    g = cosphi * coslon;

    // point projects to infinity:
    //    if (fabs(g + 1.0) <= EPSLN)

    ksp = distanceparam * 2.0 / (1.0 + g);
    *x_src = ksp * cosphi * sin(lon);
    *y_src = ksp * sinphi;

    return 1;
}

/** convert from stereographic to erect */
int erect_stereographic( double x_dest,double  y_dest, double* lon, double* lat, void*  params)
{
    double rh;		/* height above sphere*/
    double c;		/* angle					*/
    double sinc,cosc;	/* sin of c and cos of c			*/
    double con;

    /* Inverse equations
     -----------------*/
    double x = x_dest / distanceparam;
    double y = y_dest / distanceparam;
    rh = sqrt(x * x + y * y);
    c = 2.0 * atan(rh / (2.0 * 1));
    sinc = sin(c);
    cosc = cos(c);
    *lon = 0;
    if (fabs(rh) <= EPSLN)
    {
        *lat = 0;
        return 0;
    }
    else
    {
        *lat = asin((y * sinc) / rh) * distanceparam;
        con = HALF_PI;
   
        con = cosc;
        if ((fabs(cosc) < EPSLN) && (fabs(x) < EPSLN))
            return 0;
        else
            *lon = atan2((x * sinc), (cosc * rh)) * distanceparam;
    }
    return 1;
}


/** convert from stereographic to erect */
int erect_stereographic_old( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    // params: distanceparam

    // use: R = 1
    double p=sqrt(x_dest*x_dest + y_dest*y_dest) / distanceparam;
    double c= 2.0*atan(p/2.0);

    *x_src = distanceparam * atan2(x_dest/distanceparam*sin(c),(p*cos(c)));
    *y_src = distanceparam * asin(y_dest/distanceparam*sin(c)/p);
    return 1;
}

int albersEqualAreaConic_ParamCheck(Image *im) 
{
    //Parameters: phi1, phi2, phi0, n, C, rho0, yoffset
    // Albers Equal-Area Conic Projection
    // parameters phi0, phi1, phi2:
    //   latitude values
    // parameter n:
    //   (latex) $\frac{1}{2}\left(\sin\phi_1 + \sin\phi_2\right)$
    //   (C) (sin(phi1) + sin(phi2)) / 2.0
    // parameter C:
    //   (latex) $\cos^2\phi_1 + 2 n \sin\phi_1$
    //   (C) cos(phi1) * cos(phi1) + 2.0 * n * sin(phi1)
    // parameter rho0:
    //   (latex) $\frac{\sqrt{C - 2 n \sin\phi_0}}{n}$
    //   (C) sqrt(C - 2.0 * n * sin(phi0))
    // yoffset:
    //   offset in y-direction for centering

    double phi0, phi1, phi2, n, C, rho0, yoffset;
    double Aux_2N; /* auxiliary variable for (2 * n) */
    double Aux_sin_phi0; /* auxiliary variable for sin(phi0) */
    double Aux_sin_phi1; /* auxiliary variable for sin(phi1) */
    double Aux_sin_phi2; /* auxiliary variable for sin(phi2) */
    double Aux_yl, Aux_yo; /* auxiliary variables for lower and upper y value */
    double Aux_1; /* auxiliary variable */
    double phi[] = {-PI/2, 0, PI/2};
    double lambda[] = {-PI, 0, PI};
    int i, j, first;

    assert(PANO_PROJECTION_MAX_PARMS >= 2);

    if (im->formatParamCount == 1) {
        // When only one parameter provided, assume phi1 = phi0
	im->formatParamCount = 2;
	im->formatParam[1] = im->formatParam[0];
    }

    if (im->formatParamCount == 0) {
        // if no latitude values given, then set defaults
	im->formatParamCount = 2;
	im->formatParam[0] = 0;  //phi1
	im->formatParam[1] = -60; //phi2
    }

    if (im->precomputedCount == 0) {
	im->precomputedCount = 10;

	assert(PANO_PROJECTION_PRECOMPUTED_VALUES >=im->precomputedCount );


	// First, invert standard parallels. 
	// This is a hack, as the resulting projections look backwards to what they are supposed to be
	// (with respect to maps)
	
	im->precomputedValue[0] =  -1.0 * im->formatParam[0];
	im->precomputedValue[1] =  -1.0 * im->formatParam[1];

        phi0 = 0; // default value of phi0
	phi1 = im->precomputedValue[0] * PI / 180.0; //phi1 to rad
	phi2 = im->precomputedValue[1] * PI / 180.0; //phi2 to rad

        // precompute sinus functions
        Aux_sin_phi0 = sin(phi0);
        Aux_sin_phi1 = sin(phi1);
        Aux_sin_phi2 = sin(phi2);
        // precompute auxiliary term (2 * n)
        Aux_2N = Aux_sin_phi1 + Aux_sin_phi2;

        // calculate parameters
        // The stability of these operations should be improved
        n = Aux_2N / 2.0;
        C = 1.0 + Aux_sin_phi1 * Aux_sin_phi2;
        Aux_1 = (C - (Aux_2N * Aux_sin_phi0));
        Aux_1 = ( (Aux_1 > 0.0) ? (sqrt(Aux_1)) : (0.0) );
        rho0 = ( (n != 0.0) ? (Aux_1 / n) : (1.7E+308) );

        // calculate the y at 6 different positions (lambda=-pi,0,+pi; phi=-pi/2,0,pi/2).
	///Then calculate a yoffset so that the image is centered.
	first = 1;
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 3; j++) {
                Aux_1 = C - (sin(phi[i]) * Aux_2N);
                if (C >= 0.0 && Aux_1 >= 0.0 && n != 0.0) {
                    Aux_1 = (sqrt(C) - (sqrt(Aux_1) * cos(n * lambda[j]))) / n;
                    if (first || Aux_1 < Aux_yl) {
                        Aux_yl = Aux_1;
                    }
                    if (first || Aux_1 > Aux_yo) {
                        Aux_yo = Aux_1;
                    }
		first = 0;
	    }
	}
        }
	if (first) {
            yoffset = 0.0;
        }
        else {
            yoffset = Aux_yl + fabs(Aux_yl - Aux_yo) / 2.0;
	}

	im->precomputedValue[0] = phi1;
	im->precomputedValue[1] = phi2;
	im->precomputedValue[2] = phi0;
	im->precomputedValue[3] = n;
	im->precomputedValue[4] = C;
	im->precomputedValue[5] = rho0;
        im->precomputedValue[6] = yoffset;
	im->precomputedValue[7] = n*n;
        im->precomputedValue[8] = Aux_2N;
        im->precomputedValue[9] = Aux_2N;

	//	printf("Parms phi1 %f phi2 %f pho0 %f, n %f, C %f, rho0 %f, %f\n", 
	//	       phi1, phi2, phi0, n, C, rho0, y);

    }

    for (i=0;i<im->precomputedCount;i++) {
	assert(!isnan(im->precomputedValue[i]));
    }
    
    if (im->precomputedCount > 0) {
        return 1;
    }
    //    PrintError("false in alberts equal area parameters");
    return 0;
}

/** convert from erect to albersequalareaconic */
int albersequalareaconic_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void *params)
{
    double yoffset, lambda, phi, lambda0, n, C, rho0, theta, rho;
    double twiceN;

    // Forward calculation


    if (!albersEqualAreaConic_ParamCheck(mp->pn))  {
	//	printf("REturning abert->erect 0\n");
	return 0;
    }

    assert(!isnan(x_dest));
    assert(!isnan(y_dest));

    lambda = x_dest / mp->distance;
    phi = y_dest / mp->distance;

    if (lambda > PI) lambda-=2*PI;
    if (lambda < -PI) lambda+=2*PI;

    lambda0 = 0;

    n = mp->pn->precomputedValue[3];
    C = mp->pn->precomputedValue[4];
    rho0 = mp->pn->precomputedValue[5];
    yoffset = mp->pn->precomputedValue[6];
    twiceN = mp->pn->precomputedValue[9];

    theta = n * (lambda - lambda0);

    
    //    printf("value %f\n", (phi));
    //    printf("value %f\n", sin(phi));
    //    printf("value %f\n", C - 2.0 * n * sin(phi));
    //assert(C - 2.0 * n * sin(phi) >=0);
    rho = sqrt(C - twiceN * sin(phi)) / n;

    *x_src = mp->distance * (rho * sin(theta));
    *y_src = mp->distance * (rho0 - rho * cos(theta) - yoffset);

    if (isnan(*x_src) ||
	isnan(*y_src)) {
	*x_src = 0;
	*y_src = 0;
	//	PrintError("false in alberts equal area 4");
	return 0;
    }

    assert(!isnan(*x_src));
    assert(!isnan(*y_src));

    return 1;
}

/** convert from albersequalareaconic to erect */
int erect_albersequalareaconic(double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
    double x, y, yoffset, lambda0, n, C, rho0, theta, phi, lambda, nsign;
    double rho2; // rho^2
    double n2; // n^2
    double twiceN; // n * 2.0
    
    //  Inverse calculation

    if (!albersEqualAreaConic_ParamCheck(mp->pn))  {
	*x_src = 0;
	*y_src = 0;
	//	printf("false in alberts equal area\n");
	return 0;
    }

    x = x_dest / mp->distance;
    y = y_dest / mp->distance;

    lambda0 = 0;

    n = mp->pn->precomputedValue[3];
    C = mp->pn->precomputedValue[4];
    rho0 = mp->pn->precomputedValue[5];
    yoffset = mp->pn->precomputedValue[6];
    n2 = mp->pn->precomputedValue[7];
    twiceN = mp->pn->precomputedValue[9];

    y = y + yoffset;

    rho2 = x*x + (rho0 - y)*(rho0 - y);
    nsign = 1.0;

    if (n < 0) nsign = -1.0;

    theta = atan2(nsign * x, nsign * (rho0 - y));

    phi = asin((C - rho2 * n2)/twiceN);

    lambda = lambda0 + theta / n;
    if (lambda > PI || lambda < -PI)  {
	*x_src = 0;
	*y_src = 0;
	//	PrintError("false in alberts equal area 2");
	return 0;
    }

    *x_src = mp->distance * lambda;
    *y_src = mp->distance * phi;

    if (isnan(*x_src) ||  
	isnan(*y_src)) {
	*x_src = 0;
	*y_src = 0;
	//	PrintError("false in alberts equal area 3");
	return 0;
    }

    assert(!isnan(*x_src));
    assert(!isnan(*y_src));

    return 1;
}

int albersequalareaconic_distance(double* x_src, void* params) {
    double x1, x2, y, phi1, phi2, lambda;

    //    printf("alber distance\n");

    if (!albersEqualAreaConic_ParamCheck(mp->pn))  {
	*x_src = 0;
	//	printf("false in alberts equal area distance 0\n");
	return 0;
    }

    mp->distance = 1;
    phi1 = mp->pn->precomputedValue[0];
    phi2 = mp->pn->precomputedValue[1];

    //lambda where x is a maximum.
    if ( (phi1 == phi2 && phi1 == 0.0) 
        || (phi1 == -phi2) )
    {
	// THIS IS A HACK...it needs to further studied
	// why this when phi1==phi2==0 
	// this functions return 0
	// Avoid approximation error
	// PrintError("The Albers projection cannot be used for phi1==phi2==0. Use Lambert Cylindrical Equal Area instead");

	*x_src = PI;
	return 0;
    }
    lambda = fabs(PI / (sin(phi1) + sin(phi2)));
    if (lambda > PI) lambda = PI;
    albersequalareaconic_erect(lambda, -PI/2.0, &x1, &y, mp);
    albersequalareaconic_erect(lambda, PI/2.0, &x2, &y, mp);
    *x_src = max(fabs(x1), fabs(x2));

    if (isnan(*x_src))  {
	*x_src = 0;
	PrintError("false in alberts equal area distance 1");
	return 0;
    }

    assert(!isnan(*x_src));

    //    printf("return albers distance %f\n", *x_src);

    return 1;

}


int sphere_cp_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam, double b

	register double phi, theta;
	phi 	= - x_dest /  ( var0 * PI / 2.0);
	theta 	=  - ( y_dest + var1 ) / ( PI / 2.0) ;
	
	*x_src =  theta * cos( phi );
	*y_src =  theta * sin( phi );
    return 1;
}


int sphere_tp_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam

	register double phi, theta, r,s;
	double v[3];

	phi 	= x_dest / distanceparam;
	theta 	=  - y_dest / distanceparam  + PI / 2;
	if(theta < 0)
	{
		theta = - theta;
		phi += PI;
	}
	if(theta > PI)
	{
		theta = PI - (theta - PI);
		phi += PI;
	}


	s = sin( theta );
	v[0] =  s * sin( phi );	//  y' -> x
	v[1] =  cos( theta );				//  z' -> y
	
	r = sqrt( v[1]*v[1] + v[0]*v[0]);	

	theta = distanceparam * atan2( r , s * cos( phi ) );
	
	*x_src =  theta * v[0] / r;
	*y_src =  theta * v[1] / r;
    return 1;
}


int erect_sphere_cp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam, double b

	register double phi, theta;

#if 0
	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / var0;
	phi   = atan2( y_dest , -x_dest );
	
	*x_src = var0 * phi;
	*y_src = var0 * theta - var1;
#endif
	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) ;
	phi   = atan2( y_dest , -x_dest );
	
	*x_src = var0 * phi;
	*y_src = theta - var1;
    return 1;
}


int rect_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam

	register double rho, theta,r;

#if 0	
	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / distanceparam;
	phi   = atan2( y_dest , x_dest );
	
	if( theta > PI /2.0  ||  theta < -PI /2.0 )
		theta = PI /2.0 ;

	rho = distanceparam * tan( theta );

	*x_src = rho * cos( phi );
	*y_src = rho * sin( phi );
#endif
	r 		= sqrt( x_dest * x_dest + y_dest * y_dest );
	theta 	= r / distanceparam;
	
	if( theta >= PI /2.0   )
		rho = 1.6e16 ;
	else if( theta == 0.0 )
		rho = 1.0;
	else
		rho =  tan( theta ) / theta;
	*x_src = rho * x_dest ;
	*y_src = rho * y_dest ;
    return 1;
}


int sphere_tp_rect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{	
	// params: double distanceparam

	register double  theta, r;

#if 0	
	theta = atan( sqrt(x_dest*x_dest + y_dest*y_dest) / *((double*)params));
	phi   = atan2( y_dest , x_dest );
	
	*x_src = *((double*)params) * theta * cos( phi );
	*y_src = *((double*)params) * theta * sin( phi );
#endif
	r 		= sqrt(x_dest*x_dest + y_dest*y_dest) / distanceparam;
	if( r== 0.0 )
		theta = 1.0;
	else
		theta 	= atan( r ) / r;
	
	*x_src =  theta * x_dest ;
	*y_src =  theta * y_dest ;
    return 1;
}


int sphere_tp_pano( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam

	register double r, s, Phi, theta;
	
#if 0
	register double Theta, phi;
	double v[3];
	
	Phi = x_dest / *((double*)params);
	Theta = PI /2.0 - atan( y_dest / distanceparam );
	

	v[2] = *((double*)params) * sin( Theta ) * cos( Phi );   //  x' -> z
	v[0] = *((double*)params) * sin( Theta ) * sin( Phi );	//  y' -> x
	v[1] = *((double*)params) * cos( Theta );				//  z' -> y
	
	theta = atan2( sqrt( v[0]*v[0] + v[1]*v[1] ) , v[2] );
	phi   = atan2( v[1], v[0] );
	
	*x_src = *((double*)params) * theta * cos( phi );
	*y_src = *((double*)params) * theta * sin( phi );
#endif
#if 1
	Phi = x_dest / distanceparam;
		
	s =  distanceparam * sin( Phi ) ;	//  y' -> x
	
	r = sqrt( s*s + y_dest*y_dest );
	theta = distanceparam * atan2( r , (distanceparam * cos( Phi )) ) / r;
	
	*x_src =  theta * s ;
	*y_src =  theta * y_dest ;
#endif
    return 1;
}


int pano_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam
	register double r,s, theta;
	double v[3];

#if 0	
	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / distanceparam;
	phi   = atan2( y_dest , x_dest );

	v[1] = *((double*)params) * sin( theta ) * cos( phi );   //  x' -> y
	v[2] = *((double*)params) * sin( theta ) * sin( phi );	//  y' -> z
	v[0] = *((double*)params) * cos( theta );				//  z' -> x

	theta = atan2( sqrt( v[0]*v[0] + v[1]*v[1] ) , v[2] );
	phi   = atan2( v[1], v[0] );

	*x_src = *((double*)params) * phi;
	*y_src = *((double*)params) * tan( (-theta + PI /2.0) );
#endif
	
	r = sqrt( x_dest * x_dest + y_dest * y_dest );
	theta = r / distanceparam;
	if( theta == 0.0 )
		s = 1.0 / distanceparam;
	else
		s = sin( theta ) /r;

	v[1] =  s * x_dest ;   //  x' -> y
	v[0] =  cos( theta );				//  z' -> x


	*x_src = distanceparam * atan2( v[1], v[0] );
	*y_src = distanceparam * s * y_dest / sqrt( v[0]*v[0] + v[1]*v[1] );

    return 1;
}


int sphere_cp_pano( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam

	register double phi, theta;
	
	
	phi 	= -x_dest / (distanceparam * PI / 2.0) ;
	theta	= PI /2.0 + atan( y_dest / (distanceparam * PI/2.0) );

	*x_src = distanceparam * theta * cos( phi );
	*y_src = distanceparam * theta * sin( phi );
    return 1;
}


int erect_rect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam
#if 0
	theta = atan( sqrt(x_dest*x_dest + y_dest*y_dest) / distanceparam );
	phi   = atan2( y_dest , x_dest );


	v[1] = distanceparam * sin( theta ) * cos( phi );   //  x' -> y
	v[2] = distanceparam * sin( theta ) * sin( phi );	//  y' -> z
	v[0] = distanceparam * cos( theta );				//  z' -> x
	
	theta = atan2( sqrt( v[0]*v[0] + v[1]*v[1] ) , v[2] );
	phi   = atan2( v[1], v[0] );

	*x_src = distanceparam * phi;
	*y_src = distanceparam * (-theta + PI /2.0);
#endif

	*x_src = distanceparam * atan2( x_dest, distanceparam );
	*y_src = distanceparam * atan2(  y_dest, sqrt( distanceparam*distanceparam + x_dest*x_dest ) );

    return 1;
}

/** convert erect to cartesian XYZ coordinates
 */
int cart_erect( double x_dest, double y_dest, double * xyz, double distance)
{
    // phi is azimuth (negative angle around y axis, starting at the z axis)
	double phi = x_dest / distance;	
	double theta_zenith = PI/2.0 - (y_dest / distance);
	// compute cartesian coordinates..
	//pos[2] = cos(-phi)*sin(theta_zenith);
	//pos[0] = sin(-phi)*sin(theta_zenith);
	//pos[1] = cos(theta_zenith);

    xyz[0] = sin(theta_zenith)*sin(phi);
	xyz[1] = cos(theta_zenith);
	xyz[2] = sin(theta_zenith)*-cos(phi);

	return 1;
}


/** convert cartesian coordinates into spherical ones
 */
int erect_cart(double * xyz, double *x_src, double *y_src, double distance)
{
	*x_src = atan2(xyz[0],-xyz[2]) * distance;
	*y_src = asin(xyz[1]/sqrt(xyz[0]*xyz[0]+xyz[1]*xyz[1]+xyz[2]*xyz[2])) * distance;

  return 1;
}


/** Compute intersection between line and point.
 *  n : a,b,c,d coefficients of plane (a,b,c = normal vector)
 *  p1: point on line
 *  p2: point on line
 *  See http://local.wasp.uwa.edu.au/~pbourke/geometry/planeline/
 */
int line_plane_intersection(double n[4],
							double p1[3],
							double p2[3],
							double * result)
{
	int i;
	// direction vector of line
	double d[3];
	double u,num,den;

	for (i=0;i<3;i++)
		d[i] = p2[i]-p1[i];
	num = n[0]*p1[0]+n[1]*p1[1]+n[2]*p1[2] + n[3];
	den = -n[0]*d[0]-n[1]*d[1]-n[2]*d[2];
	if (fabs(den) < 1e-15) {
		return 0;
	}
	u = num/den;

	if (u < 0) {
		// This is match is in the wrong direction, ignore
		return 0;
	}
	/* printf("intersect, dir: %f %f %f, num: %f, denom: %f, u: %f\n", d[0], d[1], d[2], num, den, u);
	 */

	for (i=0;i<3;i++)
	  result[i] = p1[i]+u*d[i];

	return 1;
}

/** transfer a point from the master camera through a plane into camera 
 *  at TrX, TrY, TrZ using the plane located at Te0 (yaw), Te1 (pitch)
 */
int plane_transfer_to_camera( double x_dest, double y_dest, double * x_src, double * y_src, void * params)
{
	// params: distance, x1,y1,z1

	double plane_coeff[4];
	double p1[3];
	double p2[3];
	double intersection[3];

	// compute ray of sight for the current pixel in
	// the master panorama camera.
	// camera point
	p1[0] = p1[1] = p1[2] = 0;
	// point on sphere.
	cart_erect(x_dest, y_dest, &p2[0], mp->distance);

	// compute plane description
    cart_erect(mp->trans[3], -mp->trans[4],
			   &plane_coeff[0], 1.0);

	// plane_coeff[0..2] is both the normal and a point
	// on the plane.
	plane_coeff[3] = - plane_coeff[0]*plane_coeff[0]
		             - plane_coeff[1]*plane_coeff[1]
		             - plane_coeff[2]*plane_coeff[2];

	/*
	printf("Plane: y:%f p:%f coefficients: %f %f %f %f, ray direction: %f %f %f\n", 
	       mp->trans[3], mp->trans[4], plane_coeff[0], plane_coeff[1], plane_coeff[2], plane_coeff[3],
		   p2[0],p2[1],p2[2]);
	*/

	// perform intersection.

	if (!line_plane_intersection(plane_coeff, p1, p2, &intersection[0])) {
		// printf("No intersection found, %f %f %f\n", p2[0], p2[1], p2[2]);
		return 0;
	}

	// compute ray leading to the camera.
	intersection[0] -= mp->trans[0];
	intersection[1] -= mp->trans[1];
	intersection[2] -= mp->trans[2];

	// transform into erect
	erect_cart(&intersection[0], x_src, y_src, mp->distance);

	/*
	printf("pano->plane->cam(%.1f, %.1f, %.1f, y:%1f,p:%1f): %8.5f %8.5f -> %8.5f %8.5f %8.5f -> %8.5f %8.5f\n",
		   mp->trans[0], mp->trans[1], mp->trans[2], mp->trans[3], mp->trans[4],
		   x_dest, y_dest, 
		   intersection[0], intersection[1], intersection[2],
		   *x_src, *y_src);
	*/

	return 1;
}


/** transfer a point from a camera centered at x1,y1,z1 into the camera at x2,y2,z2 */
int plane_transfer_from_camera( double x_dest, double y_dest, double * x_src, double * y_src, void * params)
{

	double plane_coeff[4];
	double p1[3];
	double p2[3];
	double intersection[3];

	// params: MakeParams

	// compute ray of sight for the current pixel in
	// the master panorama camera.
	// camera point
	p1[0] = mp->trans[0];
	p1[1] = mp->trans[1];
	p1[2] = mp->trans[2];

	// point on sphere (direction vector in camera coordinates)
	cart_erect(x_dest, y_dest, &p2[0], mp->distance);
	// add camera position to get point on ray
	p2[0] += p1[0];
	p2[1] += p1[1];
	p2[2] += p1[2];	


	// compute plane description
	cart_erect(mp->trans[3], -mp->trans[4],
			   &plane_coeff[0], 1.0);

	// plane_coeff[0..2] is both the normal and a point
	// on the plane.
	plane_coeff[3] = - plane_coeff[0]*plane_coeff[0]
		             - plane_coeff[1]*plane_coeff[1]
		             - plane_coeff[2]*plane_coeff[2];

	/*
	printf("Plane: y:%f p:%f coefficients: %f %f %f %f, ray direction: %f %f %f\n", 
	       mp->trans[3], mp->trans[4], plane_coeff[0], plane_coeff[1], plane_coeff[2], plane_coeff[3],
		   p2[0],p2[1],p2[2]);
	*/


	// compute intersection
	if (!line_plane_intersection(plane_coeff, p1, p2, &intersection[0])) {
		//printf("No intersection found, %f %f %f\n", p2[0], p2[1], p2[2]);
		return 0;
	}

	// the intersection vector is the vector of the ray of sight from
	// the master panorama camera.

	// transform into erect
	erect_cart(&intersection[0], x_src, y_src, mp->distance);

	/*
	printf("cam->plane->pano(%.1f, %.1f, %.1f, y:%1f,p:%1f): %8.5f %8.5f -> %8.5f %8.5f %8.5f -> %8.5f %8.5f\n",
		   mp->trans[0], mp->trans[1], mp->trans[2], mp->trans[3], mp->trans[4],
		   x_dest, y_dest, 
		   intersection[0], intersection[1], intersection[2],
		   *x_src, *y_src);
		   
	*/

	return 1;
}



/** convert from erect to biplane */
int biplane_erect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params )
{
	double x,offset;
	if (fabs(x_dest / mp->distance) > mp->pn->precomputedValue[0]+DEG_TO_RAD(89))
	{
		*x_src = 0;
		*y_src = 0;
		return 0;
	}
	if(x_dest<0)
	{
		x = x_dest + mp->pn->precomputedValue[0] * mp->distance;
		offset = -mp->pn->precomputedValue[1];
	}
	else
	{
		x = x_dest - mp->pn->precomputedValue[0] * mp->distance;
		offset = mp->pn->precomputedValue[1];
	}
	rect_erect(x,y_dest,x_src,y_src,&mp->distance);
	*x_src += offset;
	return 1;
}

/** convert from biplane to erect */
int erect_biplane		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params )
{
	double x, offset;
	if(fabs(x_dest)>mp->pn->precomputedValue[1]+mp->distance*57)  // 57 = tan(89)
	{
		*x_src = 0;
		*y_src = 0;
		return 0;
	}
	if(x_dest<0)
	{
		x=x_dest + mp->pn->precomputedValue[1];
		offset = - mp->pn->precomputedValue[0];
	}
	else
	{
		x=x_dest - mp->pn->precomputedValue[1];
		offset = mp->pn->precomputedValue[0]; 
	} 
	erect_rect(x,y_dest,x_src,y_src,&mp->distance);
	*x_src += offset * mp->distance;
	return 1;
}


int biplane_distance ( double width, double b, void* params )
{
	if(mp->pn->formatParamCount==0)
	{
		mp->pn->formatParamCount = 1;
		mp->pn->formatParam[0] = 45;
	};
	mp->pn->formatParam[0]= max( min(mp->pn->formatParam[0], 179), 1);

	mp->pn->precomputedCount = 2;
	mp->pn->precomputedValue[0] = DEG_TO_RAD(mp->pn->formatParam[0]) / 2;  // angle in rad
	mp->distance = (double) width / (2.0 * (tan(mp->pn->precomputedValue[0])+tan(b/2.0 - mp->pn->precomputedValue[0])));
	mp->pn->precomputedValue[1]=mp->distance*tan(mp->pn->precomputedValue[0]);  // offset
	return 1;
}

int triplane_erect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params )
{
	double x,offset;
	if(fabs(x_dest / mp->distance)> mp->pn->precomputedValue[0] + DEG_TO_RAD(89))
	{
		*x_src = 0;
		*y_src = 0;
		return 0;
	};
	if(x_dest < -mp->pn->precomputedValue[0] / 2)
	{
		x=x_dest + mp->pn->precomputedValue[0] * mp->distance;
		offset = - mp->pn->precomputedValue[1];
	}
	else if (x_dest < mp->pn->precomputedValue[0] / 2)
	{
		x=x_dest;
		offset=0;
	}
	else
	{
		x=x_dest - mp->pn->precomputedValue[0] * mp->distance;
		offset = + mp->pn->precomputedValue[1];
	}
	rect_erect(x,y_dest,x_src,y_src,&mp->distance);
	*x_src += offset;
	return 1;
}

int erect_triplane		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params )
{
	double x, offset;
	if(fabs(x_dest) > 2* mp->pn->precomputedValue[1] + 57 * mp->distance )
	{
		*x_src = 0;
		*y_src = 0;
		return 0;
	};
	if(x_dest < -mp->pn->precomputedValue[1])
	{
		x=x_dest + 2 * mp->pn->precomputedValue[1];
		offset = - mp->pn->precomputedValue[0];
	}
	else if (x_dest < mp->pn->precomputedValue[1])
	{
		x=x_dest;
		offset=0;
	}
	else
	{
		x=x_dest - 2 * mp->pn->precomputedValue[1];
		offset = + mp->pn->precomputedValue[0];
	}
	erect_rect(x,y_dest,x_src ,y_src,&mp->distance);
	*x_src += offset * mp->distance;
	return 1;
}

int triplane_distance ( double width, double b, void* params )
{
	if(mp->pn->formatParamCount==0)
	{
		mp->pn->formatParamCount = 1;
		mp->pn->formatParam[0] = 45;
	};
	mp->pn->formatParam[0] = max( min(mp->pn->formatParam[0], 120), 1);

	mp->pn->precomputedCount = 2;
	mp->pn->precomputedValue[0] = DEG_TO_RAD(mp->pn->formatParam[0]);  // angle in rad
	mp->distance = (double) width / (4.0 * tan(mp->pn->precomputedValue[0]/2.0) + 2 * tan(b/2.0 - mp->pn->precomputedValue[0]));
	mp->pn->precomputedValue[1]=mp->distance*tan(mp->pn->precomputedValue[0]/2.0);  // offset
	return 1;
}




int erect_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam

	register double  theta,r,s;
	double	v[3];
#if 0	
	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / *((double*)params);
	phi   = atan2( y_dest , x_dest );
	
	v[1] = *((double*)params) * sin( theta ) * cos( phi );   //  x' -> y
	v[2] = *((double*)params) * sin( theta ) * sin( phi );	//  y' -> z
	v[0] = *((double*)params) * cos( theta );				//  z' -> x
	
	theta = atan( sqrt( v[0]*v[0] + v[1]*v[1] ) / v[2] ); //was atan2
	phi   = atan2( v[1], v[0] );

	*x_src = *((double*)params) * phi;
	if(theta > 0.0)
	{
		*y_src = *((double*)params) * (-theta + PI /2.0);
	}
	else
		*y_src = *((double*)params) * (-theta - PI /2.0);
#endif

	r = sqrt( x_dest * x_dest + y_dest * y_dest );
	theta = r / distanceparam;
	if(theta == 0.0)
		s = 1.0 / distanceparam;
	else
		s = sin( theta) / r;
	
	v[1] =  s * x_dest;   
	v[0] =  cos( theta );				
	

	*x_src = distanceparam * atan2( v[1], v[0] );
	*y_src = distanceparam * atan( s * y_dest /sqrt( v[0]*v[0] + v[1]*v[1] ) ); 
    return 1;
}


int mirror_sphere_cp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam, double b

	register double rho, phi, theta;

	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / ((double*)params)[0];
	phi   = atan2( y_dest , x_dest );
	
	rho = ((double*)params)[1] * sin( theta / 2.0 );
	
	*x_src = - rho * cos( phi );
	*y_src = rho * sin( phi );
    return 1;
}


int mirror_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam, double b, double b2

	register double phi, theta, rho;
	
	phi 	=  x_dest / ( ((double*)params)[0] * PI/2.0) ;
	theta 	=  - ( y_dest + ((double*)params)[2] ) / (((double*)params)[0] * PI/2.0)  ;
	
	rho = ((double*)params)[1] * sin( theta / 2.0 );
	
	*x_src = - rho * cos( phi );
	*y_src = rho * sin( phi );
    return 1;
}


int mirror_pano( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam, double b

	register double phi, theta, rho;
	
	
	phi 	= -x_dest / (((double*)params)[0] * PI/2.0) ;
	theta	= PI /2.0 + atan( y_dest / (((double*)params)[0] * PI/2.0) );

	rho = ((double*)params)[1] * sin( theta / 2.0 );
	
	*x_src = rho * cos( phi );
	*y_src = rho * sin( phi );
    return 1;
}


int sphere_cp_mirror( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam, double b

	register double phi, theta, rho;

	rho = sqrt( x_dest*x_dest + y_dest*y_dest );
	
	theta = 2 * asin( rho/((double*)params)[1] );
	phi   = atan2( y_dest , x_dest );

	*x_src = ((double*)params)[0] * theta * cos( phi );
	*y_src = ((double*)params)[0] * theta * sin( phi );
    return 1;
}

int sphere_tp_mirror( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
  register double c;
  register double normalizedX, normalizedY;
  register double azi;

  normalizedX = x_dest/ distanceparam;
  normalizedY = y_dest/ distanceparam;

  c = 2 * asin(hypot(normalizedX, normalizedY));
  azi = atan2( y_dest , x_dest );

  *x_src = distanceparam * c * cos(azi);
  *y_src = distanceparam * c * sin(azi);

  return 1;
}

int mirror_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{

  register double c;
  register double normalizedX, normalizedY;
  register double azi;
  normalizedY = y_dest/ distanceparam;
  normalizedX = x_dest/ distanceparam;
  

  c = hypot(normalizedX, normalizedY);
  azi = atan2( y_dest , x_dest );

  *x_src = distanceparam * sin(c/2) * cos(azi);
  *y_src = distanceparam * sin(c/2) * sin(azi);

  return 1;
}

int sphere_tp_equisolid( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
  // params: double distance

  register double phi, theta, rho;

  rho = sqrt( x_dest*x_dest + y_dest*y_dest );
  
  theta = 2.0 * asin( rho/(2.0*((double*)params)[0]) );
  phi   = atan2( y_dest , x_dest );

  *x_src = ((double*)params)[0] * theta * cos( phi );
  *y_src = ((double*)params)[0] * theta * sin( phi );
  return 1;
}


int equisolid_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
  // params: double distance

  register double rho, phi, theta;

  theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / ((double*)params)[0];
  phi   = atan2( y_dest , x_dest );
  
  rho = 2.0 * ((double*)params)[0] * sin( theta / 2.0 );
  
  *x_src = rho * cos( phi );
  *y_src = rho * sin( phi );
  return 1;
}

int sphere_tp_orthographic( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
  // params: double distance

  register double phi, theta, rho;

  rho = sqrt( x_dest*x_dest + y_dest*y_dest );
  
  // orthographic projection is limited to fov of 180 deg
  if(rho>((double*)params)[0])
  {
      *x_src = 0;
      *y_src = 0;
      return 0;
  };
  theta = 1.0 * asin( rho/(1.0*((double*)params)[0]) );
  phi   = atan2( y_dest , x_dest );
  

  *x_src = ((double*)params)[0] * theta * cos( phi );
  *y_src = ((double*)params)[0] * theta * sin( phi );

  return 1;
}

int orthographic_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
  // params: double distance

  register double rho, phi, theta;

  theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / ((double*)params)[0];
  phi   = atan2( y_dest , x_dest );
  //orthographic projection is limited to fov of 180 deg
  if(fabs(theta)>HALF_PI)
  {
      *x_src=0;
      *y_src=0;
      return 0;
  };

  rho = 1.0 * ((double*)params)[0] * sin( theta / 1.0 );
  
  *x_src = rho * cos( phi );
  *y_src = rho * sin( phi );
  return 1;
}

// the Thoby projection is an empirically found projection for the Nikkor 10.5 lens
// rho = THOBY_K1_PARM * sin( theta  * THOBY_K2_PARM);

int sphere_tp_thoby( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
  // params: double distance

  register double phi, theta, rho;
#define SCALE (((double*)params)[0])

  rho = sqrt( x_dest*x_dest + y_dest*y_dest )/ SCALE;

  if(fabs(rho)>THOBY_K1_PARM)
  {
    *x_src=0;
    *y_src=0;
    return 0;
  };
  theta = asin( rho/THOBY_K1_PARM) / THOBY_K2_PARM;
  phi   = atan2( y_dest , x_dest );
  
  *x_src = SCALE * theta * cos( phi );
  *y_src = SCALE * theta * sin( phi );

#undef SCALE

  return 1;
}


int thoby_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
  // params: double distance

  register double rho, phi, theta;

#define SCALE (((double*)params)[0])

  theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / SCALE;
  phi   = atan2( y_dest , x_dest );
  
  rho = THOBY_K1_PARM * sin( theta  * THOBY_K2_PARM );
  
  *x_src = SCALE * rho * cos( phi );
  *y_src = SCALE * rho * sin( phi );

#undef SCALE
  return 1;
}


int shift_scale_rotate( double x_dest,double  y_dest, double* x_src, double* y_src, void* params){
	// params: double shift_x, shift_y, scale, cos_phi, sin_phi
	
	register double x = x_dest - ((double*)params)[0];
	register double y = y_dest - ((double*)params)[1];

	*x_src = (x * ((double*)params)[3] - y * ((double*)params)[4]) * ((double*)params)[2];
	*y_src = (x * ((double*)params)[4] + y * ((double*)params)[3]) * ((double*)params)[2];
    return 1;
}





// Correct radial luminance change using parabel

unsigned char radlum( unsigned char srcPixel, int xc, int yc, void *params )
{
	// params: second and zero order polynomial coeff
	register double result;

	result = (xc * xc + yc * yc) * ((double*)params)[0] + ((double*)params)[1];
	result = ((double)srcPixel) - result;

    // JMW 2003/08/25  randomize a little
    result = result * ( (1 + LUMINANCE_RANDOMIZE/2) - LUMINANCE_RANDOMIZE * rand() / (double)RAND_MAX );
    
	if(result < 0.0) return 0;
	if(result > 255.0) return 255;

	return( (unsigned char)(result+0.5) );
}

//Kekus 16 bit: 2003/Nov/18
//Correct radial luminance change using parabel (16-bit supported)
unsigned short radlum16( unsigned short srcPixel, int xc, int yc, void *params ) 
{
	// params: second and zero order polynomial coeff
	register double result;

	result = (xc * xc + yc * yc) * ((double*)params)[0] + ((double*)params)[1];
    result = ((double) srcPixel) - result*256;
    // JMW 2003/08/25  randomize a little to remove banding added by Kekus Digital 26 Aug 2003
	// JMW 2004/07/11 a power of two less randomizing for 16 bit
    result = result * ( (1 + LUMINANCE_RANDOMIZE * LUMINANCE_RANDOMIZE /2) - 
		LUMINANCE_RANDOMIZE * LUMINANCE_RANDOMIZE * rand() / (double)RAND_MAX );
    if(result > 65535.0) return 65535;
	if(result < 0.0) return 0;

	return( (unsigned short)(result+0.5) );
}
//Kekus.

// Get smallest positive (non-zero) root of polynomial with degree deg and
// (n+1) real coefficients p[i]. Return it, or 1000.0 if none exists or error occured
// Changed to only allow degree 3
#if 0
double smallestRoot( double *p )
{
	doublecomplex 		root[3], poly[4];
	doublereal 			radius[3], apoly[4], apolyr[4];
	logical 			myErr[3];
	double 				sRoot = 1000.0;
	doublereal 			theEps, theBig, theSmall;
	integer 			nitmax;
	integer 			iter;
	integer 			n,i;
	
	n 		= 3;

	
	for( i=0; i< n+1; i++)
	{
		poly[i].r = p[i];
		poly[i].i = 0.0;
	}
	
	theEps   = DBL_EPSILON;  		// machine precision 
	theSmall = DBL_MIN ; 			// smallest positive real*8          
	theBig   = DBL_MAX ; 			// largest real*8  

	nitmax 	= 100;

    polzeros_(&n, poly, &theEps, &theBig, &theSmall, &nitmax, root, radius, myErr, &iter, apoly, apolyr);

	for( i = 0; i < n; i++ )
	{
//		PrintError("No %d : Real %g, Imag %g, radius %g, myErr %ld", i, root[i].r, root[i].i, radius[i], myErr[i]);
		if( (root[i].r > 0.0) && (dabs( root[i].i ) <= radius[i]) && (root[i].r < sRoot) )
			sRoot = root[i].r;
	}

	return sRoot;
}
#endif



void cubeZero( double *a, int *n, double *root ) {
	if( a[3] == 0.0 ){ // second order polynomial
		squareZero( a, n, root );
	}else{
		double p = ((-1.0/3.0) * (a[2]/a[3]) * (a[2]/a[3]) + a[1]/a[3]) / 3.0;
		double q = ((2.0/27.0) * (a[2]/a[3]) * (a[2]/a[3]) * (a[2]/a[3]) - (1.0/3.0) * (a[2]/a[3]) * (a[1]/a[3]) + a[0]/a[3]) / 2.0;
		
		if( q*q + p*p*p >= 0.0 ){
			*n = 1;
			root[0] = cubeRoot(-q + sqrt(q*q + p*p*p)) + cubeRoot(-q - sqrt(q*q + p*p*p)) - a[2] / (3.0 * a[3]); 
		}else{
			double phi = acos( -q / sqrt(-p*p*p) );
			*n = 3;
			root[0] =  2.0 * sqrt(-p) * cos(phi/3.0) - a[2] / (3.0 * a[3]); 
			root[1] = -2.0 * sqrt(-p) * cos(phi/3.0 + PI/3.0) - a[2] / (3.0 * a[3]); 
			root[2] = -2.0 * sqrt(-p) * cos(phi/3.0 - PI/3.0) - a[2] / (3.0 * a[3]); 
		}
	}
	// PrintError("%lg, %lg, %lg, %lg root = %lg", a[3], a[2], a[1], a[0], root[0]);
}

void squareZero( double *a, int *n, double *root ){
	if( a[2] == 0.0 ){ // linear equation
		if( a[1] == 0.0 ){ // constant
			if( a[0] == 0.0 ){
				*n = 1; root[0] = 0.0;
			}else{
				*n = 0;
			}
		}else{
			*n = 1; root[0] = - a[0] / a[1];
		}
	}else{
		if( 4.0 * a[2] * a[0] > a[1] * a[1] ){
			*n = 0; 
		}else{
			*n = 2;
			root[0] = (- a[1] + sqrt( a[1] * a[1] - 4.0 * a[2] * a[0] )) / (2.0 * a[2]);
			root[1] = (- a[1] - sqrt( a[1] * a[1] - 4.0 * a[2] * a[0] )) / (2.0 * a[2]);
		}
	}

}

double cubeRoot( double x ){
	if( x == 0.0 )
		return 0.0;
	else if( x > 0.0 )
		return pow(x, 1.0/3.0);
	else
		return - pow(-x, 1.0/3.0);
}

double smallestRoot( double *p ){
	int n,i;
	double root[3], sroot = 1000.0;
	
	cubeZero( p, &n, root );
	
	for( i=0; i<n; i++){
		// PrintError("Root %d = %lg", i,root[i]);
		if(root[i] > 0.0 && root[i] < sroot)
			sroot = root[i];
	}
	
	// PrintError("Smallest Root  = %lg", sroot);
	return sroot;
}



