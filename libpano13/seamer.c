#include "filter.h"

void SetDistance8( Image* im, Image* pano, PTRect *theRect, int showprogress );
void SetDistanceImage8 ( Image* im, Image* pano, PTRect *theRect, int showprogress, int feather );
int  merge8 (  Image *dst, Image *src,  int feather, int showprogress, int seam );
void mergeAlpha8 ( Image *im, unsigned char *alpha, int feather, PTRect *theRect );

void SetDistance16( Image* im, Image* pano, PTRect *theRect, int showprogress );
void SetDistanceImage16 ( Image* im, Image* pano, PTRect *theRect, int showprogress, int feather );
int  merge16 (  Image *dst, Image *src,  int feather, int showprogress, int seam );
void mergeAlpha16 ( Image *im, unsigned char *alpha, int feather, PTRect *theRect );



#define PIXEL_TYPE				unsigned char
#define PIXEL_MAX				UCHAR_MAX
#define DBL_TO_PIX(a,b)			DBL_TO_UC(a,b)
#define _SetDistance			SetDistance8
#define _SetDistanceImage		SetDistanceImage8
#define _merge				merge8
#define _mergeAlpha 			mergeAlpha8

#include "seamer_.c"

#undef PIXEL_TYPE				
#undef PIXEL_MAX				
#undef DBL_TO_PIX			
#undef _SetDistance			
#undef _SetDistanceImage		
#undef _merge					
#undef _mergeAlpha 			

#define PIXEL_TYPE			unsigned short
#define PIXEL_MAX			USHRT_MAX
#define DBL_TO_PIX(a,b)			DBL_TO_US(a,b)
#define _SetDistance			SetDistance16
#define _SetDistanceImage		SetDistanceImage16
#define _merge				merge16
#define _mergeAlpha 			mergeAlpha16


#include "seamer_.c" 


int  merge (  Image *dst, Image *src,  int feather, int showprogress, int seam )
{
	if( dst->bitsPerPixel == 48 || dst->bitsPerPixel == 64 )
		return merge16 (  dst, src,   feather,  showprogress,  seam );
	else
		return  merge8 (  dst, src,   feather,  showprogress,  seam );
}

void mergeAlpha ( Image *im, unsigned char *alpha, int feather, PTRect *theRect )
{
	if( im->bitsPerPixel == 48 || im->bitsPerPixel == 64 )
		mergeAlpha16 (  im, alpha, feather, theRect );
	else
		mergeAlpha8 (  im, alpha, feather, theRect );
}



double GetBlendfactor( int d, int s, int feather ) 
{
	double sfactor;
	sfactor = (-1.0 / (2.0*feather)) * d + 0.5 * ( (double)s / (double)feather + 1.0 );
	// randomize
	return sfactor * ( 1.0 - BLEND_RANDOMIZE * rand() / (double)RAND_MAX );
}



