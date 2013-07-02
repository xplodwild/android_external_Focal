/* Panorama_Tools	-	Generate, Edit and Convert Panoramic Images
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

// Added Fix of Kekus Kratzke: March.2004
// Radial Shift, when colors channels have different values and
// d is > 1 would give incorrect results around the edge of the image

// Modified by Fulvio Senore: June.2004
// Added linear interpolation between pixels in the geometric transform phase
// to speed up computation.
// Rik Littlefield added interface to morpher.c in July 2004 to avoid local errors caused
// by morphing.
// Changes are bracketed between
//
// // FS+
//
// and
//
// // FS-
//
// comments
/*------------------------------------------------------------
 JMW - merged in Rob Platt changes Oct 18, 2005
      11-June-2004 Rob Platt - Modified MyTransForm() for multithreading.
                               launch one child task per CPU. 
                               Since this does no file or net I/O, there is no advantage to more tasks than CPUs.
      12-June-2004 ..        - Run one instance of MyTransFormBody() in the context of the parent
                               and launch (n-1) child tasks. The parent updates the Progress indicator
                             - transForm() now calls MyTransForm(); this makes transForm multithreaded, too.
                               Eliminated duplicate code. 

      26-July-2004 Rob Platt - Get version from the version.h rather than always updating it here...
                   ..        - 1st step: Add Handling of 2-color resampling (Use same method as 
				               3-color, just display a different progress indicator)
                               Define 3 new 'composite' colors (R+G, R+B, G+B). See correct.c
      27-July-2004           - K.K. sent me the bugfix if different factors were set for each 
	                           color channel, merged with my mods.
      28-July-2004 ..        - Clean handling of 2-color resampling: The third color is left untouched.

      17-Aug-2004 R.Platt    - test out of memory condition (Just to be safe)
                             - #ifdef the parts not needed on Mac
      10-Sep-2005 R.Platt    - Testing for alphamasked pixels to we can interpolate
                               right up to the edge of an alphamasked region
                               without introducing contributions of non-existent pixels
                               at the edge of the interpolated input
      13-Sep-2005 R.Platt    - Renormalization when sampling a region that some pixels are masked and others not.
      15-Oct-2005 R.Platt    - Changing alphamask threshold to 94% (15/16ths) due to strange masks from Photoshop
                            
------------------------------------------------------------*/


// Program specific includes
#include "version.h"
#include "filter.h" 			

// Standard C includes

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#if _MSC_VER > 1000
#pragma warning(disable: 4100) // disable unreferenced formal parameter warning
#endif


/*------------------------------------------------------------
PROTOTYPES:
*/
//static OSStatus MyTransFormBody( MyTransFormCmdPara_s *MyTransFormCmdPara);
void MyTransForm( TrformStr *TrPtr, fDesc *fD, int color, int imageNum);
//static void FindYbounds( MyTransFormCmdPara_s *FindYboundsPara);
void transForm_aa( TrformStr *TrPtr, fDesc *fD,fDesc *finvD, int color, int imageNum);



// 			This file uses functions of type
// 	resample( unsigned char *dst, 	unsigned char **rgb,
//							register double Dx, 
//							register double Dy,
//							int color, int SamplesPerPixel);
//
// dst - output pixel
// rgb - input pixels, may be Lab as well.
// Dx  - offset of output pixel position in x-direction
// Dy  - offset of output pixel position in y-direction
// color = 0: all rgb colors; color = 1,2,3: one of r,g,b
// color=4,5,6:process 2 channels (4:r+g, 5:r+b, 6:g+b)
// BytesPerPixel = 3,4. Using color != 0, any value should (?) work.



// Arrays used for Gamma correction
PTGamma glu; // Lookup table

// prototype to avoid a warning: the function is defined in morpher.c
int getLastCurTriangle();


// Some locally needed math functions
static double 	sinc		( double x );
static double 	cubic01		( double x );
static double 	cubic12		( double x ); 



// Interpolators

static void nn( unsigned char *dst, unsigned char **rgb, 
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void bil( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void poly3( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void spline16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void spline36( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);

static void spline64( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);

static void sinc256( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);

static void sinc1024( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);

static void nn_16( unsigned char *dst, unsigned char **rgb, 
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void bil_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void poly3_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void spline16_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void spline36_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void spline64_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);

static void sinc256_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);

static void sinc1024_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);






// Various interpolators; a[] is array of coeeficients; 0 <= x < 1


#define 	NNEIGHBOR(x, a , NDIM )											\
			a[0] = 1.0;	


#define 	BILINEAR(x, a, NDIM )											\
			a[1] = x;														\
			a[0] = 1.0 - x;	
			

// Unused; has been replaced by 'CUBIC'.

#define 	POLY3( x, a , NDIM )											\
			a[3] = (  x * x - 1.0) * x / 6.0;								\
			a[2] = ( (1.0 - x) * x / 2.0 + 1.0) * x; 						\
			a[1] = ( ( 1.0/2.0  * x - 1.0 ) * x - 1.0/2.0 ) * x + 1.0;		\
			a[0] = ( ( -1.0/6.0 * x + 1.0/2.0 ) * x - 1.0/3.0 ) * x ;



#define		SPLINE16( x, a, NDIM )											\
			a[3] = ( ( 1.0/3.0  * x - 1.0/5.0 ) * x -   2.0/15.0 ) * x;		\
			a[2] = ( ( 6.0/5.0 - x     ) * x +   4.0/5.0 ) * x;				\
			a[1] = ( ( x - 9.0/5.0 ) * x -   1.0/5.0     ) * x + 1.0;		\
			a[0] = ( ( -1.0/3.0 * x + 4.0/5.0     ) * x -   7.0/15.0 ) * x ;


#define		CUBIC( x, a, NDIM )											\
			a[3] = cubic12( 2.0 - x);									\
			a[2] = cubic01( 1.0 - x);									\
			a[1] = cubic01( x );										\
			a[0] = cubic12( x + 1.0);									\




#define		SPLINE36( x, a , NDIM )														\
	a[5] = ( ( -  1.0/11.0  * x +  12.0/ 209.0 ) * x +   7.0/ 209.0  ) * x;				\
	a[4] = ( (    6.0/11.0  * x -  72.0/ 209.0 ) * x -  42.0/ 209.0  ) * x;				\
	a[3] = ( ( - 13.0/11.0  * x + 288.0/ 209.0 ) * x + 168.0/ 209.0  ) * x;				\
	a[2] = ( (   13.0/11.0  * x - 453.0/ 209.0 ) * x -   3.0/ 209.0  ) * x + 1.0;		\
	a[1] = ( ( -  6.0/11.0  * x + 270.0/ 209.0 ) * x - 156.0/ 209.0  ) * x;				\
	a[0] = ( (    1.0/11.0  * x -  45.0/ 209.0 ) * x +  26.0/ 209.0  ) * x;



#define		SPLINE64( x, a , NDIM )														\
	a[7] = ((  1.0/41.0 * x -   45.0/2911.0) * x -   26.0/2911.0) * x;					\
	a[6] = ((- 6.0/41.0 * x +  270.0/2911.0) * x +  156.0/2911.0) * x;					\
	a[5] = (( 24.0/41.0 * x - 1080.0/2911.0) * x -  624.0/2911.0) * x;					\
	a[4] = ((-49.0/41.0 * x + 4050.0/2911.0) * x + 2340.0/2911.0) * x;					\
	a[3] = (( 49.0/41.0 * x - 6387.0/2911.0) * x -    3.0/2911.0) * x + 1.0;			\
	a[2] = ((-24.0/41.0 * x + 4032.0/2911.0) * x - 2328.0/2911.0) * x;					\
	a[1] = ((  6.0/41.0 * x - 1008.0/2911.0) * x +  582.0/2911.0) * x;					\
	a[0] = ((- 1.0/41.0 * x +  168.0/2911.0) * x -   97.0/2911.0) * x;					


#define		SINC( x, a, NDIM )										\
	{																\
		register int idx;											\
		register double xadd;										\
		for( idx = 0, xadd = NDIM / 2 - 1.0 + x; 					\
			 idx < NDIM / 2; 										\
			 xadd-=1.0)												\
		{															\
			a[idx++] = sinc( xadd ) * sinc( xadd / ( NDIM / 2 ));	\
		}															\
		for( xadd = 1.0 - x; 										\
			 idx < NDIM; 											\
			 xadd+=1.0)												\
		{															\
			a[idx++] = sinc( xadd ) * sinc( xadd / ( NDIM / 2 ));	\
		}															\
	}																\
		








// Set up the arrays for gamma correction

int SetUpGamma( double pgamma, unsigned int psize)
{
	int i;
	double gnorm, xg, rgamma = 1.0/pgamma;

	if( psize == 1 )
	{
		glu.ChannelSize 	=   256;
		glu.ChannelStretch 	=    16;
	}
	else if( psize == 2 )
	{
		glu.ChannelSize 	= 65536;
		glu.ChannelStretch 	= 	  4;
	}
	else
		return -1;

	glu.GammaSize = glu.ChannelSize * glu.ChannelStretch;
	
	glu.DeGamma 	= NULL;
	glu.Gamma  		= NULL;
	glu.DeGamma 	= (double*) 		malloc( glu.ChannelSize * sizeof( double ) );
	glu.Gamma  		= (unsigned short*) malloc( glu.GammaSize * sizeof( unsigned short) );
	
	if( glu.DeGamma == NULL || glu.Gamma == NULL )
	{
		PrintError("Not enough memory");
		return -1;
	}

	glu.DeGamma[0] = 0.0;
	gnorm = (glu.ChannelSize-1) / pow( glu.ChannelSize-1 , pgamma ) ; 
	for(i=1; i<glu.ChannelSize; i++)
	{
		glu.DeGamma[i] = pow( (double)i , pgamma ) * gnorm;
	}

	glu.Gamma[0] = 0;
	gnorm = (glu.ChannelSize-1) /  pow( glu.ChannelSize-1 , rgamma ) ; 
	if( psize == 1 )
	{
		for(i=1; i<glu.GammaSize; i++)
		{
			xg	 = pow(  ((double)i) / glu.ChannelStretch , rgamma ) * gnorm;
			DBL_TO_UC( glu.Gamma[i], xg );
		}
	}
	else
	{
		for(i=1; i<glu.GammaSize; i++)
		{
			xg	 = pow(  ((double)i) / glu.ChannelStretch , rgamma ) * gnorm;
			DBL_TO_US( glu.Gamma[i], xg );
		}
	}
	return 0;
}

unsigned short gamma_correct( double pix )
{
	int k = (int)(glu.ChannelStretch * pix);
	if( k < 0 )
		return 0;
	if( k > glu.GammaSize - 1 )
		return glu.ChannelSize - 1;
	return (glu.Gamma)[ k ] ;
}

#define gamma_char(pix) (char)(gamma_correct(pix))
#define gamma_short(pix) (short)(gamma_correct(pix))
#define gamma_float(pix) (float)(pix)

#define degamma_char(pix) glu.DeGamma[pix]
#define degamma_short(pix) glu.DeGamma[pix]
#define degamma_float(pix) pix


/////////// N x N Sampler /////////////////////////////////////////////


#define RESAMPLE_N( intpol, ndim, psize )                               \
    double ya[ndim];                                                    \
    double yr[ndim], yg[ndim], yb[ndim], w[ndim];                       \
    register double ad;                                                 \
    register double rd, gd, bd, weight ;                                \
    register int k,i;                                                   \
    register unsigned psize *r, *ri;                                    \
    register unsigned psize *tdst;                                      \
    int alpha_ok = TRUE;                                                \
                                                                        \
    intpol( Dx, w, ndim )                                               \
    if( color == 0)                                                     \
    {                                                                   \
        for(k=0; k<ndim; k++)                                           \
        {                                                               \
            r = ((unsigned psize**)rgb)[k];                             \
            ad = 0.0;                                                   \
            rd = gd = bd = 0.0;                                         \
                                                                        \
            for(i=0; i<ndim; i++)                                       \
            {                                                           \
                weight = w[ i ];                                        \
                ri     = r + i * SamplesPerPixel;                       \
                if(SamplesPerPixel==4)                                  \
                {                                                       \
                    if( ((int)*ri++) < threshold)                       \
                        alpha_ok = FALSE;                               \
                    else                                                \
                    {                                                   \
                        ad += weight;                                   \
						rd += degamma_##psize((int)*ri++) * weight;         \
						gd += degamma_##psize((int)*ri++) * weight;         \
						bd += degamma_##psize((int)*ri)   * weight;         \
                    }                                                   \
                }                                                       \
                else                                                    \
                {                                                       \
					rd += degamma_##psize((int)*ri++) * weight;             \
					gd += degamma_##psize((int)*ri++) * weight;             \
					bd += degamma_##psize((int)*ri)   * weight;             \
                }                                                       \
            }                                                           \
            ya[k] = ad;                                                 \
            yr[k] = rd; yg[k] = gd; yb[k] = bd;                         \
        }                                                               \
                                                                        \
        intpol( Dy, w, ndim )                                           \
        ad = 0.0;                                                       \
        rd = gd = bd = 0.0;                                             \
                                                                        \
        for(i=0; i<ndim; i++)                                           \
        {                                                               \
            weight = w[ i ];                                            \
            ad += ya[i] * weight;                                       \
            rd += yr[i] * weight;                                       \
            gd += yg[i] * weight;                                       \
            bd += yb[i] * weight;                                       \
        }                                                               \
                                                                        \
        if(!alpha_ok)                                                   \
        {                                                               \
            if(ad>0.5)                                                  \
            {   /* Renormalize */                                       \
                weight = 1.0/ad;                                        \
                rd *= weight;                                           \
                gd *= weight;                                           \
                bd *= weight;                                           \
                alpha_ok = TRUE;                                        \
            }                                                           \
            else                                                        \
            {                                                           \
                rd=gd=bd=0.0;                                           \
            }                                                           \
        }                                                               \
                                                                        \
        tdst = (unsigned psize *)dst;                                   \
        if(SamplesPerPixel==4)                                          \
        {                                                               \
            if(alpha_ok)                                                \
                *tdst++  =  maxalpha;                                   \
            else                                                        \
                *tdst++  =  0;                                          \
        }                                                               \
		*tdst++   =   gamma_##psize( rd );                              \
		*tdst++   =   gamma_##psize( gd );                              \
		*tdst     =   gamma_##psize( bd );                              \
    }                                                                   \
    else if (color < 4)                                                 \
    {                                                                   \
        color-=1;                                                       \
        for(k=0; k<ndim; k++)                                           \
        {                                                               \
            r = ((unsigned psize**)rgb)[k] + SamplesPerPixel - 3 + color; \
            yr[k] =  0.0;                                               \
                                                                        \
            for(i=0; i<ndim; i++)                                       \
            {                                                           \
                yr[k] += degamma_##psize((int)r[i*SamplesPerPixel]) * w[i]; \
            }                                                           \
        }                                                               \
                                                                        \
        intpol( Dy, w, ndim )                                           \
        rd = 0.0;                                                       \
                                                                        \
        for(i=0; i<ndim; i++)                                           \
        {                                                               \
            rd += yr[i] * w[ i ];                                       \
        }                                                               \
        tdst = (unsigned psize *)dst;                                   \
        if(SamplesPerPixel==4)                                          \
            *tdst++  =  maxalpha;                                       \
                                                                        \
        *(tdst+color)  =    gamma_##psize( rd );                        \
    }                                                                   \
    else                                                                \
    {                                                                   \
        for(k=0; k<ndim; k++)                                           \
        {                                                               \
            r = ((unsigned psize**)rgb)[k]  + SamplesPerPixel - 3;      \
            rd = gd = bd = 0.0;                                         \
                                                                        \
            for(i=0; i<ndim; i++)                                       \
            {                                                           \
                weight = w[ i ];                                        \
                ri     = r + i * SamplesPerPixel;                       \
                rd += degamma_##psize((int)*ri++) * weight;                 \
                gd += degamma_##psize((int)*ri++) * weight;                 \
                bd += degamma_##psize((int)*ri)   * weight;                 \
            }                                                           \
            yr[k] = rd; yg[k] = gd; yb[k] = bd;                         \
        }                                                               \
                                                                        \
        intpol( Dy, w, ndim )                                           \
        rd = gd = bd = 0.0;                                             \
                                                                        \
        for(i=0; i<ndim; i++)                                           \
        {                                                               \
            weight = w[ i ];                                            \
            rd += yr[i] * weight;                                       \
            gd += yg[i] * weight;                                       \
            bd += yb[i] * weight;                                       \
        }                                                               \
                                                                        \
        tdst = (unsigned psize *)dst;                                   \
        if(SamplesPerPixel==4)                                          \
            *tdst++  =  maxalpha;                                       \
                                                                        \
        if (color==4) /* Red+Grn */                                     \
        {                                                               \
            *tdst++   =   gamma_##psize( rd );                          \
            *tdst     =   gamma_##psize( gd );                          \
            /*                              blue untouched */           \
        }                                                               \
        else                                                            \
        if (color==5) /* Red+Blue */                                    \
        {                                                               \
            *tdst++   =   gamma_##psize( rd );                          \
             tdst++;  /* green untouched */                             \
            *tdst     =   gamma_##psize( bd );                          \
        }                                                               \
        else /* (color=6) Green+Blue */                                 \
        {                                                               \
             tdst++;  /* red untouched */                               \
            *tdst++   =   gamma_##psize( gd );                          \
            *tdst     =   gamma_##psize( bd );                          \
        }                                                               \
                                                                        \
    }                                                                   \
;


static double sinc( double x )
{
	x *= PI;
	if(x != 0.0) 
		return(sin(x) / x);
	return(1.0);
}


// Cubic polynomial with parameter A
// A = -1: sharpen; A = - 0.5 homogeneous
// make sure x >= 0
#define	A	(-0.75)

// 0 <= x < 1
static double cubic01( double x )
{
	return	(( A + 2.0 )*x - ( A + 3.0 ))*x*x +1.0;
}
// 1 <= x < 2

static double cubic12( double x )
{
	return	(( A * x - 5.0 * A ) * x + 8.0 * A ) * x - 4.0 * A;

}

#undef A





// ---------- Sampling functions ----------------------------------

#define maxalpha  255
#define threshold (maxalpha / 16)

// Nearest neighbor sampling, nowhere used (yet)

static void nn( unsigned char *dst, unsigned char **rgb, 
		register double Dx PT_UNUSED, register double Dy PT_UNUSED,	
		int color, int SamplesPerPixel)
		{
			RESAMPLE_N( NNEIGHBOR, 1, char)	}

// Bilinear sampling, nowhere used (yet).

static void bil( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( BILINEAR, 2, char) 	}


// Lowest quality sampler in distribution; since version 1.8b1 changed to closely
// resemble Photoshop's bicubic interpolation

static void poly3( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( CUBIC, 4, char) 	}

// Spline using 16 pixels; smoother and less artefacts than poly3, softer; same speed

static void spline16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SPLINE16, 4, char) 	}

// Spline using 36 pixels; significantly sharper than both poly3 and spline16,
// almost no artefacts

static void spline36( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SPLINE36, 6, char) 	}

// Not used anymore

static void spline64( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SPLINE64, 8, char) 	}


// Highest quality sampler since version 1.8b1
// Extremely slow, but defintely worth every second.

static void sinc256( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SINC, 16, char) 	}
		

// Highest quality sampler since version 1.8b1
// Extremely slow, but defintely worth every second.

static void sinc1024( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SINC, 32, char) 	}


//--------------- Same as above, for shorts (16 bit channel size-------------------

#undef maxalpha
#define maxalpha  65535

// Nearest neighbor sampling, nowhere used (yet)

static void nn_16( unsigned char *dst, unsigned char **rgb, 
		register double Dx PT_UNUSED, register double Dy PT_UNUSED,
		int color, int SamplesPerPixel)
		{
			RESAMPLE_N( NNEIGHBOR, 1, short)	}

// Bilinear sampling, nowhere used (yet).

static void bil_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( BILINEAR, 2, short) 	}


// Lowest quality sampler in distribution; since version 1.8b1 changed to closely
// resemble Photoshop's bicubic interpolation

static void poly3_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( CUBIC, 4, short) 	}


// Spline using 16 pixels; smoother and less artefacts than poly3, softer; same speed

static void spline16_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SPLINE16, 4, short) 	}

// Spline using 36 pixels; significantly sharper than both poly3 and spline16,
// almost no artefacts

static void spline36_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SPLINE36, 6, short) 	}

// Not used anymore

static void spline64_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SPLINE64, 8, short) 	}


// Highest quality sampler since version 1.8b1
// Extremely slow, but defintely worth every second.

static void sinc256_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SINC, 16, short) 	}
		

// Highest quality sampler since version 1.8b1
// Extremely slow, but defintely worth every second.

static void sinc1024_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SINC, 32, short) 	}
		
//--------------- Same as above, for float -------------------

/*

A note about the use of undef signed below.

	When RESAMPLE_N uses its third parameter sometimes it prefixes
	it with unsigned. But floats can be unsigned. We previously
	hacked it by removing "unsigned" via the preprocessor. We
	originally wrapped all the functiosn that used RESAMPLE_N with
	float with a single undef unsigned. Unfortunately this also
	wrapped other (correct) uses of unsigned. What I did in this
	change is wrap only the invocation to RESAMPLE_N. This should
	improve readability and maintanability.
	
	Another alternative (suggested by Walter Harms) is to do something
	like this:

	#define MACRO( psize , sign ) \
	sign psize *r;    \
	call##psize ( r );

	MACRO(float, )
	MACRO(int, unsigned )

	This in fact works, but it requires far more changed lines than
	the current fix. Furthermore RESAMPLE_N  is one of those macros
	that nobody (ok, I) wants to debug if something goes wrong.
*/


#undef maxalpha
#define maxalpha  1.0




// Nearest neighbor sampling, nowhere used (yet)

static void nn_32( unsigned char *dst, unsigned char **rgb, 
		register double Dx PT_UNUSED, register double Dy PT_UNUSED,
		int color, int SamplesPerPixel)
{
#define unsigned
  RESAMPLE_N( NNEIGHBOR, 1, float);
#undef unsigned
}

// Bilinear sampling, nowhere used (yet).

static void bil_32( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
{	
#define unsigned
  RESAMPLE_N( BILINEAR, 2, float);
#undef unsigned
}


// Lowest quality sampler in distribution; since version 1.8b1 changed to closely
// resemble Photoshop's bicubic interpolation

static void poly3_32( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
{
#define unsigned  
  RESAMPLE_N( CUBIC, 4, float);
#undef unsigned
}

// Spline using 16 pixels; smoother and less artefacts than poly3, softer; same speed

static void spline16_32( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
{
#define unsigned
  RESAMPLE_N( SPLINE16, 4, float) ;
#undef unsigned
}

// Spline using 36 pixels; significantly sharper than both poly3 and spline16,
// almost no artefacts

static void spline36_32( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
{
#define unsigned
  RESAMPLE_N( SPLINE36, 6, float) ;
#undef unsigned
}

// Not used anymore

static void spline64_32( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
{
#define unsigned
  RESAMPLE_N( SPLINE64, 8, float) ;
#undef unsigned
}


// Highest quality sampler since version 1.8b1
// Extremely slow, but defintely worth every second.

static void sinc256_32( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
{
#define unsigned
  RESAMPLE_N( SINC, 16, float) ;
#undef unsigned
}
		

// Highest quality sampler since version 1.8b1
// Extremely slow, but defintely worth every second.

static void sinc1024_32( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
{
#define unsigned
  RESAMPLE_N( SINC, 32, float) ;
#undef unsigned
}
		
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FS+ start of functions used to compute the pixel tranform from dest to source using linear interpolation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// computes the source coordinates of a single pixel at position x using the math transforms
void ComputePixelCoords( double *ax, double *ay, int *trinum, char *avalid, uint32_t x, long offset, double w2, double y_d, 
						  fDesc *fD, double sw2, double sh2, double min_x, double max_x, double min_y, double max_y ) {
	double x_d, Dx, Dy;
        int tvalid;

	// Convert destination screen coordinates to cartesian coordinates.			
	// Offset is the distance between the left edge of the ROI and the left edge of the full output canvas (always less than or equal to 0)
	x_d = (double) (x - offset) - w2;

	// Get source cartesian coordinates 
	tvalid = fD->func( x_d, y_d , &Dx, &Dy, fD->param);

	// Convert source cartesian coordinates to screen coordinates 
	Dx += sw2;
	Dy = sh2 + Dy;

	// stores the computed pixel
	ax[x] = Dx;
	ay[x] = Dy;
	trinum[x] = getLastCurTriangle();

	// Is the pixel valid, i.e. from within source image?
	if( (Dx >= max_x)   || (Dy >= max_y) || (Dx < min_x) || (Dy < min_y)  || (tvalid==0))
		avalid[x] = FALSE;
	else
		avalid[x] = TRUE;
}

// fills a part of the arrays with the coordinates in the source image for every pixel
// xl is the left border of the array, xr is the right border. The array values have already been
//   computed in xl and xr.
void ComputePartialRowCoords( double *ax, double *ay, int *trinum, char *avalid, uint32_t xl, uint32_t xr, long offset, double w2, double y_d, 
						  fDesc *fD, double sw2, double sh2, double min_x, double max_x, double min_y, double max_y ) {
	uint32_t xm, idx;
	double srcX_lin, srcY_lin;
	double deltaX, deltaY, tmpX, tmpY;

	////////////////////////////////////////////
	// maximum estimated error to be accepted: higher values produce a faster execution but a more distorted image
	// the real maximum error seems to be much lower, about 1/4 of MAX_ERR
	double MAX_ERR = 1;

	if( xl >= (xr - 1) ) return;

	if( !avalid[xl] && !avalid[xr] ) {
		// first and last pixel are not valid, assume that others are not valid too
		// ax[] and ay[] values are not set since thay will not be used
		for( idx = xl + 1; idx < xr; idx++ ) {
			avalid[idx] = FALSE;
		}
		return;
	}

	// computes the source coords of the middle point of [xl, xr] using the transformation
	xm = (xl + xr)/2;
	ComputePixelCoords( ax, ay, trinum, avalid, xm, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
	// computes the coords of the same point with linear interpolation
	srcX_lin = ax[xl] + ((ax[xr] - ax[xl])/(xr - xl))*(xm - xl);
	srcY_lin = ay[xl] + ((ay[xr] - ay[xl])/(xr - xl))*(xm - xl);

	if( fabs(srcX_lin - ax[xm]) > MAX_ERR || fabs(srcY_lin - ay[xm]) > MAX_ERR ||
	    trinum[xl] != trinum[xr] || trinum[xl] != trinum[xm]) {
		// the error is still too large or the points are in different morph triangles: recursion
		ComputePartialRowCoords( ax, ay, trinum, avalid, xl, xm, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
		ComputePartialRowCoords( ax, ay, trinum, avalid, xm, xr, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
		return;
	}

	// fills the array, first the left half...
	if( !avalid[xl] || !avalid[xm] ) {
		// one end is valid and the other is not: computes every pixel with math transform
		for( idx = xl + 1; idx < xm; idx++ ) {
			ComputePixelCoords( ax, ay, trinum, avalid, idx, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
		}
	}
	else {
		// linear interpolation	
		deltaX = (ax[xm] - ax[xl]) / (xm - xl);
		deltaY = (ay[xm] - ay[xl]) / (xm - xl);
		tmpX = ax[xl];
		tmpY = ay[xl];
		for( idx = xl + 1; idx < xm; idx++ ) {
			tmpX += deltaX;
			tmpY += deltaY;
			ax[idx] = tmpX;
			ay[idx] = tmpY;
			if( (tmpX >= max_x)   || (tmpY >= max_y) || (tmpX < min_x) || (tmpY < min_y)  )
				avalid[idx] = FALSE;
			else
				avalid[idx] = TRUE;
			trinum[idx] = trinum[xl];
		}
	}

	// ...then the right half
	if( !avalid[xm] || !avalid[xr] ) {
		// one end is valid and the other is not: computes every pixel with math transform
		for( idx = xm + 1; idx < xr; idx++ ) {
			ComputePixelCoords( ax, ay, trinum, avalid, idx, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
		}
	}
	else {
		// linear interpolation	
		deltaX = (ax[xr] - ax[xm]) / (xr - xm);
		deltaY = (ay[xr] - ay[xm]) / (xr - xm);
		tmpX = ax[xm];
		tmpY = ay[xm];
		for( idx = xm + 1; idx < xr; idx++ ) {
			tmpX += deltaX;
			tmpY += deltaY;
			ax[idx] = tmpX;
			ay[idx] = tmpY;
			if( (tmpX >= max_x)   || (tmpY >= max_y) || (tmpX < min_x) || (tmpY < min_y)  )
				avalid[idx] = FALSE;
			else
				avalid[idx] = TRUE;
			trinum[idx] = trinum[xr];
		}
	}

}


// fills the arrays with the source coords computed using linear interpolation
// asize is the number of elements of the arrays
// the array elements lie in the interval [0, asize], the image elements in [destRect.left, destRect.right]: the offset parameter
//   is used for the conversion
void ComputeRowCoords( double *ax, double *ay, int *trinum, char *avalid, int32_t asize, long offset, double w2, double y_d, 
						  fDesc *fD, double sw2, double sh2, double min_x, double max_x, double min_y, double max_y, int STEP_WIDTH) {

	// STEP_WIDTH is initial distance betwen correctly computed points. The distance will be reduced if needed.

	uint32_t x;

	x = 0;
	ComputePixelCoords( ax, ay, trinum, avalid, x, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
	x += STEP_WIDTH;
	while( x < asize ) {
		ComputePixelCoords( ax, ay, trinum, avalid, x, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
		ComputePartialRowCoords( ax, ay, trinum, avalid, x - STEP_WIDTH, x, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
		x += STEP_WIDTH;
	}
	// compute the last pixels, if any
	x -= STEP_WIDTH;
	if( x < asize - 1 ) {
		ComputePixelCoords( ax, ay, trinum, avalid, asize - 1, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
		ComputePartialRowCoords( ax, ay, trinum, avalid, x, asize - 1, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FS- end of functions used to compute the pixel transform from dest to source using linear interpolation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//    Main transformation function. Destination image is calculated using transformation
//    Function "func". Either all colors (color = 0) or one of rgb (color =1,2,3) are
//    determined. If successful, TrPtr->success = 1. Memory for destination image
//    must have been allocated and locked!
//
//    MODIFICATIONS:
//      June 2004 - R.Platt - moved body of transForm into MyTransForm and MyTransformBody to eliminate code duplication.
//                          - This was also needed for multithreading.

void transForm( TrformStr *TrPtr, fDesc *fD, int color)
{
    int imageNum = 1;
    MyTransForm( TrPtr, fD, color, imageNum );
}

void transFormEx( TrformStr *TrPtr, fDesc *fD, fDesc *finvD, int color, int imageNum )
{
	if (TrPtr->interpolator<_aabox)
	{
		MyTransForm(TrPtr, fD, color, imageNum);
	} 
	else
	{
		 transForm_aa(TrPtr, fD, finvD, color, imageNum);
	}
}


/*This function was added by Kekus Digital on 18/9/2002. 
This function takes the parameter 'imageNum' which repesents the 
index of the image that has to be converted.*/
void MyTransForm( TrformStr *TrPtr, fDesc *fD, int color, int imageNum)
{
	register uint32_t 		x, y;		// Loop through destination image
	register uint32_t     	i, k; 	 	// Auxilliary loop variables
	int 			skip = 0;	// Update progress counter
	unsigned char 		*dest,*src,*sry;// Source and destination image data
	register unsigned char 		*sr;	// Source  image data
	char			progressMessage[30];// Message to be displayed by progress reporter
	char                	percent[8];	// Number displayed by Progress reporter
	int			valid;		// Is this pixel valid? (i.e. inside source image)
        int                     tvalid;         // temp variable, holds if the transform for this pixel was defined
	long			coeff;		// pixel coefficient in destination image
	long			cy;		// rownum in destimage
	int			xc,yc;

	double 			x_d, y_d;	// Cartesian Coordinates of point ("target") in Destination image
	double 		  	Dx, Dy;		// Coordinates of target in Source image
	int 			xs, ys;	

	unsigned char		**rgb  = NULL, 
				*cdata = NULL;	// Image data handed to sampler

	double			max_x = (double) TrPtr->src->width; // Maximum x values in source image
	double			max_y = (double) TrPtr->src->height; // Maximum y values in source image
	double			min_x =  -1.0;//0.0; // Minimum x values in source image
	double			min_y =  -1.0;//0.0; // Minimum y values in source image

	int			mix	  = TrPtr->src->width - 1; // maximum x-index src
	int			mix2;
	int			miy	  = TrPtr->src->height - 1;// maximum y-index src
	int			miy2;

	// Variables used to convert screen coordinates to cartesian coordinates

		
	double 			w2 	= (double) TrPtr->dest->width  / 2.0 - 0.5;  // Steve's L
	double 			h2 	= (double) TrPtr->dest->height / 2.0 - 0.5;
	double 			sw2 = (double) TrPtr->src->width   / 2.0 - 0.5;
	double 			sh2 = (double) TrPtr->src->height  / 2.0 - 0.5;
	
	int			BytesPerLine	= TrPtr->src->bytesPerLine;
	int			FirstColorByte, SamplesPerPixel;
	unsigned int	BytesPerPixel, BytesPerSample;

	int			n, n2;		// How many pixels should be used for interpolation	
	intFunc 		intp; 		// Function used to interpolate
	// int 			lu = 0;		// Use lookup table?
	int			wrap_x = FALSE;
	double			theGamma;	// gamma handed to SetUpGamma()

	//////////////////////////////////////////////////////////////////////////
	// FS+ variables used for linear interpolation of the pixel transform
	double *ax = NULL, *ay = NULL;	// source coordinates of each pixel in a row
	int *trinum = NULL;             // triangle number if morphing
	char *avalid = NULL;			// is the pixel valid?
	double maxErrX, maxErrY;
	long offset;
    int FastTransform = TrPtr->fastStep; // non 0 if we will use the new fast pixel transformation
	int evaluateError = FALSE;		// true if we want to write a file with the transformation errors
	// FS-
	//////////////////////////////////////////////////////////////////////////

	// Selection rectangle
	PTRect			destRect;
	if( TrPtr->dest->selection.bottom == 0 && TrPtr->dest->selection.right == 0 ){
		destRect.left 	= 0;
		destRect.right	= TrPtr->dest->width;
		destRect.top	= 0;
		destRect.bottom = TrPtr->dest->height;
	}else{
		memcpy( &destRect, &TrPtr->dest->selection, sizeof(PTRect) );
	}

	// FS+
	offset = -destRect.left;
	maxErrX = 0;
	maxErrY = 0;
	// FS-

	switch( TrPtr->src->bitsPerPixel ){
		case 128:	FirstColorByte = 4; BytesPerPixel = 16; SamplesPerPixel = 4; BytesPerSample = 4; break;
		case  96:	FirstColorByte = 0; BytesPerPixel = 12; SamplesPerPixel = 3; BytesPerSample = 4; break;
		case 64:	FirstColorByte = 2; BytesPerPixel = 8; SamplesPerPixel = 4; BytesPerSample = 2; break;
		case 48:	FirstColorByte = 0; BytesPerPixel = 6; SamplesPerPixel = 3; BytesPerSample = 2; break;
		case 32:	FirstColorByte = 1; BytesPerPixel = 4; SamplesPerPixel = 4; BytesPerSample = 1; break;
		case 24:	FirstColorByte = 0; BytesPerPixel = 3; SamplesPerPixel = 3; BytesPerSample = 1; break;
		case  8:	FirstColorByte = 0; BytesPerPixel = 1; SamplesPerPixel = 1; BytesPerSample = 1; break;
		default:	PrintError("Unsupported Pixel Size: %d", TrPtr->src->bitsPerPixel);
					TrPtr->success = 0;
					return;
	}
	
	// Set interpolator etc:
	switch( TrPtr->interpolator ){
		case _poly3:// Third order polynomial fitting 16 nearest pixels
			if( BytesPerSample == 1 ) intp = poly3; 
			if( BytesPerSample == 2 ) intp = poly3_16;		
			if( BytesPerSample == 4 ) intp = poly3_32;
			n = 4;
			break;
		case _spline16:// Cubic Spline fitting 16 nearest pixels
			if( BytesPerSample == 1 ) intp = spline16;
			if( BytesPerSample == 2 ) intp = spline16_16;
			if( BytesPerSample == 4 ) intp = spline16_32;
			n = 4;
			break;
		case _spline36:	// Cubic Spline fitting 36 nearest pixels
			if( BytesPerSample == 1 ) intp = spline36;
			if( BytesPerSample == 2 ) intp = spline36_16;
			if( BytesPerSample == 4 ) intp = spline36_32;
			n = 6;
			break;
		case _spline64:	// Cubic Spline fitting 64 nearest pixels
			if( BytesPerSample == 1 ) intp = spline64;
			if( BytesPerSample == 2 ) intp = spline64_16;
			if( BytesPerSample == 4 ) intp = spline64_32;
			n = 8;
			break;
		case _sinc256:	// sinc windowed to 256 (2*8)^2 pixels
			if( BytesPerSample == 1 ) intp = sinc256;
			if( BytesPerSample == 2 ) intp = sinc256_16;
			if( BytesPerSample == 4 ) intp = sinc256_32;
			n = 16;
			break;
		case _sinc1024:	// sinc windowed to 1024 (2*16)^2 pixels
			if( BytesPerSample == 1 ) intp = sinc1024;
			if( BytesPerSample == 2 ) intp = sinc1024_16;
			if( BytesPerSample == 4 ) intp = sinc1024_32;
			n = 32;
			break;
		case _bilinear:	// Bilinear fit using 4 nearest points
			if( BytesPerSample == 1 ) intp = bil;
			if( BytesPerSample == 2 ) intp = bil_16;
			if( BytesPerSample == 4 ) intp = bil_32;
			n = 2;
			break;
		case _nn:// nearest neighbor fit using 4 nearest points
			if( BytesPerSample == 1 ) intp = nn;
			if( BytesPerSample == 2 ) intp = nn_16;
			if( BytesPerSample == 4 ) intp = nn_32;
			n = 1;
			break;
		default: 
			PrintError( "Invalid Interpolator selected" );
			TrPtr->success = 0;
			return;
	}

	// Set up arrays that hold color data for interpolators

	rgb 	= (unsigned char**) malloc( n * sizeof(unsigned char*) );
	cdata	= (unsigned char*)  malloc( n * n * BytesPerPixel * sizeof( unsigned char ) );
	
	
	if( rgb == NULL || cdata == NULL ){
		PrintError( "Not enough Memory" );
		TrPtr->success = 0;
		goto Trform_exit;
	}
		
	n2 = n/2 ;
	mix2 = mix +1 - n;
	miy2 = miy +1 - n;

	dest = *TrPtr->dest->data;
	src  = *TrPtr->src->data; // is locked

    //MRDL: There seems to be a strange bug somewhere that
    //corrupts the first three bytes of each source image.
    //Rather than looking like this (ARGB ARGB ARGB ARGB):
    //   255 128 128 128   255 128 128 128   255 128 128 128   255 128 128 128
    //They look like this:
    //   255 255 255 255   255 128 255 128   255 128 128 255   255 128 128 128
    //So fix up the first few bytes to workaround this if file is large than 3 bytes
    if ((mix+1) * (miy+1) > 3)
    {
        memcpy( src                      , src + (BytesPerPixel * 3), BytesPerPixel);
        memcpy( src + (BytesPerPixel * 1), src + (BytesPerPixel * 3), BytesPerPixel);
        memcpy( src + (BytesPerPixel * 2), src + (BytesPerPixel * 3), BytesPerPixel);        
    }
    
	if(TrPtr->mode & _show_progress){
		switch(color){
			case 0:
                        { 
                            char title[30];
#if BROKEN
                            int the_Num;
                            NumToString(imageNum, the_Num);
                            p2cstr(the_Num);
                            strcpy(title, "Converting Image #");
                            strcat(title, (char *)the_Num);
#else
                            sprintf(title, "Converting Image #%d", imageNum);
#endif
                            strcpy(progressMessage, title);	
                            //progressMessage = "Image Conversion"; 	
                        }
                        break;
			case 1:	switch( TrPtr->src->dataformat){
						case _RGB: 	strcpy(progressMessage,"Red Channel " PROGRESS_VERSION); break;
						case _Lab:	strcpy(progressMessage, "Lightness"); break;
					} break;
			case 2:	switch( TrPtr->src->dataformat){
						case _RGB: 	strcpy(progressMessage, "Green Channel " PROGRESS_VERSION); break;
						case _Lab:	strcpy(progressMessage, "Color A"); break;
					} break; 
			case 3:	switch( TrPtr->src->dataformat){
						case _RGB: 	strcpy(progressMessage, "Blue Channel " PROGRESS_VERSION); break;
						case _Lab:	strcpy(progressMessage, "Color B"); break;
					} break; 
			case 4:	switch( TrPtr->src->dataformat)
            {
                case _RGB: 	strcpy(progressMessage,"Red/Grn Channels " PROGRESS_VERSION); break;
                case _Lab:	strcpy(progressMessage, "Unsupported!!"); break;
            } break;
                
			case 5:	switch( TrPtr->src->dataformat)
            {
                case _RGB: 	strcpy(progressMessage, "Red/Blue Channels " PROGRESS_VERSION); break;
                case _Lab:	strcpy(progressMessage, "Unsupported!!"); break;
            } break; 
                
			case 6:	switch( TrPtr->src->dataformat)
            {
                case _RGB: 	strcpy(progressMessage, "Grn/Blue Channels " PROGRESS_VERSION); break;
                case _Lab:	strcpy(progressMessage, "Unsupported!!"); break;
            } break; 

			default: strcpy(progressMessage, "Something is wrong here");
		}
		Progress( _initProgress, progressMessage );
	}

	if(TrPtr->mode & _wrapX)
		wrap_x = TRUE;

	if( TrPtr->src->dataformat == _RGB )	// Gamma correct only RGB-images
		theGamma = TrPtr->gamma;
	else
		theGamma = 1.0;
	
	if (BytesPerSample<=2) { // No Gammatable for float!
	if( SetUpGamma( theGamma, BytesPerSample) != 0 ){
		PrintError( "Could not set up lookup table for Gamma Correction" );
		TrPtr->success = 0;
		goto Trform_exit;
	}
	}

	// FS+ allocates the temporary arrays
	ax = (double *) malloc( (destRect.right - destRect.left + 20)*sizeof(double) );
	ay = (double *) malloc( (destRect.right - destRect.left + 20)*sizeof(double) );
	trinum = (int *) malloc( (destRect.right - destRect.left + 20)*sizeof(int) );
	avalid = (char *) malloc( (destRect.right - destRect.left + 20)*sizeof(char) );
	// opens the preference file to read options
	evaluateError = FALSE;
	{
		FILE *fp;
		char buf[100];
		char *s;
		s = buf;
		fp = fopen( "pano12_opt.txt", "rt" );
		if( fp != NULL ) {
			// parse the file
			s = fgets( s, 98, fp );
			while( !feof(fp) && buf != NULL ) {
				//s = strupr( buf );	commented out because it causes linking problems with the microsoft compiler
				if( strncmp( s, "FAST_TRANSFORM", 14 )  == 0 )
					FastTransform = FAST_TRANSFORM_STEP_NORMAL;
				if( strncmp( s, "MORPH_TRANSFORM", 15 )  == 0 )
					FastTransform = FAST_TRANSFORM_STEP_MORPH;
				if( strncmp( s, "EVALUATE_ERROR", 14 )  == 0 )
					evaluateError = TRUE;
				s = fgets( buf, 98, fp );
			}
			fclose( fp );
		}
		if( FastTransform == 0 ) evaluateError = FALSE;	// only evaluate error if fast transform is activated
	}
	// FS-

	for(y=destRect.top; y<destRect.bottom; y++){
		// Update Progress report and check for cancel every 2%.
		skip++;
		if( skip == (int)ceil(TrPtr->dest->height/50.0) ){
			if(TrPtr->mode & _show_progress){	
				sprintf( percent, "%d", (int) ((y * 100)/ TrPtr->dest->height));
				if( ! Progress( _setProgress, percent ) ){
					TrPtr->success = 0;
					goto Trform_exit;
				}
			}else{
				if( ! Progress( _idleProgress, 0) ){
					TrPtr->success = 0;
					goto Trform_exit;
				}
			}
			skip = 0;
		}
		
		// y-coordinate in dest image relative to center		
		y_d = (double) y - h2 ;
		cy  = (y-destRect.top) * TrPtr->dest->bytesPerLine;	
		
		// FS+ computes the transform for this row using linear interpolation
		if( FastTransform != 0 || evaluateError )
			ComputeRowCoords( ax, ay, trinum, avalid, destRect.right - destRect.left + 1, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y, FastTransform);
		// FS-

		for(x=destRect.left; x<destRect.right; x++){
			// Calculate pixel coefficient in dest image just once

			coeff = cy  + BytesPerPixel * (x-destRect.left);		

			// FS+
			if( FastTransform == 0 || evaluateError ) {
				// Convert destination screen coordinates to cartesian coordinates.			
				x_d = (double) x - w2 ;
				
				// Get source cartesian coordinates 
				tvalid = fD->func( x_d, y_d , &Dx, &Dy, fD->param);
	
				// Convert source cartesian coordinates to screen coordinates 
				Dx += sw2;
				Dy =  sh2 + Dy ;
				
				if( evaluateError ) {
					valid = avalid[x];
				}
				else {
					// Is the pixel valid, i.e. from within source image?
					if( (Dx >= max_x)   || (Dy >= max_y) || (Dx < min_x) || (Dy < min_y) || (tvalid==0) )
						valid = FALSE;
					else
						valid = TRUE;
				}
			} else {
				//Do "fast transform" by looking up coordinates from pre-populated arrays
				//NB: "fast transform" arrays are as large as the ROI...
				Dx = ax[x-destRect.left];
				Dy = ay[x-destRect.left];
				valid = avalid[x-destRect.left];
			}
			// was:

			//// Convert destination screen coordinates to cartesian coordinates.			
			//x_d = (double) x - w2 ;
			//
			//// Get source cartesian coordinates 
			//fD->func( x_d, y_d , &Dx, &Dy, fD->param);

			//// Convert source cartesian coordinates to screen coordinates 
			//Dx += sw2;
			//Dy =  sh2 + Dy ;
			//

			//// Is the pixel valid, i.e. from within source image?
			//if( (Dx >= max_x)   || (Dy >= max_y) || (Dx < min_x) || (Dy < min_y)  )
			//	valid = FALSE;
			//else
			//	valid = TRUE;

			// FS-

			// Convert only valid pixels
			if( valid ){

				// FS+
				if( evaluateError ) {
					double errX, errY;
					errX = fabs( Dx - ax[x + offset] );
					errY = fabs( Dy - ay[x + offset] );
					if( errX > maxErrX )
						maxErrX = errX;
					if( errY > maxErrY )
						maxErrY = errY;
				}
				// FS-

				// Extract integer and fractions of source screen coordinates
				xc 	  =  (int)floor( Dx ) ; Dx -= (double)xc;
				yc 	  =  (int)floor( Dy ) ; Dy -= (double)yc;
				
				// if alpha channel marks valid portions, set valid 
				if(TrPtr->mode & _honor_valid)
				switch( FirstColorByte ){
					case 1:{
						int xn = xc, yn = yc;
						if( xn < 0 ) xn = 0; //  -1  crashes Windows
						if( yn < 0 ) yn = 0; //  -1  crashes Windows
						if( src[ yn * BytesPerLine + BytesPerPixel * xn] < 128 )
							valid = FALSE;
						}
						break;
					case 2:{
						int xn = xc, yn = yc;
						if( xn < 0 ) xn = 0; //  -1  crashes Windows
						if( yn < 0 ) yn = 0; //  -1  crashes Windows
						if( *((uint16_t*)(src + yn * BytesPerLine + BytesPerPixel * xn)) < 32768 )
							valid = FALSE;
						}
						break;
					default: break;
				}
			}
			
			if( valid ){	
				ys = yc +1 - n2 ; // smallest y-index used for interpolation
				xs = xc +1 - n2 ; // smallest x-index used for interpolation
					
				// y indices used: yc-(n2-1)....yc+n2
				// x indices used: xc-(n2-1)....xc+n2
					
				if( ys >= 0 && ys <= miy2 && xs >= 0 && xs <= mix2 ){  // all interpolation pixels inside image
					sry = src + ys * BytesPerLine + xs * BytesPerPixel;
					for(i = 0;  i < n;  i++, sry += BytesPerLine){
						rgb[i] = sry;
					}
				}else{ // edge pixels
					if( ys < 0 )
						sry = src;
					else if( ys > miy )
						sry = src + miy * BytesPerLine;
					else
						sry = src + ys  * BytesPerLine;
					
					for(i = 0; i < n; i++){	
						xs = xc +1 - n2 ; // smallest x-index used for interpolation
						if( wrap_x ){
							while( xs < 0 )  xs += (mix+1);
							while( xs > mix) xs -= (mix+1);
						}
						if( xs < 0 )
							 sr = sry;
						else if( xs > mix )
							sr = sry + mix *BytesPerPixel;
						else
							sr = sry + BytesPerPixel * xs;
					
						rgb[i] = cdata + i * n * BytesPerPixel;
						for(k = 0; k < n; k++ ){
							memcpy( &(rgb[i][k * BytesPerPixel]), sr, (size_t)BytesPerPixel);
							xs++;
							if( wrap_x ){
								while( xs < 0 )  xs += (mix+1);
								while( xs > mix) xs -= (mix+1);
							}
							if( xs < 0 )
							 	sr = sry;
							else if( xs > mix )
								sr = sry + mix *BytesPerPixel;
							else
								sr = sry + BytesPerPixel * xs;
						}
						 ys++;
						 if( ys > 0 && ys <= miy )
						 	sry +=  BytesPerLine; 
					}

				}
				
						
				intp( &(dest[ coeff ]), rgb, Dx, Dy, color, SamplesPerPixel ); 

                }// END: if is a valid pixel
                else
                {  
                    // not valid (source pixel x,y not inside source image, etc.)
                    
                    //Fix: Correct would use incorrect correction values if different factors were set for each color channel
                    //PT.Fix.mt.Begin: March.2004
                    //was:
                    //memset( &(dest[ coeff ]), 0 ,BytesPerPixel );
                    //now:
                    if(color==0) // RGB same time
                    {
                        memset( &(dest[ coeff ]), 0 ,BytesPerPixel ); //mt_test
                                                                      //PT.Dev.mt.End: March.2004( &(dest[ coeff ]), 0 ,BytesPerPixel ); 
                    }
                    else
                    {
                        char*   ptr = &(dest[ coeff ]);
                    
                        if(color < 4) // R or G or B
                        {
                        ptr += FirstColorByte + (color - 1)*BytesPerSample;
                        memset( ptr, 0 , BytesPerSample ); //mt_test
                    }	
                    else
                        if(color==4) // R+G
                        {
                            ptr += FirstColorByte;
                            memset( ptr, 0 , 2*BytesPerSample ); //rjp
                        }
                        else
                        if(color==5) // R+B
                        {
                            ptr += FirstColorByte;
                            memset( ptr, 0 , BytesPerSample ); 
                            ptr += 2*BytesPerSample;
                            memset( ptr, 0 , BytesPerSample ); //rjp
                        }
                        else // (color==6) G+B
                        {
                            ptr += FirstColorByte + BytesPerSample;
                            memset( ptr, 0 , 2*BytesPerSample ); //rjp
                        }
                    }
                    
                }// END: else Not a valid pixel

		}
	}

//	if(TrPtr->mode & _show_progress){
//		Progress( _disposeProgress, percent );
//	}	
	TrPtr->success = 1;


Trform_exit:
	if( rgb ) 		free( rgb );
	if( cdata ) 		free( cdata );
	if( glu.DeGamma )	free( glu.DeGamma ); 	glu.DeGamma 	= NULL;
	if( glu.Gamma )		free( glu.Gamma );	glu.Gamma 	= NULL;

	// FS+
	if( ax != NULL ) free( ax );
	if( ay != NULL ) free( ay );
	if( trinum != NULL ) free( trinum);
	if( avalid != NULL ) free( avalid );

	if( evaluateError ) {
		FILE *fp;
		fp = fopen( "Errors.txt", "a+t" );
		fprintf( fp, "%f  "FMT_INT32"\n", maxErrX, destRect.top );
		fprintf( fp, "%f\n", maxErrY );
		fclose( fp );
	}
	// FS-

	return;
}


// The following Filter functions for anti aliasing filters are inspired by the 
// Graphick Magick resize function. http://www.graphicsmagick.org

static double J1(double x)
{
  double
    p,
    q;

  register long
    i;

  static const double
    Pone[] =
    {
       0.581199354001606143928050809e+21,
      -0.6672106568924916298020941484e+20,
       0.2316433580634002297931815435e+19,
      -0.3588817569910106050743641413e+17,
       0.2908795263834775409737601689e+15,
      -0.1322983480332126453125473247e+13,
       0.3413234182301700539091292655e+10,
      -0.4695753530642995859767162166e+7,
       0.270112271089232341485679099e+4
    },
    Qone[] =
    {
      0.11623987080032122878585294e+22,
      0.1185770712190320999837113348e+20,
      0.6092061398917521746105196863e+17,
      0.2081661221307607351240184229e+15,
      0.5243710262167649715406728642e+12,
      0.1013863514358673989967045588e+10,
      0.1501793594998585505921097578e+7,
      0.1606931573481487801970916749e+4,
      0.1e+1
    };

  p=Pone[8];
  q=Qone[8];
  for (i=7; i >= 0; i--)
  {
    p=p*x*x+Pone[i];
    q=q*x*x+Qone[i];
  }
  return(p/q);
}

static double P1(double x)
{
  double
    p,
    q;

  register long
    i;

  static const double
    Pone[] =
    {
      0.352246649133679798341724373e+5,
      0.62758845247161281269005675e+5,
      0.313539631109159574238669888e+5,
      0.49854832060594338434500455e+4,
      0.2111529182853962382105718e+3,
      0.12571716929145341558495e+1
    },
    Qone[] =
    {
      0.352246649133679798068390431e+5,
      0.626943469593560511888833731e+5,
      0.312404063819041039923015703e+5,
      0.4930396490181088979386097e+4,
      0.2030775189134759322293574e+3,
      0.1e+1
    };

  p=Pone[5];
  q=Qone[5];
  for (i=4; i >= 0; i--)
  {
    p=p*(8.0/x)*(8.0/x)+Pone[i];
    q=q*(8.0/x)*(8.0/x)+Qone[i];
  }
  return(p/q);
}

static double Q1(double x)
{
  double
    p,
    q;

  register long
    i;

  static const double
    Pone[] =
    {
      0.3511751914303552822533318e+3,
      0.7210391804904475039280863e+3,
      0.4259873011654442389886993e+3,
      0.831898957673850827325226e+2,
      0.45681716295512267064405e+1,
      0.3532840052740123642735e-1
    },
    Qone[] =
    {
      0.74917374171809127714519505e+4,
      0.154141773392650970499848051e+5,
      0.91522317015169922705904727e+4,
      0.18111867005523513506724158e+4,
      0.1038187585462133728776636e+3,
      0.1e+1
    };

  p=Pone[5];
  q=Qone[5];
  for (i=4; i >= 0; i--)
  {
    p=p*(8.0/x)*(8.0/x)+Pone[i];
    q=q*(8.0/x)*(8.0/x)+Qone[i];
  }
  return(p/q);
}

static double BesselOrderOne(double x)
{
  double
    p,
    q;

  if (x == 0.0)
    return(0.0);
  p=x;
  if (x < 0.0)
    x=(-x);
  if (x < 8.0)
    return(p*J1(x));
  q=sqrt(2.0/(PI*x))*(P1(x)*(1.0/sqrt(2.0)*(sin(x)-cos(x)))-8.0/x*Q1(x)*
    (-1.0/sqrt(2.0)*(sin(x)+cos(x))));
  if (p < 0.0)
    q=(-q);
  return(q);
}

static double Bessel(const double x,const double support)
{
  if (x == 0.0)
    return(PI/4.0);
  return(BesselOrderOne(PI*x)/(2.0*x));
}

static double Sinc(const double x,const double support)
{
  if (x == 0.0)
    return(1.0);
  return(sin(PI*x)/(PI*x));
}

static double Blackman(const double x,const double support)
{
  if (x < -1.0) return(0.0);
  if (x > 1.0) return(0.0);
  return(0.42+0.5*cos(PI*x)+0.08*cos(2*PI*x));
}

static double BlackmanBessel(const double x,const double support)
{
  return(Blackman(x/support,support)*Bessel(x,support));
}

static double BlackmanSinc(const double x,const double support)
{
  return(Blackman(x/support,support)*Sinc(x,support));
}

static double Box(const double x,const double support)
{
  if (x < -0.5)
    return(0.0);
  if (x < 0.5)
    return(1.0);
  return(0.0);
}

static double Catrom(const double x,const double support)
{
  if (x < -2.0)
    return(0.0);
  if (x < -1.0)
    return(0.5*(4.0+x*(8.0+x*(5.0+x))));
  if (x < 0.0)
    return(0.5*(2.0+x*x*(-5.0-3.0*x)));
  if (x < 1.0)
    return(0.5*(2.0+x*x*(-5.0+3.0*x)));
  if (x < 2.0)
    return(0.5*(4.0+x*(-8.0+x*(5.0-x))));
  return(0.0);
}

static double Cubic(const double x,const double support)
{
  if (x < -2.0)
    return(0.0);
  if (x < -1.0)
    return((2.0+x)*(2.0+x)*(2.0+x)/6.0);
  if (x < 0.0)
    return((4.0+x*x*(-6.0-3.0*x))/6.0);
  if (x < 1.0)
    return((4.0+x*x*(-6.0+3.0*x))/6.0);
  if (x < 2.0)
    return((2.0-x)*(2.0-x)*(2.0-x)/6.0);
  return(0.0);
}

static double Gaussian(const double x,const double support)
{
  // Gaussian 1/sqrt(2)
  return(sqrt(2.0/PI) * exp(-2.0*x*x));
}

static double Gaussian_2(const double x,const double support)
{
  // Gaussian 1/2
//  double d=0.5;
//  return ( 1.0/(2.0*d*sqrt(2.0*PI)) * exp(-2.0*x*x/(2*d*d)) );
  return ( 1.0/sqrt(2.0*PI) * exp(-4.0*x*x) );
}

static double Hanning(const double x,const double support)
{
  if (fabs(x) > 1.0) return 0;
  return(0.5+0.5*cos(PI*x));
}

static double Hamming(const double x,const double support)
{
  if (fabs(x) > 1.0) return 0;
  return(0.54+0.46*cos(PI*x));
}

static double Hermite(const double x,const double support)
{
  if (x < -1.0)
    return(0.0);
  if (x < 0.0)
    return((2.0*(-x)-3.0)*(-x)*(-x)+1.0);
  if (x < 1.0)
    return((2.0*x-3.0)*x*x+1.0);
  return(0.0);
}

static double Lanczos(const double x,const double support)
{
  if (x < -3.0)
    return(0.0);
  if (x < 0.0)
    return(Sinc(-x,support)*Sinc(-x/3.0,support));
  if (x < 3.0)
    return(Sinc(x,support)*Sinc(x/3.0,support));
  return(0.0);
}

static double Mitchell(const double x,const double support)
{
#define B   (1.0/3.0)
#define C   (1.0/3.0)
#define P0  ((  6.0- 2.0*B       )/6.0)
#define P2  ((-18.0+12.0*B+ 6.0*C)/6.0)
#define P3  (( 12.0- 9.0*B- 6.0*C)/6.0)
#define Q0  ((       8.0*B+24.0*C)/6.0)
#define Q1  ((     -12.0*B-48.0*C)/6.0)
#define Q2  ((       6.0*B+30.0*C)/6.0)
#define Q3  ((     - 1.0*B- 6.0*C)/6.0)

  if (x < -2.0)
    return(0.0);
  if (x < -1.0)
    return(Q0-x*(Q1-x*(Q2-x*Q3)));
  if (x < 0.0)
    return(P0+x*x*(P2-x*P3));
  if (x < 1.0)
    return(P0+x*x*(P2+x*P3));
  if (x < 2.0)
    return(Q0+x*(Q1+x*(Q2+x*Q3)));
  return(0.0);
}

static double Quadratic(const double x,const double support)
{
  if (x < -1.5)
    return(0.0);
  if (x < -0.5)
    return(0.5*(x+1.5)*(x+1.5));
  if (x < 0.5)
    return(0.75-x*x);
  if (x < 1.5)
    return(0.5*(x-1.5)*(x-1.5));
  return(0.0);
}

static double Triangle(const double x,const double support)
{
  if (x < -1.0)
    return(0.0);
  if (x < 0.0)
    return(1.0+x);
  if (x < 1.0)
    return(1.0-x);
  return(0.0);
}

double Lanczos2(double x,double support) {
	if (fabs(x)>=2) return 0;
	return sinc(x) * sinc(x/2);
}

double Lanczos3(double x,double support) {
	if (fabs(x)>=3) return 0;
	return sinc(x) * sinc(x/3);
}


// Cache for the Invers transformation

struct invCacheItem {
		short dstX,dstY;
		double srcX,srcY;
	};
typedef struct invCacheItem invCacheItem;

// 2 Random selected Prime numbers for the Hashmap 
#define INV_CACHE_SIZE 299993
#define INV_CACHE_FY	25793
#define FF_STACK_SIZE	100000

//
struct ffQueueItem {
		int x,y; 
	};

typedef struct ffQueueItem ffQueueItem;

#define ffStackAdd( c_x , c_y) {						\
	int nx=c_x,ny=c_y;										\
	unsigned int bp;										\
	if (warpover) nx=(nx + srcWidth) % srcWidth;				\
	if ((nx>=0) && (nx<srcWidth) && (ny>=0) && (ny<srcHeight)) {	\
		int pp,p=(int)nx+ny*srcWidth;								\
		pp=p / 32;													\
		bp=1 << (p % 32);											\
		if ((ffIsInQueue[pp] & bp)==0) {							\
			ffStack[ffStackTop].x=nx;								\
			ffStack[ffStackTop].y=ny;								\
			ffStackTop++;											\
			assert(ffStackTop<FF_STACK_SIZE);						\
			if (ptmod_last<pp) ptmod_last=pp;						\
			if (ptmod_first>pp) ptmod_first=pp;						\
			ffIsInQueue[pp]|=bp;									\
		}															\
	}																\
}		

/**************************************************************************
Thomas Rauscher, Okt 2005

New Transformation method with antialiasing filters. The new function needs the
inverse of the transformation, so it can't be used as a direct replacement

Rik Littelfield described the function as followed:

To compute the value of an output pixel: 
1. Let the integer coordinates of the output pixel be called COC (central output coordinates). 
2. Transform COC into floating point coordinates within the input image.  Call the latter CIC (central input coordinates). 
3. For all input pixels that "close" to CIC: 
3a. Transform the input pixel's coordinates (IC) to output coordinates (OC). 
3b. Determine the output distance OD = OC - COC 
3c. Use OD and the filter function to determine weight w. 
3d. Accumulate w and w * (input pixel value) 
4. Store output pixel value = (weighted sum of input pixels) / (sum of weights) 

"Close" in step 3 is defined as being any pixel that will have non-zero weight.  
To find those the function uses a flood-fill algorithm based on steps 3a-3c. 

The function caches the transformed coordinates OC to speed things up. 

*/


void transForm_aa( TrformStr *TrPtr, fDesc *fD,fDesc *finvD, int color, int imageNum){
	register uint32_t 	x, y;		// Loop through destination image
	int 			skip = 0;	// Update progress counter
	unsigned char 		*dest,*src;// Source and destination image data
										// Message to be displayed by progress reporter
	char*			progressMessage = "Something is wrong here";
	char                	percent[8];	// Number displayed by Progress reporter
	int			valid;		// Is this pixel valid? (i.e. inside source image)
	long			coeff;		// pixel coefficient in destination image
	long			cy;		// rownum in destimage
	int			xc,yc;

	double 			x_d, y_d;	// Cartesian Coordinates of point ("target") in Destination image
	double 		  	Dx, Dy;		// Coordinates of target in Source image
	double 		  	orgDx, orgDy;		// Coordinates of target in Source image

	double			max_x = (double) TrPtr->src->width; // Maximum x values in source image
	double			max_y = (double) TrPtr->src->height; // Maximum y values in source image
	double			min_x =  -1.0;//0.0; // Minimum x values in source image
	double			min_y =  -1.0;//0.0; // Minimum y values in source image

	int			mix	  = TrPtr->src->width - 1; // maximum x-index src
	
	int			mix2;
	int			miy	  = TrPtr->src->height - 1;// maximum y-index src
	int			miy2;

	// Variables used to convert screen coordinates to cartesian coordinates

	double 			w2 	= (double) TrPtr->dest->width  / 2.0 - 0.5;  // Steve's L
	double 			h2 	= (double) TrPtr->dest->height / 2.0 - 0.5;
	double 			sw2 = (double) TrPtr->src->width   / 2.0 - 0.5;
	double 			sh2 = (double) TrPtr->src->height  / 2.0 - 0.5;
	
	int			BytesPerLine	= TrPtr->src->bytesPerLine;
	int			FirstColorByte, SamplesPerPixel;
	unsigned int	BytesPerPixel, BytesPerSample;

	int			n, n2;		// How many pixels should be used for interpolation	
	// int 			lu = 0;		// Use lookup table?
	int			wrap_x = FALSE;
	double			theGamma;	// gamma handed to SetUpGamma()
	
	// Some things for the floodfill algorithm
	
	invCacheItem *invCache;

	ffQueueItem ffItem;

	int ptmod_last=0,ptmod_first=0;
	int ffStackTop=0,ffIsInQueueSize;
	ffQueueItem *ffStack;
	int			srcWidth; 
	int			srcHeight; 
	uint32_t *ffIsInQueue;

	int ccx,ccy;
	double d,sd,ox,oy;
	long cp;

	// Variables for antialiasing filter
	aaFilter aafilter;
	double aaSupport=0;

	//////////////////////////////////////////////////////////////////////////

	// Selection rectangle
	PTRect			destRect;
	if( TrPtr->dest->selection.bottom == 0 && TrPtr->dest->selection.right == 0 ){
		destRect.left 	= 0;
		destRect.right	= TrPtr->dest->width;
		destRect.top	= 0;
		destRect.bottom = TrPtr->dest->height;
	}else{
		memcpy( &destRect, &TrPtr->dest->selection, sizeof(PTRect) );
	}

	srcWidth  = TrPtr->src->width; 
	srcHeight  = TrPtr->src->height; 

	switch( TrPtr->src->bitsPerPixel ){
		case 128:	FirstColorByte = 4; BytesPerPixel = 16; SamplesPerPixel = 4; BytesPerSample = 4; break;
		case  96:	FirstColorByte = 0; BytesPerPixel = 12; SamplesPerPixel = 3; BytesPerSample = 4; break;
		case  64:	FirstColorByte = 2; BytesPerPixel = 8; SamplesPerPixel = 4; BytesPerSample = 2; break;
		case  48:	FirstColorByte = 0; BytesPerPixel = 6; SamplesPerPixel = 3; BytesPerSample = 2; break;
		case  32:	FirstColorByte = 1; BytesPerPixel = 4; SamplesPerPixel = 4; BytesPerSample = 1; break;
		case  24:	FirstColorByte = 0; BytesPerPixel = 3; SamplesPerPixel = 3; BytesPerSample = 1; break;
		case   8:	FirstColorByte = 0; BytesPerPixel = 1; SamplesPerPixel = 1; BytesPerSample = 1; break;
		default :	PrintError("Unsupported Pixel Size: %d", TrPtr->src->bitsPerPixel);
					TrPtr->success = 0;
					return;
	}
/* Patch for PTStitcher to support 32 bit
	if ((TrPtr->dest->dataSize==0) && (TrPtr->dest->bitsPerPixel==128)) {
		TrPtr->dest->bitsPerPixel=TrPtr->src->bitsPerPixel;
		TrPtr->dest->bytesPerLine =TrPtr->dest->width * (TrPtr->dest->bitsPerPixel / 8) ; 
		TrPtr->dest->dataSize=TrPtr->dest->height * TrPtr->dest->bytesPerLine;
		myfree ((unsigned char**)TrPtr->dest->data);
		TrPtr->dest->data=(unsigned char**) mymalloc ((size_t)TrPtr->dest->dataSize);
	}
*/
	// Set interpolator etc:
	n=1;
	switch( TrPtr->interpolator ){
		case _aabox:
			aaSupport = 0.5;
			aafilter=Box;
			break;
		case _aatriangle:
			aaSupport = 1;
			aafilter=Triangle;
			break;
		case _aahermite:
			aaSupport = 1;
			aafilter=Hermite;
			break;
		case _aahanning:
			aaSupport = 1;
			aafilter=Hanning;
			break;
		case _aahamming:
			aaSupport = 1;
			aafilter=Hamming;
			break;
		case _aablackman:
			aaSupport = 1;
			aafilter=Blackman;
			break;
		case _aagaussian:
			aaSupport = 1.25;
			aafilter=Gaussian;
			break;
		case _aagaussian2:
			aaSupport = 1.0;
			aafilter=Gaussian_2;
			break;
		case _aaquadratic:
			aaSupport = 1.5;
			aafilter=Quadratic;
			break;
		case _aacubic:
			aaSupport = 2;
			aafilter=Cubic;
			break;
		case _aacatrom:
			aaSupport = 2;
			aafilter=Catrom;
			break;
		case _aamitchell:
			aaSupport = 2;
			aafilter=Mitchell;
			break;
		case _aalanczos2: // antialias lanczos2
			aaSupport = 2;
			aafilter=Lanczos2;
			break;
		case _aalanczos3: // antialias lanczos3
			aaSupport = 3;
			aafilter=Lanczos3;
			break;
		case _aablackmanbessel:
			aaSupport = 3.2383;
			aafilter=BlackmanBessel;
			break;
		case _aablackmansinc:
			aaSupport = 4;
			aafilter=BlackmanSinc;
			break;
		default: 
			PrintError( "Invalid Antialiased Interpolator selected" );
			TrPtr->success = 0;
			return;
	}

	n2 = n/2 ;
	mix2 = mix +1 - n;
	miy2 = miy +1 - n;

	dest = *TrPtr->dest->data;
	src  = *TrPtr->src->data; // is locked

	if(TrPtr->mode & _show_progress){
		switch(color){
			case 0:  { 
                            char title[30];
#if BROKEN
                            int the_Num;
                            NumToString(imageNum, the_Num);
                            p2cstr(the_Num);
                            strcpy(title, "Converting Image #");
                            strcat(title, (char *)the_Num);
#else
                            sprintf(title, "Converting Image #%d", imageNum);
#endif
                            strcpy(progressMessage, title);	
                            //progressMessage = "Image Conversion"; 	
                        }
                        break;
			case 1:	switch( TrPtr->src->dataformat){
						case _RGB: 	progressMessage = "Red Channel"  ; break;
						case _Lab:	progressMessage = "Lightness" 	 ; break;
					} break;
			case 2:	switch( TrPtr->src->dataformat){
						case _RGB: 	progressMessage = "Green Channel"; break;
						case _Lab:	progressMessage = "Color A" 	 ; break;
					} break; 
			case 3:	switch( TrPtr->src->dataformat){
						case _RGB: 	progressMessage = "Blue Channel"; break;
						case _Lab:	progressMessage = "Color B" 	; break;
					} break; 
					
			case 4:	switch( TrPtr->src->dataformat)
            {
                case _RGB: 	progressMessage = "Red/Grn Channels " ; break;
                case _Lab:	progressMessage = "Unsupported!!"; break;
            } break;
                
			case 5:	switch( TrPtr->src->dataformat)
            {
                case _RGB: 	progressMessage = "Red/Blue Channels " ; break;
                case _Lab:	progressMessage = "Unsupported!!"; break;
            } break; 
                
			case 6:	switch( TrPtr->src->dataformat)
            {
                case _RGB: 	progressMessage = "Grn/Blue Channels " ; break;
                case _Lab:	progressMessage = "Unsupported!!"; break;
            } break; 
					
					
//			default: progressMessage = "Something is wrong here";
		}
		Progress( _initProgress, progressMessage );
	}

	if(TrPtr->mode & _wrapX)
		wrap_x = TRUE;

	if( TrPtr->src->dataformat == _RGB )	// Gamma correct only RGB-images
		theGamma = TrPtr->gamma;
	else
		theGamma = 1.0;
	
	if (BytesPerSample<=2) {
		if( SetUpGamma( theGamma, BytesPerSample) != 0 ){
			PrintError( "Could not set up lookup table for Gamma Correction" );
			TrPtr->success = 0;
			goto Trform_exit;
		}
	}

	// Allocate the memory for the Stack, Floodfill markers and Cache
	invCache= calloc(INV_CACHE_SIZE * sizeof(invCacheItem),1);
	ffStack= calloc(FF_STACK_SIZE * sizeof(ffQueueItem),1);
	ffIsInQueueSize= ((srcWidth*srcHeight) / 32) + 1;
	ffIsInQueue=  calloc(ffIsInQueueSize * sizeof(uint32_t),1);

	for(y=destRect.top; y<destRect.bottom; y++){
		// Update Progress report and check for cancel every 2%.
		skip++;
		if( skip == (int)ceil(TrPtr->dest->height/50.0) ){
			if(TrPtr->mode & _show_progress){	
				sprintf( percent, "%d", (int) ((y * 100)/ TrPtr->dest->height));
				if( ! Progress( _setProgress, percent ) ){
					TrPtr->success = 0;
					goto Trform_exit;
				}
			}else{
				if( ! Progress( _idleProgress, 0) ){
					TrPtr->success = 0;
					goto Trform_exit;
				}
			}
			skip = 0;
		}
		
		// y-coordinate in dest image relative to center		
		y_d = (double) y - h2 ;
		cy  = (y-destRect.top) * TrPtr->dest->bytesPerLine;	
		
		for(x=destRect.left; x<destRect.right; x++){
			// Calculate pixel coefficient in dest image just once

			coeff = cy  + BytesPerPixel * (x-destRect.left);		

			// FS+
			{
				// Convert destination screen coordinates to cartesian coordinates.			
				x_d = (double) x - w2 ;
				
				// Get source cartesian coordinates 
				fD->func( x_d, y_d , &Dx, &Dy, fD->param);

				orgDx=Dx;
				orgDy=Dy;

				Dx += sw2;
				Dy += sh2;
				
				// Is the pixel valid, i.e. from within source image?
				if( (Dx >= max_x)   || (Dy >= max_y) || (Dx < min_x) || (Dy < min_y)  )
					valid = FALSE;
				else
					valid = TRUE;
			}

			// Convert only valid pixels
			if( valid ){


				// Extract integer and fractions of source screen coordinates
				xc 	  =  (int)floor( Dx ) ; Dx -= (double)xc;
				yc 	  =  (int)floor( Dy ) ; Dy -= (double)yc;
				
				// if alpha channel marks valid portions, set valid 
				if(TrPtr->mode & _honor_valid)
				switch( FirstColorByte ){
					case 1:{
						int xn = xc, yn = yc;
						if( xn < 0 ) xn = 0; //  -1  crashes Windows
						if( yn < 0 ) yn = 0; //  -1  crashes Windows
						if( src[ yn * BytesPerLine + BytesPerPixel * xn] == 0 )
							valid = FALSE;
						}
						break;
					case 2:{
						int xn = xc, yn = yc;
						if( xn < 0 ) xn = 0; //  -1  crashes Windows
						if( yn < 0 ) yn = 0; //  -1  crashes Windows
						if( *((uint16_t*)(src + yn * BytesPerLine + BytesPerPixel * xn)) == 0 )
							valid = FALSE;
						}
						break;
					case 4:{
						int xn = xc, yn = yc;
						if( xn < 0 ) xn = 0; //  -1  crashes Windows
						if( yn < 0 ) yn = 0; //  -1  crashes Windows
						if( *((float*)(src + yn * BytesPerLine + BytesPerPixel * xn)) == 0 )
							valid = FALSE;
						}
						break;
					default: break;
				}
			}
			
			if( valid ){	

				int warpover=(TrPtr->src->format==_equirectangular);

				int bx,by,ex,ey;											
				double DstX,DstY,rDstX,rDstY;
				double weight,w,rd,gd,bd;
				uint32_t *ptui;

				bx = (int)(floor(orgDx + sw2));
				ex = (int)(ceil(orgDx + sw2));
				by = (int)(floor(orgDy + sh2));
				ey = (int)(ceil(orgDy + sh2));
				
				// Clear only the modified floodfill markers
				ptui=&ffIsInQueue[ptmod_first];
				if (ptmod_last>=ptmod_first) memset(ptui,0,(ptmod_last-ptmod_first+1)*sizeof(ptui[0]));
				ptmod_first=ffIsInQueueSize;
				ptmod_last=0;

				// Add the 4 surrounding pixels as seeds for the floodfile algorithm
				ffStackAdd(bx,by);
				ffStackAdd(ex,by);
				ffStackAdd(bx,ey);
				ffStackAdd(ex,ey);

				weight=0;
				rd=0;
				gd=0;
				bd=0;
				while (ffStackTop) {
					// Get the next position from the Stack
					ffItem=ffStack[--ffStackTop];
					ccx=ffItem.x;
					ccy=ffItem.y;
					
					// Calculate the hash, +1 to avoid the 0,0 problem
					cp=(1 + ccx + ccy*INV_CACHE_FY) % INV_CACHE_SIZE; 
					if ((invCache[cp].dstX==ccx) && (invCache[cp].dstY==ccy)) { 
						// Cachehit
						DstX=invCache[cp].srcX;
						DstY=invCache[cp].srcY;
					} else {
						// Calculate the invers function to get the exact position in the source image
						finvD->func((ccx-sw2), (ccy-sh2) , &DstX, &DstY, finvD->param);
						invCache[cp].dstX=ccx;
						invCache[cp].dstY=ccy;
						invCache[cp].srcX=DstX;
						invCache[cp].srcY=DstY;
					}
 					
					// distance from the exact of the source pixel to the 
					// position in the destination image
					rDstX=x_d-DstX;
					rDstY=y_d-DstY;

					// distance from the exact position in the source image
					ox=(ccx-sw2) - orgDx;
					oy=(ccy-sh2) - orgDy;
					
					if (warpover) {
						if (ox > max_x/2.0) ox-=max_x;
						if (ox < -max_x/2.0) ox+=max_x;
						if (rDstX > max_x/2.0) rDstX-=max_x;
						if (rDstX < -max_x/2.0) rDstX+=max_x;
					}

					sd=sqrt(rDstX*rDstX + rDstY*rDstY);
					d=sqrt(ox*ox + oy*oy);

					if (sd>d) { // we are upscaling!
						rDstX*=d/sd;
						rDstY*=d/sd;
					}

					// Calculate the weight for the current pixel of the source image
					if ((fabs(rDstX)<aaSupport) && (fabs(rDstY)<aaSupport)) {
						w=aafilter(rDstX,aaSupport) * aafilter(rDstY,aaSupport);
					} else {
						w=0;
					}
					if (w!=0) {
						// Sum the total weight
						weight +=w;			
						// Add the weighted color values
						switch(BytesPerSample) {
							case 1:	{	
										register unsigned char *ric=src + FirstColorByte + ccx*BytesPerPixel + ccy*BytesPerLine; // Pointer for 1 byte per pixel
										rd += glu.DeGamma[(int)*ric] * w;
										ric++;
										gd += glu.DeGamma[(int)*ric] * w;					
										ric++;
										bd += glu.DeGamma[(int)*ric] * w;	
									}
									break;
							case 2:	{
										register unsigned short *ris=(unsigned short *)((char *)src + FirstColorByte + ccx*BytesPerPixel + ccy*BytesPerLine); 
										rd += glu.DeGamma[(int)*ris] * w;
										ris++;
										gd += glu.DeGamma[(int)*ris] * w;					
										ris++; 
										bd += glu.DeGamma[(int)*ris] * w;	
									}
									break;
							case 4:{
										register float *rif=(float *)((char *)src + FirstColorByte + ccx*BytesPerPixel + ccy*BytesPerLine); 
										rd += *rif * w;
										rif++;
										gd += *rif * w;					
										rif++;
										bd += *rif * w;	
									}
									break;
						}
						// Add the surround pixels as seeds to the stack
						ffStackAdd(ccx-1,ccy-1);
						ffStackAdd(ccx-1,ccy  );
						ffStackAdd(ccx-1,ccy+1);

						ffStackAdd(ccx+1,ccy-1);
						ffStackAdd(ccx+1,ccy  );
						ffStackAdd(ccx+1,ccy+1);
						
						ffStackAdd(ccx,ccy-1);
						ffStackAdd(ccx,ccy+1);
					}
				}

				if (weight==0) weight=1; // Just in case....
				switch(BytesPerSample) {
					case 1:	{
								register unsigned char *aadst=&(dest[ coeff ]);
								if (FirstColorByte) {
									*aadst++=UCHAR_MAX;		 // Set alpha channel
								}
								if ((color==0) || (color==1) || (color==4) || (color==5)) {
									*aadst  =   gamma_char( rd/weight );
								}
								aadst++;
								if ((color==0) || (color==2) || (color==4) || (color==6)) {
									*aadst  =   gamma_char( gd/weight );
								}
								aadst++;
								if ((color==0) || (color==3) || (color==5) || (color==6)) {
									*aadst  =   gamma_char( bd/weight );
								}
							}
							break;
					case 2:	{
								register unsigned short *aadst=(void *)&(dest[ coeff ]);
								if (FirstColorByte) {
									*aadst++=USHRT_MAX;		 // Set alpha channel
								}
								if ((color==0) || (color==1) || (color==4) || (color==5)) {
									*aadst  =   gamma_short( rd/weight );
								}
								aadst++;
								if ((color==0) || (color==2) || (color==4) || (color==6)) {
									*aadst  =   gamma_short( gd/weight );
								}
								aadst++;
								if ((color==0) || (color==3) || (color==5) || (color==6)) {
									*aadst  =   gamma_short( bd/weight );
								}
							}
							break;
					case 4:	{
								float *aadst=((float*)(dest + coeff));
								if (FirstColorByte) {
									*aadst++=1.0; // Set alpha channel
								}
								if ((color==0) || (color==1) || (color==4) || (color==5)) {
									*aadst  = gamma_float(rd/weight);
								}
								aadst++;
								if ((color==0) || (color==2) || (color==4) || (color==6)) {
									*aadst  = gamma_float(gd/weight);
								}
								aadst++;
								if ((color==0) || (color==3) || (color==5) || (color==6)) {
									*aadst  = gamma_float(bd/weight);
								}
							}
							break;
				}
                }// END: if is a valid pixel
                else
                {  
                    // not valid (source pixel x,y not inside source image, etc.)
                    
                    //Fix: Correct would use incorrect correction values if different factors were set for each color channel
                    //PT.Fix.mt.Begin: March.2004
                    //was:
                    //memset( &(dest[ coeff ]), 0 ,BytesPerPixel );
                    //now:
                    if(color==0) // RGB same time
                    {
                        memset( &(dest[ coeff ]), 0 ,BytesPerPixel ); //mt_test
                                                                      //PT.Dev.mt.End: March.2004( &(dest[ coeff ]), 0 ,BytesPerPixel ); 
                    }
                    else
                    {
                        char*   ptr = &(dest[ coeff ]);
                    
                        if(color < 4) // R or G or B
                        {
                        ptr += FirstColorByte + (color - 1)*BytesPerSample;
                        memset( ptr, 0 , BytesPerSample ); //mt_test
                    }	
                    else
                        if(color==4) // R+G
                        {
                            ptr += FirstColorByte;
                            memset( ptr, 0 , 2*BytesPerSample ); //rjp
                        }
                        else
                        if(color==5) // R+B
                        {
                            ptr += FirstColorByte;
                            memset( ptr, 0 , BytesPerSample ); 
                            ptr += 2*BytesPerSample;
                            memset( ptr, 0 , BytesPerSample ); //rjp
                        }
                        else // (color==6) G+B
                        {
                            ptr += FirstColorByte + BytesPerSample;
                            memset( ptr, 0 , 2*BytesPerSample ); //rjp
                        }
                    }
                    
                }// END: else Not a valid pixel
		}
	}
	TrPtr->success = 1;


Trform_exit:
	if( glu.DeGamma )	free( glu.DeGamma ); 	glu.DeGamma 	= NULL;
	if( glu.Gamma )		free( glu.Gamma );	glu.Gamma 	= NULL;

	if(invCache != NULL) free(invCache);
	if(ffStack != NULL) free(ffStack);
	if(ffIsInQueue != NULL) free(ffIsInQueue);

	return;
}



#if 0

// An unused lookup version of sinc256
// Somewhat  faster on non-floating point machines like Intel

static double*   SetUpWeights(  );


// Weigths for sinc function

#define	NUM_WEIGHTS	256

static double *wt = NULL;




#define		SINC256( x, a )						\
	if( wt == NULL ) wt = SetUpWeights( );		\
	if( wt != NULL )							\
	{											\
		int xn = x * NUM_WEIGHTS;				\
		a[15]	= wt[ 8*NUM_WEIGHTS - xn ];		\
		a[14]	= wt[ 7*NUM_WEIGHTS - xn ];		\
		a[13]	= wt[ 6*NUM_WEIGHTS - xn ];		\
		a[12]	= wt[ 5*NUM_WEIGHTS - xn ];		\
		a[11]	= wt[ 4*NUM_WEIGHTS - xn ];		\
		a[10]	= wt[ 3*NUM_WEIGHTS - xn ];		\
		a[ 9]	= wt[ 2*NUM_WEIGHTS - xn ];		\
		a[ 8]	= wt[ 1*NUM_WEIGHTS - xn ];		\
		a[ 7]	= wt[ 0*NUM_WEIGHTS + xn ];		\
		a[ 6]	= wt[ 1*NUM_WEIGHTS + xn ];		\
		a[ 5]	= wt[ 2*NUM_WEIGHTS + xn ];		\
		a[ 4]	= wt[ 3*NUM_WEIGHTS + xn ];		\
		a[ 3]	= wt[ 4*NUM_WEIGHTS + xn ];		\
		a[ 2]	= wt[ 5*NUM_WEIGHTS + xn ];		\
		a[ 1]	= wt[ 6*NUM_WEIGHTS + xn ];		\
		a[ 0]	= wt[ 7*NUM_WEIGHTS + xn ];		\
	}											\

// Create Weights for A * NUM_WEIGHTS positions

static double* SetUpWeights(  )
{
#define A 	8
	int i,k,id;
	double dx = 1.0 / (double)NUM_WEIGHTS;
	double *w;
	
	w = (double*)malloc( A * NUM_WEIGHTS * sizeof(double) );
	if( w )
	{
		for( k=0; k < A ; k++ )
		{
			id = k * NUM_WEIGHTS;
			for( i=0; i<NUM_WEIGHTS; i++ )
			{
				w[id + i] = sinc8( (double)k + i*dx );
			}
		}
	}
	return w;
}
		
#undef A


#endif		




#if 0
/////////////// Results of calc for poly3  ////////////////////////////////////////////////////////////	

Equations:

1:  y0 = -a3 + a2 - a1 + a0 
2:  y1 = a0
3:  y2 = a3 + a2 + a1 + a0
4:  y3 = 8 a3 + 4 a2 + 2 a1 + a0


--- Emacs Calculator Mode ---
4:  a1 = y2 - y3 / 6 - y0 / 3 - y1 / 2
3:  6 a3 = y3 - y0 + 3 y1 - 3 y2
2:  2 a2 = y2 + y0 - 2 y1
1:  a0 = y1


/////////////// Results of Calc for Spline 16 ////////////////////////////////////////////////////////////	

1.  y0 = -c3 + c2 - c1 + c0
2.  y1 = c0
3.  y1 = a0
4.  y2 = a3 + a2 + a1 + a0
5.  y2 = b3 + b2 + b1 + b0
6.  y3 = 8 b3 + 4 b2 + 2 b1 + b0
7.  c1 = a1
8.  3 a3 + 2 a2 + a1 = 3 b3 + 2 b2 + b1
9.  2 c2 = 2 a2
10. 6 a3 + 2 a2 = 6 b3 + 2 b2
11. -6 c3 + 2 c2 = 0
12. 12 b3 + 2 b2 = 0

/////////////// Results of Calc for Spline 36 ////////////////////////////////////////////////////////////	

Equations:

--- Emacs Calculator Mode ---
20: 18 c3 + 2 c2 = 0
19: 2 e2 - 12 e3 = 0
18: 12 b3 + 2 b2 = 12 c3 + 2 c2
17: 6 a3 + 2 a2 = 6 b3 + 2 b2
16: 2 d2 = 2 a2
15: 2 e2 - 6 e3 = 2 d2 - 6 d3
14: 12 b3 + 4 b2 + b1 = 12 c3 + 4 c2 + c1
13: 3 a3 + 2 a2 + a1 = 3 b3 + 2 b2 + b1
12: d1 = a1
11: 3 e3 - 2 e2 + e1 = 3 d3 - 2 d2 + d1
10: 27 c3 + 9 c2 + 3 c1 + c0 = y5
9:  8 c3 + 4 c2 + 2 c1 + c0 = y4
8:  y4 = 8 b3 + 4 b2 + 2 b1 + b0
7:  y3 = b3 + b2 + b1 + b0
6:  y3 = a3 + a2 + a1 + a0
5:  y0 = 4 e2 - 8 e3 - 2 e1 + e0
4:  y1 = e2 - e3 - e1 + e0
3:  y1 = d2 - d3 - d1 + d0
2:  y2 = d0
1:  y2 = a0

--- Emacs Calculator Mode ---
4:  11 a3 = 6 y4 - y5 - 13 y3 - 6 y1 + y0 + 13 y2
3:  209 a1 
      = 7 y5 + 168 y3 - 42 y4 - 3 y2 + 26 y0 
          - 156 y1
2:  a2 = 12:209 y5 + 288:209 y3 - 72:209 y4 
           - 45:209 y0 - 453:209 y2 + 270:209 y1
1:  a0 = y2


/////////////// Results of Calc for Spline 64 ////////////////////////////////////////////////////////////	

Equations:

1:   y0 = -27 g3 + 9 g2 - 3 g1 + g0
2:   y1 = -8 g3 + 4 g2 - 2 g1 + g0
3:   y1 = -8 f3 + 4 f2 - 2 f1 + f0
4:   y2 = -f3 + f2 - f1 + f0
5:   y2 = -e3 + e2 - e1 + e0
6:   y3 = e0
7:   y3 = a0
8:   y4 = a3 + a2 + a1 + a0
9:   y4 = b3 + b2 + b1 + b0
10:  y5 = 8 b3 + 4 b2 + 2 b1 + b0
11:  y5 = 8 c3 + 4 c2 + 2 c1 + c0
12:  y6 = 27 c3 + 9 c2 + 3 c1 + c0
13:  y6 = 27 d3 + 9 d2 + 3 d1 + d0
14:  y7 = 64 d3 + 16 d2 + 4 d1 + d0
15:  12 g3 - 4 g2 + g1 = 12 f3 - 4 f2 + f1
16:  3 f3 - 2 f2 + f1 = 3 e3 - 2 e2 + e1
17:  e1 = a1
18   3 a3 + 2 a2 + a1 = 3 b3 + 2 b2 + b1
19   12 b3 + 4 b2 + b1 = 12 c3 + 4 c2 + c1
20   27 c3 + 6 c2 + c1 = 27 d3 + 6 d2 + d1
21   -12 g3 + 2 g2 = -12 f3 + 2 f2
22   -6 f3 + 2 f2 = -6 e3 + 2 e2 
23   2 e2 = 2 a2
24   6 a3 + 2 a2 = 6 b3 + 2 b2
25   12 b3 + 2 b2 = 12 c3 + 2 c2
26   18 c3 + 2 c2 = 18 d3 + 2 d2
27   -18 g3 + 2 g2 = 0
28   24 d3 + 2 d2 = 0

--- Emacs Calculator Mode ---
4:  41 a3 
      = y7 + 24 y5 - 6 y6 - 49 y4 - 24 y2 - y0 
          + 49 y3 + 6 y1
3:  a2 = 270:2911 y6 + 4050:2911 y4 - 45:2911 y7 
           - 1080:2911 y5 + 168:2911 y0 
           + 4032:2911 y2 - 1008:2911 y1 
           - 6387:2911 y3
2:  2911 a1 
      = 156 y6 + 2340 y4 - 26 y7 - 624 y5 + 582 y1 
          - 3 y3 - 2328 y2 - 97 y0
1:  a0 = y3



#endif


