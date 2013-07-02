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
   
#include "filter.h"
#include "fftn.h"
#include "f2c.h"

static  void 	fconvolution		( TrformStr *TrPtr, Image *psf );
static 	void 	makePSF				( uint32_t width, uint32_t height, Image *image, double *re, double *im, int color, int direction );
static  void 	makeDoubleDiffImage ( Image *src, Image *fimage, double *re, double *im, int color );
static  int 	makeDoubleImage		( Image *image, double *re, double *im, int color, double pgamma );
static  void 	makeUcharImage		( Image *image, double *re, int color );
// static  void 	makeGaussPSF		( Image *im, double s );
static  void 	windowFunction		( double *im, uint32_t width, uint32_t height, double frame);
// static  void 	invWindowFunction	( double *im, int width, int height, double frame);
static void 	fresize				( TrformStr *TrPtr );




void	fourier	( TrformStr *TrPtr, cPrefs *cP )
{
	Image psf, nff;
	psf.data = nff.data = NULL;
	
	switch( cP->fourier_mode )
	{
		case _faddBlurr:
				if( readPSD( &psf, &(cP->psf), 1) != 0 )
				{
					PrintError("Error reading Point Spread Image");
					TrPtr->success = 0;
					return;
				}
				fconvolution( TrPtr, &psf );
				break;
		
		case _fremoveBlurr:
				if( readPSD( &psf, &(cP->psf), 1) != 0 )
				{
					PrintError("Error reading Point Spread Image");
					TrPtr->success = 0;
					return;
				}
				if( cP->fourier_nf == _nf_internal )
				{
					memcpy( &nff, TrPtr->src, sizeof( Image ));
					nff.data = (unsigned char**) mymalloc( (size_t)nff.dataSize );
					if( nff.data == NULL )
					{ 
						PrintError("Not enough memory");
						TrPtr->success = 0;
						goto _fourier_exit;
					}
					noisefilter( &nff, TrPtr->src );
				}
				else 
				{
					if(  readPSD( &nff, &(cP->nff), 1) != 0 )
					{
						PrintError("Error reading Filtered Image");
						TrPtr->success = 0;
						goto _fourier_exit;
					}
					if( !HaveEqualSize( &nff, TrPtr->src ) )
					{
						PrintError("Filtered Image and Source must have equal Size and Pixel Size");
						TrPtr->success = 0;
						goto _fourier_exit;
					}
				}
				// Now we have a noise filtered image in nff
		
				fwiener( TrPtr, &nff, &psf, cP->filterfactor, cP->fourier_frame );
				myfree( (void**) nff.data );
				break;
		
		case _fresize:
				{					
					if(cP->width <= 0 && cP->height <= 0)
					{
						PrintError("Parameter error: New image dimensions");
						TrPtr->success = 0;
						goto _fourier_exit;
					}
					fresize( TrPtr );
				}
						
				break;

		default:PrintError("Unknown Error");
				TrPtr->success = 0;
				return;
				break;
	}

_fourier_exit:	
	if( psf.data ) myfree( (void**)psf.data );
}





#define BX 1 
// 2
#define MSUM 2.6
//4.4
void noisefilter( Image *dest, Image *src )	
{
	register int x,y,cy,cx,i,k;
	int  fc, bpp;
	
	register unsigned char *s, *d;
	double r,g,b, fi;
#if 0
	double bl[5][5] = 	{	{ 0.0, 	0.1, 	0.2, 	0.1,	0.0},
							{ 0.1, 	0.2, 	0.5, 	0.2, 	0.1},
							{ 0.2, 	0.5, 	1.0, 	0.5, 	0.2},
							{ 0.1, 	0.2, 	0.5, 	0.2, 	0.1},
							{ 0.0, 	0.1, 	0.2, 	0.1, 	0.0}};
#endif
	double bl[3][3] = 	{	{ 0.1, 0.3, 0.1},
							{ 0.3, 1.0, 0.3},
							{ 0.1, 0.3, 0.1}};


	d = *(dest->data);
	s = *(src->data);
	
	if( src->bitsPerPixel == 32 )
	{
		fc = 1;
		bpp = 4;
	}
	else
	{
		fc = 0;
		bpp = 3;
	}
		
	
	memcpy( d, s, (size_t)(dest->dataSize));
	
		
	for(y=BX; y<src->height-BX; y++)
	{
		for(x=BX; x<src->width-BX;x++)
		{
			cy = y*src->bytesPerLine + x*bpp + fc;
			
			r = b = g = 0.0;
			for(i=-BX; i<=BX; i++)
			{
				for(k=-BX; k<=BX; k++)
				{
					cx = cy + k*bpp + i*src->bytesPerLine;
					fi = bl[i+BX][k+BX];
					r+=(double)s[cx] * fi;
					g+=(double)s[cx+1] * fi;
					b+=(double)s[cx+2] * fi;
				}
			}
			r/= MSUM ; g/= MSUM; b/= MSUM;
			DBL_TO_UC( d[cy],   r );
			DBL_TO_UC( d[cy+1], g );
			DBL_TO_UC( d[cy+2], b );
			
		}
	}
	return;
}


#define UPDATE_PROGRESS_CONVOLUTION			prog += delta; sprintf( percent, "%d", prog );		\
											if( ! Progress( _setProgress, percent ) )			\
											{													\
												TrPtr->success = 0; goto _fconvolution_exit;	\
											}													


static void fconvolution( TrformStr *TrPtr, Image *psf )
{
	int 		dims[2], i, k, dim, prog=0, delta = 100/15;
	double		**Re = NULL, **Im = NULL, **PRe = NULL, **PIm = NULL;
	char		percent[25];

	dims[0] = TrPtr->src->width;
	dims[1] = TrPtr->src->height;
	dim = TrPtr->src->width * TrPtr->src->height;
	
	Progress( _initProgress, "Convolution Filter" );
	
	Re 	= (double**)mymalloc( dim * sizeof( double ) );
	Im 	= (double**)mymalloc( dim * sizeof( double ) );
	PRe = (double**)mymalloc( dim * sizeof( double ) );
	PIm = (double**)mymalloc( dim * sizeof( double ) );
	if( Re == NULL || Im == NULL || PRe == NULL || PIm == NULL)
	{
		PrintError("Not enough memory");
		TrPtr->success = 0;
		goto _fconvolution_exit;
	}

	
	for( i=0; i<3; i++ )
	{
		register double x,y,a,b;
		
		UPDATE_PROGRESS_CONVOLUTION

		makePSF( TrPtr->src->width, TrPtr->src->height, psf, *PRe, *PIm, i, 1 );
		
		fftn (2, dims, *PRe, *PIm, 1, 1.0 );

		UPDATE_PROGRESS_CONVOLUTION

		
		if(  makeDoubleImage ( TrPtr->src, *Re, *Im, i, TrPtr->gamma ) != 0 )
		{
			PrintError("Could not make Real-version of image");
			TrPtr->success = 0;
			goto _fconvolution_exit;
		}
		fftn (2, dims, *Re, *Im, 1, 1.0 ); // forward transform; don't scale

		UPDATE_PROGRESS_CONVOLUTION

		
		// Multiply with psf
		
		for( k= 0; k<dim; k++)
		{
			x = (*Re)[k]; 	y = (*Im)[k];
			a = (*PRe)[k];	b = (*PIm)[k];
			(*Re)[k] = x*a-y*b;
			(*Im)[k] = x*b+y*a;
		}

		UPDATE_PROGRESS_CONVOLUTION

		
 		fftn (2, dims, *Re, *Im, -1, -1.0 ); // backward transform; scale by dim;

		UPDATE_PROGRESS_CONVOLUTION


		makeUcharImage ( TrPtr->dest, *Re, i );
	}
	TrPtr->success = 1;

_fconvolution_exit:	
	Progress( _disposeProgress, percent );
	fft_free();	
	if( Re  != NULL ) myfree( (void**)Re  ); 
	if( Im  != NULL ) myfree( (void**)Im  );  
	if( PRe != NULL ) myfree( (void**)PRe );
	if( PIm != NULL ) myfree( (void**)PIm );

}

	

#define UPDATE_PROGRESS_WIENER				prog += delta; sprintf( percent, "%d", prog );		\
											if( ! Progress( _setProgress, percent ) )			\
											{													\
												TrPtr->success = 0; goto _fwiener_exit;			\
											}													


void fwiener( TrformStr *TrPtr, Image *nf, Image *psf, double gamma , double frame)
{
	int 		dims[2], i, k, dim, prog=0, delta = 100/27;
	double		**d1 = NULL, **d2 = NULL, **d3 = NULL, **d4 = NULL; // double arrays
	char		percent[25];

	dims[0] = TrPtr->src->width;
	dims[1] = TrPtr->src->height;
	dim = TrPtr->src->width * TrPtr->src->height;

	Progress( _initProgress, "Wiener Filter" );
	
	d1 	= (double**)mymalloc( dim * sizeof( double ) );
	d2 	= (double**)mymalloc( dim * sizeof( double ) );
	d3 =  (double**)mymalloc( dim * sizeof( double ) );
	d4 =  (double**)mymalloc( dim * sizeof( double ) );

	if( d1 == NULL || d2 == NULL || d3 == NULL || d4 == NULL )
	{
		PrintError("Not enough memory");
		TrPtr->success = 0;
		goto _fwiener_exit;
	}

	
	for( i=0; i<3; i++ )
	{
		register double x,y,si,sn,rden,a,b;
		
		UPDATE_PROGRESS_WIENER
		
		if( makeDoubleImage ( nf, *d1, *d2, i, TrPtr->gamma ) != 0 )
		{
			PrintError("Could not make Real-Version of image");
			TrPtr->success = 0;
			goto _fwiener_exit;
		}
			
		fftn (2, dims, *d1, *d2, 1, 1.0 ); // Noise filtered image

		UPDATE_PROGRESS_WIENER	
		
		makeDoubleDiffImage ( TrPtr->src, nf, *d3, *d4, i );
		fftn (2, dims, *d3, *d4, 1, 1.0 ); // Noise in image

		UPDATE_PROGRESS_WIENER

		for( k= 0; k<dim; k++)
		{
			x = (*d1)[k]; y = (*d2)[k];
			
			(*d1)[k] = x * x + y * y;	// S_ii
			
			x = (*d3)[k]; y = (*d4)[k];
			
			(*d2)[k] = x * x + y * y;	// S_nn
		}

		UPDATE_PROGRESS_WIENER
		
		makePSF( TrPtr->src->width, TrPtr->src->height, psf, *d3, *d4, i, -1 );
		
		fftn (2, dims, *d3, *d4, 1, 1.0 ); // H(w1,w2)

		UPDATE_PROGRESS_WIENER

		for( k= 0; k<dim; k++)
		{

			x = (*d3)[k]; y = (*d4)[k];
			
			si = (*d1)[k];
			sn = (*d2)[k];
			
			if( si == 0.0 )
			{
				(*d1)[k] = 0.0;
				(*d2)[k] = 0.0;
			}
			else
			{
				rden 		= x*x +y*y + gamma * sn/si;
				(*d1)[k] = x / rden ;
				(*d2)[k] = y / rden ;
			}
		}

		UPDATE_PROGRESS_WIENER
		
		if( makeDoubleImage	( TrPtr->src, *d3, *d4, i, TrPtr->gamma ) != 0 )
		{
			PrintError("Could not make Real-Version of image");
			TrPtr->success = 0;
			goto _fwiener_exit;
		}

		if( frame > 0.0)
			windowFunction( *d3, TrPtr->src->width, TrPtr->src->height, frame );

		fftn (2, dims, *d3, *d4, 1, 1.0 ); // v(w1,w2)

		UPDATE_PROGRESS_WIENER


		for( k= 0; k<dim; k++)
		{

			x = (*d3)[k]; y = (*d4)[k];
			a = (*d1)[k]; b = (*d2)[k];
			
			(*d1)[k] = x*a - y*b;
			(*d2)[k] = x*b + y*a;
		}
		
		UPDATE_PROGRESS_WIENER
			
		
 		fftn (2, dims, *d1, *d2, -1, -1.0 ); // backward transform; 

		// invWindowFunction( *d1, TrPtr->src->width, TrPtr->src->height, 25.0);

		UPDATE_PROGRESS_WIENER
		
		makeUcharImage ( TrPtr->dest, *d1,  i );
	}
	TrPtr->success = 1;

_fwiener_exit:	
	Progress( _disposeProgress, percent );
	fft_free();	
	if( d1  != NULL ) myfree( (void**)d1  ); 
	if( d2  != NULL ) myfree( (void**)d2  );  
	if( d3  != NULL ) myfree( (void**)d3  );
	if( d4  != NULL ) myfree( (void**)d4  );

}

// Create double point spread function from colour channel in image.
// Move point from center to (x|y) = (0|0)
// Mirror image (both x & y) if backXform (direction = -1)

static void makePSF( uint32_t width, uint32_t height, Image *image, double *re, double *im, int color, int direction )
{
	uint32_t w, h, w2, h2, dim = width*height, cb, bpp, bpl, yw, cy;
	register uint32_t i,x,y;
	register unsigned char *data = *(image->data);
	register double scale, *r;
	
	w = (width < image->width  ? width/2 : image->width /2 );
	h = (height< image->height ? height/2: image->height/2 );
	w2 = image->width/2; h2 = image->height/2; 
	bpl=image->bytesPerLine;
	
	if( image->bitsPerPixel == 32 )
	{
		cb = 1 + color;
		bpp = 4;
	}
	else
	{
		cb = 0 + color;
		bpp = 3;
	}
	
	for(i=0; i<dim; i++)
	{
		re[i] = 0.0; im[i] = 0.0;
	}

	if( direction == -1 )
	{
		for(y=0; y<h; y++)
		{
			yw = y*width;
			cy = (h2 - y) * bpl + cb;
			for(x=0; x<w; x++)
			{
				re[yw + x] 				= (double)data[cy + (w2 - x) * bpp];
			}

			for(x=1; x<w; x++)
			{
				re[yw + (width-x)] 		= (double)data[cy + (w2 + x) * bpp];
			}
		}
		
		for(y=1; y<h; y++)
		{
			yw = y*width;

			cy = (h2 + y) * bpl + cb;
			for(x=0; x<w; x++)
			{
				re[dim - yw + x] 			= (double)data[cy + (w2 - x) * bpp];
			}
			for(x=1; x<w; x++)
			{
				re[dim - yw + (width-x)] 	= (double)data[cy + (w2 + x) * bpp];
			}
		}
	}
	else
	{
		for(y=0; y<h; y++)
		{
			yw = y*width;
			cy = (h2 + y) * bpl + cb;
			for(x=0; x<w; x++)
			{
				re[yw + x] 				= (double)data[cy + (w2 + x) * bpp];
			}

			for(x=1; x<w; x++)
			{
				re[yw + (width-x)] 		= (double)data[cy + (w2 - x) * bpp];
			}
		}
		
		for(y=1; y<h; y++)
		{
			yw = y*width;

			cy = (h2 - y) * bpl + cb;
			for(x=0; x<w; x++)
			{
				re[dim - yw + x] 			= (double)data[cy + (w2 + x) * bpp];
			}
			for(x=1; x<w; x++)
			{
				re[dim - yw + (width-x)] 	= (double)data[cy + (w2 - x) * bpp];
			}
		}
	}
	// Scale image
	
	scale = 0.0; r = re;
	for(i=0; i<dim; i++)
	{
		scale += *r++;
	}
	scale = 1.0 / scale;
 	r = re;
	for(i=0; i<dim; i++)
	{
		*r *= scale;
		r++;
	}
}


// Create double version of image data
// color = 0,1,2

static int makeDoubleImage( Image *image, double *re, double *im, int color, double pgamma )
{
	register int x,y;
	register unsigned char* data = *(image->data);
	int		cy, dy, cl = color, bpp = image->bitsPerPixel/8;
	
	if( SetUpGamma( pgamma, 1 ) != 0 )
		return -1;
	
	if( bpp == 4 ) cl++;
	
	for( y=0; y<image->height; y++)
	{
		cy = y * image->bytesPerLine + cl;
		dy = y * image->width;
		for( x=0; x<image->width; x++)
		{
			re[ dy + x ] = glu.DeGamma[ (int) (data[ cy + bpp * x ]) ];
			im[ dy + x ] = 0.0;
		}
	}
	return 0;
}

// Subtract image - (noise filtered image)
// allow negative values
static void makeDoubleDiffImage ( Image *src, Image *fimage, double *re, double *im, int color )
{
	register int x,y;
	register unsigned char *data1 = *(src->data), *data2 = *(fimage->data);
	int		cy, dy, cl = color, bpp = src->bitsPerPixel/8, cf;
	
	if( bpp == 4 ) cl++;
	
	for( y=0; y<src->height; y++)
	{
		cy = y * src->bytesPerLine + cl;
		dy = y * src->width;
		for( x=0; x<src->width; x++)
		{
			cf = cy + bpp * x;
			re[ dy + x ] = (double) data1[ cf ] - (double) data2[ cf ];
			im[ dy + x ] = 0.0;
		}
	}
}

// Create unsigned char version of image data
// color = 0,1,2

static void makeUcharImage( Image *image, double *re, int color )
{
	register int x,y;
	register unsigned char *data = *(image->data);
	int		cy, dy, cl = color, bpp = image->bitsPerPixel/8;
	double maxval = 0.0, scale = 1.0;
	
	if( bpp == 4 ) cl++;

	for( y=0; y<image->height; y++)
	{
		cy = y * image->bytesPerLine + cl;
		dy = y * image->width;
		for( x=0; x<image->width; x++)
		{
			if( re[ dy + x ]  > maxval) maxval = re[ dy + x ];
		}
	}

	if(maxval > (double)glu.ChannelSize || maxval < (double)glu.ChannelSize/3.0 )
		scale = (double)glu.ChannelSize / maxval;
		
	for( y=0; y<image->height; y++)
	{
		cy = y * image->bytesPerLine + cl;
		dy = y * image->width;
		for( x=0; x<image->width; x++)
		{
			data[ cy + bpp * x ] = (unsigned char)(gamma_correct( re[ dy + x ] * scale ));
		}
	}
	// Dangerous, but should be ok
	if( glu.DeGamma ) free( glu.DeGamma ); glu.DeGamma 	= NULL;
	if( glu.Gamma )	free( glu.Gamma );	glu.Gamma 	= NULL;
}


/*
static void makeGaussPSF( Image *im, double s )
{
	register int x,y,cy,bpp,fc;
	int w2 = im->width/2, h2 = im->height/2;
	unsigned char pix, *data = *(im->data), *d;
	double xw,yw,sw = s*s;

	if( im->bitsPerPixel == 32 )
	{
		fc = 1;
		bpp = 4;
	}
	else
	{
		fc = 0;
		bpp = 3;
	}
	
	for(y=0; y<im->height; y++)
	{
		cy = y*im->bytesPerLine + fc;
		for(x=0; x<im->width; x++)
		{
			d   = &(data[cy+x*bpp]);
			xw = x-w2; yw = y-h2;
			pix = 255.0 * exp( -(yw*yw + xw*xw)/sw );
			*d++ = pix; *d++ = pix; *d = pix;
		}
	}
}
*/


// Mask image with window function exp( -frame/x )
#define MEDIUMGRAY	127.0

static void windowFunction( double *im, uint32_t width, uint32_t height, double frame)
{
	double *wf;
	register double z;
	uint32_t w2 = width/2, h2 = height/2,dx,dy,cy,i,x,y;
	uint32_t dl = (width < height ? width : height) / 2 + 1;
	
	
	wf = (double*)malloc( dl * sizeof( double ) );
	if( wf == NULL )
	{
		PrintError("Not enough memory to apply windowfunction. Trying without...");
		return;
	}
	
	wf[0] = 0.0; 
	for(i=1; i<dl; i++)
		wf[i] = exp( - frame/ (double)i );
	
	for(y=0; y<height; y++)
	{
		if( y < h2 )
			dy = y;
		else
			dy = height - 1 - y;
		
		cy = y * width;
			
		for(x=0; x<width; x++)
		{
			if( x < w2 )
				dx = x;
			else
				dx = width - 1 - x;
			
			if( dy < dx )
				dx = dy;
			
			//im[cy+x] *= wf[dx];
			z = im[cy+x];
			z = MEDIUMGRAY + wf[dx] * ( z - MEDIUMGRAY );
			DBL_TO_UC( im[cy+x], z );
		}
	}
	free( wf );
}


// Unmask image with window function exp( -frame/x )

/* 
static void invWindowFunction( double *im, int width, int height, double frame)
{
	double *wf;
	int w2 = width/2, h2 = height/2,dx,dy,cy,i,x,y;
	int dl = (width < height ? width : height) / 2 + 1;
	
	
	wf = (double*)malloc( dl * sizeof( double ) );
	if( wf == NULL )
	{
		PrintError("Not enough memory to apply windowfunction. Trying without...");
		return;
	}
	
	wf[0] = 0.1; // ???????
	for(i=1; i<dl; i++)
		wf[i] = exp( - frame/ (double)i );
	
	for(y=0; y<height; y++)
	{
		if( y < h2 )
			dy = y;
		else
			dy = height - 1 - y;
		
		cy = y * width;
			
		for(x=0; x<width; x++)
		{
			if( x < w2 )
				dx = x;
			else
				dx = width - 1 - x;
			
			if( dy < dx )
				dx = dy;
			
			im[cy+x] /= wf[dx];
		}
	}
	free( wf );
}
*/

#define UPDATE_PROGRESS_ANTIALIAS		prog += delta; sprintf( percent, "%d", prog );		\
											if( ! Progress( _setProgress, percent ) )			\
											{													\
												TrPtr->success = 0; goto _antialias_exit;	\
											}													


static void fresize( TrformStr *TrPtr )
{
	int 		dims[2], dest_dims[2], i, dim, prog=0, delta = 100/12;
	double		**Re = NULL, **Im = NULL;
	char		percent[25];
	double 		*re,*im;
	// unsigned char *ch;
	// int 		x,y,dy,sy,x1,x2,y1,y2,rx,ry;
	int 		x,y,dy,sy,x1,y1,rx,ry;

	
	dims[0] = TrPtr->src->width;
	dims[1] = TrPtr->src->height;
	dim = max( TrPtr->src->width * TrPtr->src->height, TrPtr->dest->width * TrPtr->dest->height );

	dest_dims[1] = TrPtr->dest->height;
	dest_dims[0] = TrPtr->dest->width;

	ry = (TrPtr->src->height - TrPtr->dest->height);
	rx = (TrPtr->src->width  - TrPtr->dest->width);
		
	x1 = min( TrPtr->dest->width/2, TrPtr->src->width/2) ; 
		
	y1 = min( TrPtr->dest->height/2, TrPtr->src->height/2) ;

	
	Progress( _initProgress, "Resize Filter" );
	
	Re 	= (double**)mymalloc( dim * sizeof( double ) );
	Im 	= (double**)mymalloc( dim * sizeof( double ) );
	if( Re == NULL || Im == NULL )
	{
		PrintError("Not enough memory");
		TrPtr->success = 0;
		goto _antialias_exit;
	}
	re = *Re; im = *Im;
	
	for( i=0; i<3; i++ )
	{
		
		UPDATE_PROGRESS_ANTIALIAS
		
		if(  makeDoubleImage ( TrPtr->src, *Re, *Im, i, TrPtr->gamma ) != 0 )
		{
			PrintError("Could not make Real-version of image");
			TrPtr->success = 0;
			goto _antialias_exit;
		}
		fftn (2, dims, *Re, *Im, 1, -1.0 ); // forward transform; don't scale

		UPDATE_PROGRESS_ANTIALIAS

		
		if( TrPtr->dest->width < TrPtr->src->width )
		{
			// Cut frame if decimating	
		
			for( y=0; y<y1; y++)
			{
				sy = y * TrPtr->src->width;
				dy = y * TrPtr->dest->width;
				for(x=0; x<TrPtr->dest->width; x++)
				{
					if(x<x1)
					{
						re[dy+x] = re[sy+x];im[dy+x] = im[sy+x];
					}
					if(x>=x1)
					{
						re[dy+x] = re[sy+x+rx];im[dy+x] = im[sy+x+rx];
					}
				}
			}
			for( y=y1; y<TrPtr->dest->height; y++ )
			{
				sy = (y+ry) * TrPtr->src->width;
				dy = y * TrPtr->dest->width;
				
				for(x=0; x<TrPtr->dest->width; x++)
				{
					if(x<x1)
					{
						re[dy+x] = re[sy+x];im[dy+x] = im[sy+x];
					}
					if(x>=x1)
					{
						re[dy+x] = re[sy+x+rx];im[dy+x] = im[sy+x+rx];
					}
				}
			}
		}
		else
		{
			// Pad if enlarging	
		
			for( y=TrPtr->dest->height-1;y>=y1;  y-- )
			{
				sy = (y+ry) * TrPtr->src->width;
				dy = y * TrPtr->dest->width;
				
				for(x=TrPtr->dest->width-1;x>=0; x--)
				{
					if(x<x1)
					{
						re[dy+x] = re[sy+x];im[dy+x] = im[sy+x];
					}
					else if(x>=x1)
					{
						re[dy+x] = re[sy+x+rx];im[dy+x] = im[sy+x+rx];
					}
					else
					{
						re[dy+x] = 0.0;im[dy+x] = 0.0;
					}
				}
			}
			for( y=y1-1;y>=0;y--)
			{
				sy = y * TrPtr->src->width;
				dy = y * TrPtr->dest->width;
				for(x=TrPtr->dest->width-1;x>=0; x--)
				{
					if(x<x1)
					{
						re[dy+x] = re[sy+x];im[dy+x] = im[sy+x];
					}
					else if(x>=x1)
					{
						re[dy+x] = re[sy+x+rx];im[dy+x] = im[sy+x+rx];
					}
					else
					{
						re[dy+x] = 0.0;im[dy+x] = 0.0;
					}
				}
			}
		}		
		
							
		UPDATE_PROGRESS_ANTIALIAS

		
 		fftn (2, dest_dims, *Re, *Im, -1, 1.0 ); // backward transform; scale by dim;

		UPDATE_PROGRESS_ANTIALIAS
		
		makeUcharImage ( TrPtr->dest, *Re, i );
		
		
	}
	TrPtr->success = 1;

_antialias_exit:	
	Progress( _disposeProgress, percent );
	fft_free();	
	if( Re  != NULL ) myfree( (void**)Re  ); 
	if( Im  != NULL ) myfree( (void**)Im  );  
}



