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

#ifndef FILTER_H
#define FILTER_H

#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#include "panorama.h"
//#include "tiffio.h"

#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

#ifndef bzero
	#define bzero(dest, len)   memset((dest), 0, (len))
#endif


//---------------------- Some useful math defines --------------------------

#ifndef PI
        #define PI 3.14159265358979323846264338327950288
#endif
#ifndef HALF_PI
    #define HALF_PI (PI*0.5)
#endif

#define EPSLN	1.0e-10

// Normalize an angle to +/-180degrees

#define NORM_ANGLE( x )      while( x >180.0 ) x -= 360.0; while( x < -180.0 ) x += 360.0;
#define NORM_ANGLE_RAD( x )  while( (x) >PI ) (x) -= 2 * PI; while( (x) < -PI ) (x) += 2 * PI;

// Convert degree to radian

#define DEG_TO_RAD( x )		( (x) * 2.0 * PI / 360.0 )

// and reverse

#define RAD_TO_DEG( x )		( (x) * 360.0 / ( 2.0 * PI ) )

// Convert double x to unsigned char/short c



#define	DBL_TO_UC( c, x )	if((x)>255.0) c=255U;								\
								else if((x)<0.0) c=0;							\
								else c=(unsigned char)floor((x)+0.5);

#define	DBL_TO_US( c, x )	if((x)>65535.0) c=65535U;							\
								else if((x)<0.0) c=0;							\
								else c=(unsigned short)floor((x)+0.5);

#define	DBL_TO_FL( c, x )	if((x)>1e+038) c=1e+038;							\
								else if((x)<0.0) c=0;							\
								else c=(float)(x);


#define MAX_FISHEYE_FOV		179.0

extern int JavaUI; // Flag to indicate use of java dialogs
PANO13_IMPEX void JPrintError( char* text );

#define FAST_TRANSFORM_STEP_NORMAL	40
#define FAST_TRANSFORM_STEP_MORPH	6
#define FAST_TRANSFORM_STEP_NONE    0

struct PTPoint
{
	double x;
	double y;
};

typedef struct PTPoint PTPoint;

#define CopyPTPoint( to, from )       memcpy( &to, &from, sizeof( PTPoint ))
#define SamePTPoint( p, s )			  ((p).x == (s).x && (p).y == (s).y)

struct PTLine
{
	PTPoint v[2];
};

typedef struct PTLine PTLine;


struct PTTriangle
{
	PTPoint v[3];
};

typedef struct PTTriangle PTTriangle;




// Maximum number of controlpoints in a pair of images, which can be read
// via Barcodes
#define NUMPTS 21

// Randomization of feather in stitching tools
#define	BLEND_RANDOMIZE		0.1

// Randomization of luminance adjustment in correct filter
#define LUMINANCE_RANDOMIZE 	0.007


//----------------------- Structures -------------------------------------------

struct remap_Prefs{								// Preferences Structure for remap
		int32_t    		magic;					//  File validity check, must be 30
		int				from;					// Image format source image
		int				to;						// Image format destination image
		double			hfov;					// horizontal field of view /in degrees
		double			vfov;					// vertical field of view (usually ignored)
		} ;

typedef struct remap_Prefs rPrefs;

struct perspective_Prefs{						//  Preferences structure for tool perspective
		int32_t			magic;					//  File validity check, must be 40
		int				format;					//  rectilinear or fisheye?
		double  		hfov;					//  Horizontal field of view (in degree)
		double			x_alpha;				//  New viewing direction (x coordinate or angle)
		double 			y_beta;					//  New viewing direction (y coordinate or angle)
		double			gamma;					//  Angle of rotation
		int				unit_is_cart;			//  true, if viewing direction is specified in coordinates
		int				width;					//  new width
		int				height;					//  new height
		} ;
		
typedef struct perspective_Prefs pPrefs;


struct optVars{									//  Indicate to optimizer which variables to optimize
		int hfov;								//  optimize hfov? 0-no 1-yes , etc

  // panotools uses these variables for two purposes: to 
  // determine which variables are used for reference to another one
  // and to determine which variables to optimize

		int yaw;				
		int pitch;				
		int roll;				
		int a;
		int b;
		int c;					
		int d;
		int e;
		int shear_x;
		int shear_y;
		int tiltXopt;
		int tiltYopt;
		int tiltZopt;
		int tiltScaleOpt;
  
		int transXopt;
		int transYopt;
		int transZopt;
        int transYawOpt;
        int transPitchOpt;

		int testP0opt;
		int testP1opt;
		int testP2opt;
		int testP3opt;

		};
		
typedef struct optVars optVars;


enum{										// Enumerates for stBuf.seam
	_middle,								// seam is placed in the middle of the overlap
	_dest									// seam is places at the edge of the image to be inserted
	};

enum{										// Enumerates for colcorrect
	_colCorrectImage 	= 1,
	_colCorrectBuffer	= 2,
	_colCorrectBoth		= 3,
	};

struct stitchBuffer{						// Used describe how images should be merged
	char				srcName[256];		// Buffer should be merged to image; 0 if not.
	char				destName[256];		// Converted image (ie pano) should be saved to buffer; 0 if not
	int				feather;		// Width of feather
	int				colcorrect;		// Should the images be color corrected?
	int				seam;			// Where to put the seam (see above)
        unsigned char                   psdOpacity;               // Opacity of the layer. Currently used only by PSD output. 0 trans, 255 opaque
        unsigned char                   psdBlendingMode;          // blending mode (photoshop)
	};


typedef struct stitchBuffer stBuf;

struct panControls{							// Structure for realtime Panoeditor
		double panAngle;					// The amount by which yaw/pitch are changed per click
		double zoomFactor;					// The percentage for zoom in/out
		};
		
		
typedef struct panControls panControls;



enum{										// Enumerates for aPrefs.mode
		_readControlPoints,
		_runOptimizer,
		_insert,
		_extract,
		_useScript = 8,						// else use options
	};

struct adjust_Prefs{                  //  Preferences structure for tool adjust
    int32_t            magic;        //  File validity check, must be 50
    int32_t            mode;         //  What to do: create Panorama etc?
    Image               im;           //  Image to be inserted/extracted
    Image               pano;         //  Panorama to be created/ used for extraction

    stBuf               sBuf;
    fullPath            scriptFile;   // On Mac: Cast to FSSpec; else: full path to scriptFile
    int                 nt;           // morphing triangles
    PTTriangle         *ts;           // Source triangles
    PTTriangle         *td;           // Destination triangles

    int                 interpolator; // Which interpolator to use 
    double              gamma;        // Gamma correction value
    int                 fastStep;     // 0 no fast Transformation (default), FAST_TRANSFORM_STEP_MORPH, FAST_TRANSFORM_STEP_NORMAL
};
		
		
typedef struct adjust_Prefs aPrefs;
		


union panoPrefs{
		cPrefs	cP;
		pPrefs	pP;
		rPrefs	rP;
		aPrefs	aP;
		panControls pc;
		};
		
typedef union panoPrefs panoPrefs;


struct size_Prefs{                      // Preferences structure for 'pref' dialog
    int32_t        magic;              //  File validity check; must be 70
    int             displayPart;        // Display cropped/framed image ?
    int             saveFile;           // Save to tempfile? 0-no, 1-yes
    fullPath        sFile;              // Full path to file (short name)
    int             launchApp;          // Open sFile ?
    fullPath        lApp;               // the Application to launch
    int             interpolator;       // Which interpolator to use 
    double          gamma;              // Gamma correction value
    int             noAlpha;            // If new file is created: Don't save mask (Photoshop LE)
    int             optCreatePano;      // Optimizer creates panos? 0  no/ 1 yes
    int             fastStep;           // 0 no fast Transformation (default), FAST_TRANSFORM_STEP_MORPH, FAST_TRANSFORM_STEP_NORMAL
} ;

typedef struct size_Prefs sPrefs;



#if 0
struct controlPoint{							// Control Points to adjust images
		int  num[2];							// Indices of Images 
		int	 x[2];								// x - Coordinates 
		int  y[2];								// y - Coordinates 
		int  type;								// What to optimize: 0-r, 1-x, 2-y
		} ;
#endif
struct controlPoint{							// Control Points to adjust images
		int  num[2];							// Indices of Images 
		double x[2];								// x - Coordinates 
		double y[2];								// y - Coordinates 
		int  type;								// What to optimize: 0-r, 1-x, 2-y
		} ;

typedef struct controlPoint controlPoint;

struct CoordInfo{								// Real World 3D coordinates
		int  num;								// auxilliary index
		double x[3];
		int  set[3];
		};
		
typedef struct CoordInfo CoordInfo;

// Some useful macros for vectors

#define SCALAR_PRODUCT( v1, v2 )	( (v1)->x[0]*(v2)->x[0] + (v1)->x[1]*(v2)->x[1] + (v1)->x[2]*(v2)->x[2] ) 
#define ABS_SQUARED( v )			SCALAR_PRODUCT( v, v )
#define ABS_VECTOR( v )				sqrt( ABS_SQUARED( v ) )
#define CROSS_PRODUCT( v1, v2, r )  { (r)->x[0] = (v1)->x[1] * (v2)->x[2] - (v1)->x[2]*(v2)->x[1];  \
									  (r)->x[1] = (v1)->x[2] * (v2)->x[0] - (v1)->x[0]*(v2)->x[2];	\
									  (r)->x[2] = (v1)->x[0] * (v2)->x[1] - (v1)->x[1]*(v2)->x[0]; }
#define DIFF_VECTOR( v1, v2, r )  	{ 	(r)->x[0] = (v1)->x[0] - (v2)->x[0];  \
									  	(r)->x[1] = (v1)->x[1] - (v2)->x[1];  \
									  	(r)->x[2] = (v1)->x[2] - (v2)->x[2]; }
#define DIST_VECTOR( v1, v2 )		sqrt( ((v1)->x[0] - (v2)->x[0]) * ((v1)->x[0] - (v2)->x[0]) + \
										  ((v1)->x[1] - (v2)->x[1]) * ((v1)->x[1] - (v2)->x[1]) + \
										  ((v1)->x[2] - (v2)->x[2]) * ((v1)->x[2] - (v2)->x[2]) )

struct transformCoord{							// 
		int nump;								// Number of p-coordinates
		CoordInfo  *p;							// Coordinates "as is"
		int numr;								// Number of r-coordinates
		CoordInfo  *r;							// Requested values for coordinates
		} ;
	
typedef struct transformCoord transformCoord;

struct  tMatrix{
		double alpha;
		double beta;
		double gamma;
		double x_shift[3];
		double scale;
		};
		
typedef struct tMatrix tMatrix;

		
		
	
	


struct MakeParams{								// Actual parameters used by Xform functions for pano-creation
	double 	scale[2];							// scaling factors for resize;
	double 	shear[2];							// shear values
	double  rot[2];								// horizontal rotation params
	void	*perspect[2];						// Parameters for perspective control functions
	double	rad[6];								// coefficients for polynomial correction (0,...3) and source width/2 (4) and correction radius (5)	
	double	mt[3][3];							// Matrix
	double  distance;
	double	horizontal;
	double	vertical;

    // Tilt 
	double tilt[4]; // 0 around x, 1 around y, 2 around z, 3 scaling factor
    // Translation of camera plane
    double trans[5]; // 0 x, 1 y, 2 z, 3 yaw, 4 pitch
    // For testing new projections
    double test[4];


	Image *im;
	Image *pn;
	};

struct LMStruct{								// Parameters used by the Levenberg Marquardt-Solver
	int			m;								
	int			n;
	double 		*x;
	double 		*fvec;
	double 		ftol;
	double 		xtol;
	double 		gtol;
	int 		maxfev; 
	double 		epsfcn;
	double 		*diag;
	int 		mode;	
	double 		factor;
	int			nprint;
	int			info;
	int			nfev;
	double 		*fjac;
	int			ldfjac;
	int 		*ipvt;
	double 		*qtf;
	double 		*wa1;
	double 		*wa2;
	double 		*wa3;
	double 		*wa4;
	};

// function to minimize in Levenberg-Marquardt solver

typedef		int (*lmfunc)();	

struct triangle
{
	int vert[3];	// Three vertices from list
	int nIm;		// number of image for texture mapping
};

typedef struct triangle triangle;




struct AlignInfo{							// Global data structure used by alignment optimization
	Image 				*im;				// Array of Pointers to Image Structs
	optVars				*opt;				// Mark variables to optimize
	int				numIm;				// Number of images 
	controlPoint 			*cpt;				// List of Control points
	triangle			*t;				// List of triangular faces
	int				nt;				// Number of triangular faces
	int     			numPts;				// Number of Control Points
	int				numParam;			// Number of parameters to optimize
	Image				pano;				// Panoramic Image decription
	stBuf				st;				// Info on how to stitch the panorama
	void				*data;
	lmfunc				fcn;
	sPrefs				sP;	
	CoordInfo			*cim;				// Real World coordinates
	};  

typedef struct AlignInfo AlignInfo;

struct OptInfo{
	int numVars;				// Number of variables to fit
	int numData;				// Number of data to fit to
	int (*SetVarsToX)(double *x);		// Translate variables to x-values
	int (*SetXToVars)(double *x);		// and reverse
	lmfunc fcn;				// Levenberg Marquardt function measuring quality
	char message[256];			// info returned by LM-optimizer
};
	
typedef struct OptInfo OptInfo;



struct VRPanoOptions{
	int		width;
	int		height;
	double 		pan;
	double 		tilt;
	double 		fov;
	int 		codec;
	int 		cquality;
	int			progressive;
};

typedef struct VRPanoOptions VRPanoOptions;


struct MultiLayerImage{
	int	numLayers;
	Image	*Layer;
	PTRect	selection;
};

typedef struct MultiLayerImage MultiLayerImage;


	
PANO13_IMPEX void filter_main( TrformStr *TrPtr, sPrefs *spref);	


// Transformation function type (we have only one...)

typedef         int (*trfn)( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );


// Function descriptor to be executed by exec_function
struct fDesc {
	trfn	func;			// The function to be called
	void	*param;			// The parameters to be used
	};		

typedef struct fDesc fDesc;

#define SetDesc(fD,f,p)		fD.func = f; fD.param = p

// Panorama tool type

typedef		void (*fnPtr)(TrformStr *TrPtr);


// Filter function type

typedef unsigned char (*flfn)( unsigned char srcPixel, int xc, int yc, void *params );
typedef unsigned short (*flfn16)( unsigned short srcPixel, int xc, int yc, void *params );


// Interpolating functions for resampler

typedef		void (*intFunc)( unsigned char *dst, 	unsigned char **rgb,
							register double Dx, 
							register double Dy,
							int color, int SamplesPerPixel);

// Filter function type for anti aliasing Filter

typedef double  (*aaFilter)(const double,const double);

// Gamma Correction

struct PTGamma{
	double *DeGamma;
	unsigned short *Gamma;
	int		ChannelSize;
	int 	ChannelStretch;
	int		GammaSize;
	};

typedef struct PTGamma PTGamma;

extern PTGamma glu;


// Some macros to find out more about images

#define GetBitsPerChannel( im, x )		switch( (im)->bitsPerPixel )	\
									{									\
										case 24:	x =  8; break;		\
										case 32: 	x =  8; break;		\
										case 48: 	x = 16; break;		\
										case 64: 	x = 16; break;		\
										default: 	x =  8; break;		\
									}												

#define GetChannels( im, x )		switch( (im)->bitsPerPixel )		\
									{									\
										case 24:	x =  3; break;		\
										case 32: 	x =  4; break;		\
										case 48: 	x =  3; break;		\
										case 64: 	x =  4; break;		\
										default: 	x =  3; break;		\
									}												

									

//---------------------------------- Functions identical in all platforms ------------------------


PANO13_IMPEX void 	dispatch 	(TrformStr *TrPtr, sPrefs *s);	   // Entry into platform independent code
PANO13_IMPEX void 	DoTransForm	(TrformStr *TrPtr, panoPrefs *p );

PANO13_IMPEX void setLibToResFile  ( void );			// MacOS: Get resources from shared lib
PANO13_IMPEX void unsetLibToResFile( void );			// MacOS: Don't get resources from shared lib

enum{					// Enumerates used by Progress and infoDlg
	_initProgress,   	// display message "argument"
	_setProgress,		// display progress (argument is percentage converted to string)
	_disposeProgress,	// dispose progress indicator
	_idleProgress		// do nothing; on Mac: call waitnextevent;
	};

PANO13_IMPEX void PT_setProgressFcn(int (*ptr)(int, char *));           // set custom progress callback
PANO13_IMPEX int 	Progress( int command, char* argument );	// Progress Reporting 
PANO13_IMPEX void PT_setInfoDlgFcn(int (*ptr)(int, char *));            // set custom info callback
PANO13_IMPEX int 	infoDlg ( int command, char* argument );	// Display info: same argumenmts as progress
PANO13_IMPEX void PT_setErrorFcn( void (*ptr)( char* , va_list va));         // set custom error function
PANO13_IMPEX void  	PrintError( char* fmt, ...);				// Error Reporting
PANO13_IMPEX void dieWithError(char*fmt, ...);

PANO13_IMPEX int 	ccommand( char ***argvPtr);					// Shell for standalone programs


//  Panorama Tool functions


PANO13_IMPEX void 	perspective	(TrformStr *TrPtr, pPrefs *p); 	
PANO13_IMPEX void 	correct		(TrformStr *TrPtr, cPrefs *c);  
PANO13_IMPEX void 	remap		(TrformStr *TrPtr, rPrefs *r); 
PANO13_IMPEX void 	adjust		(TrformStr *TrPtr, aPrefs *a); 
PANO13_IMPEX void 	pan			(TrformStr *TrPtr, panControls *pc);

// Set Struct defaults

PANO13_IMPEX void    SetPrefDefaults			(panoPrefs *prPtr,  int selector);
PANO13_IMPEX void 	SetCorrectDefaults		( cPrefs *p );
PANO13_IMPEX void 	SetAdjustDefaults		( aPrefs *p );
PANO13_IMPEX void 	SetRemapDefaults		( rPrefs *p );
PANO13_IMPEX void 	SetPerspectiveDefaults	( pPrefs *p );
PANO13_IMPEX void 	SetImageDefaults		( Image *im);
PANO13_IMPEX void	SetOptDefaults			( optVars *opt );
PANO13_IMPEX void	SetPanDefaults			( panControls *pc);
PANO13_IMPEX void 	SetSizeDefaults			( sPrefs *pref);
PANO13_IMPEX void	SetStitchDefaults		( stBuf *sbuf);
PANO13_IMPEX void	SetVRPanoOptionsDefaults( VRPanoOptions *v);
PANO13_IMPEX void 	SettMatrixDefaults		( tMatrix *t );
PANO13_IMPEX void 	SetCoordDefaults		( CoordInfo *c, int num);

PANO13_IMPEX int    SetAlignParams			( double *x );
PANO13_IMPEX int 	SetLMParams				( double *x );
PANO13_IMPEX void 	SetGlobalPtr			( AlignInfo *p );

PANO13_IMPEX int    CheckParams( AlignInfo *g );
PANO13_IMPEX void   setFcnPanoHuberSigma(double sigma);


// Dialogs
PANO13_IMPEX int 	SetPrefs		( panoPrefs *p );
PANO13_IMPEX int	SetPanPrefs		( panControls *p );
PANO13_IMPEX int 	SetCorrectPrefs		( cPrefs *p );
PANO13_IMPEX int 	SetRadialOptions	( cPrefs *p );
PANO13_IMPEX int 	SetHorizontalOptions	( cPrefs *p );
PANO13_IMPEX int 	SetVerticalOptions	( cPrefs *p );
PANO13_IMPEX int 	SetShearOptions		( cPrefs *p );
PANO13_IMPEX int 	SetScaleOptions		( cPrefs *p );
PANO13_IMPEX int 	SetLumOptions		( cPrefs *p );
PANO13_IMPEX int 	setSizePrefs		( sPrefs *p, int can_resize );
PANO13_IMPEX int 	SetRemapPrefs		( rPrefs *p );
PANO13_IMPEX int 	SetPerspectivePrefs	( pPrefs *p );
PANO13_IMPEX int 	SetAdjustPrefs		( aPrefs *p );
PANO13_IMPEX int 	SetInterpolator		( sPrefs *p );
PANO13_IMPEX int 	SetCreateOptions	( aPrefs *p );
PANO13_IMPEX int 	SetCutOptions		( cPrefs *p );
PANO13_IMPEX int 	SetFourierOptions	( cPrefs *p );



// File I/O

PANO13_IMPEX int 	readPrefs			(char* p, int selector );   			// Preferences, same selector as dispatch
PANO13_IMPEX void 	writePrefs			(char* p, int selector );   			// Preferences, same selector as dispatch

PANO13_IMPEX int	LoadBufImage		( Image *image, char *fname, int mode);
PANO13_IMPEX int	SaveBufImage		( Image *image, char *fname );
PANO13_IMPEX int writeCroppedTIFF    ( Image *im, fullPath *sfile, CropInfo *crop_info);
PANO13_IMPEX int	writeTIFF			( Image *im, fullPath *fname);			// On Mac: fname is FSSpec*				
PANO13_IMPEX void 	SaveOptions			( struct correct_Prefs * thePrefs );
PANO13_IMPEX int 	LoadOptions			( struct correct_Prefs * thePrefs );
PANO13_IMPEX void  	FindScript			( struct adjust_Prefs *thePrefs );
PANO13_IMPEX char* 	LoadScript			( fullPath* scriptFile  );
PANO13_IMPEX int 	WriteScript			( char* res, fullPath* scriptFile, int launch );
// Write PSB and PSD files
PANO13_IMPEX int 	writePS 			( Image *im, fullPath* fname, Boolean bBig );			// On Mac: fname is FSSpec*	
PANO13_IMPEX int 	writePSD			( Image *im, fullPath* fname );
PANO13_IMPEX int 	readPSD				( Image *im, fullPath* fname, int mode); // Can handle both PSD and PSB
PANO13_IMPEX int 	writePSwithLayer	( Image *im, fullPath *fname, Boolean bBig);
PANO13_IMPEX int 	writePSDwithLayer	( Image *im, fullPath *fname);
PANO13_IMPEX int 	addLayerToFile		( Image *im, fullPath* sfile, fullPath* dfile, stBuf *sB); //works with PSD & PSB
PANO13_IMPEX int 	readPSDMultiLayerImage( MultiLayerImage *mim, fullPath* sfile);
PANO13_IMPEX int 	FindFile			( fullPath *fname );
PANO13_IMPEX int 	SaveFileAs			( fullPath *fname, char *prompt, char *name );
PANO13_IMPEX void 	ConvFileName		( fullPath *fname,char *string);
PANO13_IMPEX void 	showScript			( fullPath* scriptFile );
PANO13_IMPEX void 	MakeTempName		( fullPath *fspec, char *fname );
PANO13_IMPEX void 	makePathForResult	( fullPath *path );
PANO13_IMPEX int 	makePathToHost 		( fullPath *path );
PANO13_IMPEX void    open_selection		( fullPath *path );
PANO13_IMPEX int 	GetFullPath 		(fullPath *path, char *filename); // Somewhat confusing, for compatibility easons
PANO13_IMPEX int 	StringtoFullPath	(fullPath *path, char *filename);
PANO13_IMPEX int 	IsTextFile			( char* fname );
PANO13_IMPEX int 	readPositions		( char* script, transformCoord *tP );
PANO13_IMPEX int	readJPEG			( Image *im, fullPath *sfile );
PANO13_IMPEX int	readTIFF			( Image *im, fullPath *sfile );
PANO13_IMPEX int 	writeJPEG			( Image *im, fullPath *sfile, 	int quality, int progressive );
PANO13_IMPEX int 	writePNG			( Image *im, fullPath *sfile );
PANO13_IMPEX int 	readPNG				( Image *im, fullPath *sfile );
PANO13_IMPEX int 	LaunchAndSendScript(char* application, char* script);
PANO13_IMPEX aPrefs* readAdjustLine( fullPath *theScript );

#ifdef __Mac__

 int 	readImage			( Image *im, fullPath *sfile );
 int 	writeImage			( Image *im, fullPath *sfile );
 int 	makeTempPath		( fullPath *path );
#endif

//int	readtif(Image *im, TIFF* tif);
PANO13_IMPEX void getCropInformation(char *filename, CropInfo *c);

// Read and Write Radiance HDR files
PANO13_IMPEX int 	writeHDR			( Image *im, fullPath *sfile );
PANO13_IMPEX int 	readHDR				( Image *im, fullPath *sfile );

#define FullPathtoString( path, string ) 		GetFullPath( path, string)


PANO13_IMPEX int		ReadMorphPoints( char *script, AlignInfo *gl, int nIm );

// Image manipulation

PANO13_IMPEX void 	addAlpha			( Image *im ); 
PANO13_IMPEX void 	transForm			( TrformStr *TrPtr, fDesc *fD, int color);
PANO13_IMPEX void 	transFormEx			( TrformStr *TrPtr, fDesc *fD, fDesc *finvD, int color, int imageNum);
PANO13_IMPEX void    filter				( TrformStr *TrPtr, flfn func, flfn16 func16, void* params, int color);		
PANO13_IMPEX void 	CopyImageData		( Image *dest, Image *src );
PANO13_IMPEX void 	laplace				( Image *im );
PANO13_IMPEX void 	blurr				( Image *im );
PANO13_IMPEX void 	MakePano			( TrformStr *TrPtr, aPrefs *aP);
PANO13_IMPEX void	MyMakePano			( TrformStr *TrPtr, aPrefs *aP, int imageNum );
PANO13_IMPEX void 	ExtractStill		( TrformStr *TrPtr , aPrefs *p );
PANO13_IMPEX int 	HaveEqualSize		( Image *im1, Image *im2 );
PANO13_IMPEX int 	merge				( Image *dst, Image *src, int feather, int showprogress, int seam );
PANO13_IMPEX void 	mergeAlpha			( Image *im, unsigned char *alpha, int feather, PTRect *theRect );
PANO13_IMPEX void 	SetEquColor			( cPrefs *p );
PANO13_IMPEX void 	CopyPosition		( Image *to, Image *from );
PANO13_IMPEX int  	isColorSpecific		( cPrefs *p );
PANO13_IMPEX void 	ThreeToFourBPP		( Image *im );
PANO13_IMPEX void 	FourToThreeBPP		( Image *im );
PANO13_IMPEX int 	SetUpGamma			( double pgamma, unsigned int psize);
PANO13_IMPEX int 	cutTheFrame			( Image *dest, Image *src, int width, int height, int showprogress );
PANO13_IMPEX int 	PositionCmp			( Image *im1, Image *im2 );
PANO13_IMPEX int 	MorphImage			( Image *src, Image *dst, PTTriangle *ts, PTTriangle *td, int nt );
PANO13_IMPEX int 	MorphImageFile		( fullPath *sfile, fullPath *dfile, AlignInfo *g,int nIm );
PANO13_IMPEX int 	blendImages			( fullPath *f0,  fullPath *f1, fullPath *result, double s );
PANO13_IMPEX int 	InterpolateImage	( Image *src, Image *dst, PTTriangle *ts, PTTriangle *td, int nt );
PANO13_IMPEX int 	InterpolateTrianglesPerspective( AlignInfo *g, int nIm, double s, PTTriangle** t  );
PANO13_IMPEX int 	InterpolateImageFile( fullPath *sfile, fullPath *dfile, AlignInfo *g,int nIm );
PANO13_IMPEX void 	OneToTwoByte		( Image *im );
PANO13_IMPEX void 	TwoToOneByte		( Image *im );
PANO13_IMPEX void 	SetMakeParams           ( struct fDesc *stack, struct MakeParams *mp, Image *im , Image *pn, int color );
PANO13_IMPEX void 	SetInvMakeParams	( struct fDesc *stack, struct MakeParams *mp, Image *im , Image *pn, int color );
// same as SetInvMakeParams but includes Joosts inverted changes to SetMakeParams
PANO13_IMPEX void 	SetInvMakeParamsCorrect( struct fDesc *stack, struct MakeParams *mp, Image *im , Image *pn, int color );

PANO13_IMPEX void 	GetControlPointCoordinates(int i, double *x, double *y, AlignInfo *gl );
PANO13_IMPEX void 	ARGBtoRGBA(uint8_t* buf, int width, int bitsPerPixel);
PANO13_IMPEX void 	RGBAtoARGB(uint8_t* buf, int width, int bitsPerPixel);
PANO13_IMPEX int 	CropImage(Image *im, PTRect *r);
PANO13_IMPEX void 	DoColorCorrection( Image *im1, Image *im2, int mode );

// Script Reading/Parsing/Writing

PANO13_IMPEX int 	ParseScript			( char* script, AlignInfo *gl );
PANO13_IMPEX void 	WriteResults		( char* script, fullPath *sfile, AlignInfo *g, double ds( int i) , int launch);
PANO13_IMPEX int 	readAdjust		( aPrefs *p,  fullPath* sfile, int insert, sPrefs *sP );
PANO13_IMPEX void 	readControlPoints	(char* script, controlPoint *c );
PANO13_IMPEX int	getVRPanoOptions	( VRPanoOptions *v, char *line );
PANO13_IMPEX void 	nextWord			( register char* word, char** ch );
PANO13_IMPEX void 	nextLine			( register char* line, char** ch );
PANO13_IMPEX int 	numLines			( char* script, char first );

PANO13_IMPEX char *panoParserFindOLine(char *script, int index);




// Memory

PANO13_IMPEX void 	DisposeAlignInfo	( AlignInfo *g );
PANO13_IMPEX void**  mymalloc			( size_t numBytes );					// Memory allocation, use Handles
PANO13_IMPEX void 	myfree				( void** Hdl );						// free Memory, use Handles
PANO13_IMPEX int 	SetDestImage		( TrformStr *TrPtr, int width, int height) ;
PANO13_IMPEX void	DisposeMultiLayerImage( MultiLayerImage *mim );


// Math

PANO13_IMPEX void 	RunLMOptimizer		( OptInfo	*g);
PANO13_IMPEX void 	RunBROptimizer		( OptInfo	*g, double minStepWidth);
PANO13_IMPEX void 	RunOverlapOptimizer ( AlignInfo	*g);

PANO13_IMPEX void 	SetMatrix			( double a, double b, double c , double m[3][3], int cl );
PANO13_IMPEX void 	matrix_mult			( double m[3][3], double vector[3] );
PANO13_IMPEX void 	matrix_inv_mult		( double m[3][3], double vector[3] );
PANO13_IMPEX double 	smallestRoot		( double *p );
PANO13_IMPEX void 	SetCorrectionRadius	( cPrefs *cP );
PANO13_IMPEX int		lmdif				();
PANO13_IMPEX void	fourier				( TrformStr *TrPtr, cPrefs *cP );
PANO13_IMPEX unsigned short 	gamma_correct( double pix );
PANO13_IMPEX int 	EqualCPrefs( cPrefs *c1, cPrefs *c2 );
PANO13_IMPEX double 	OverlapRMS			( MultiLayerImage *mim );
PANO13_IMPEX double 	distSquared			( int num ); 
PANO13_IMPEX int		fcnPano();
PANO13_IMPEX int		EvaluateControlPointError ( int num, double *errptr, double errComponent[2]);
PANO13_IMPEX void 	doCoordinateTransform( CoordInfo *c, tMatrix *t );
PANO13_IMPEX void 	findOptimumtMatrix( transformCoord *tP, tMatrix *tM, lmfunc f);
PANO13_IMPEX int 	SolveLinearEquation2( double a[2][2], double b[2], double x[2] );
PANO13_IMPEX void 	SortControlPoints( AlignInfo *g , int nIm);
PANO13_IMPEX void 	noisefilter			( Image *dest, Image *src );	
PANO13_IMPEX void 	fwiener				( TrformStr *TrPtr, Image *nf, Image *psf, double gamma, double frame );


// Triangulation
PANO13_IMPEX int 	PointInTriangle( double x, double y, PTTriangle *T, double c[2] );
PANO13_IMPEX int 	SetSourceTriangles( AlignInfo *g, int nIm, PTTriangle** t  );
PANO13_IMPEX int 	SetDestTriangles( AlignInfo *g, int nIm, PTTriangle** t  );
PANO13_IMPEX int 	InterpolateTriangles( AlignInfo *g, int nIm, double s, PTTriangle** t  );
PANO13_IMPEX int 	DelaunayIteration( AlignInfo *g, int nIm );
PANO13_IMPEX int 	PointInCircumcircle( double x, double y, PTTriangle *tC );
PANO13_IMPEX int 	TriangulatePoints( AlignInfo *g, int nIm );
PANO13_IMPEX int 	AddTriangle( triangle *t, AlignInfo *g );
PANO13_IMPEX int 	RemoveTriangle( int nt, AlignInfo *g );
PANO13_IMPEX void 	OrderVerticesInTriangle( int nt, AlignInfo *g );
PANO13_IMPEX void 	SetTriangleCoordinates( triangle *t, PTTriangle *tC, AlignInfo *g );
PANO13_IMPEX int 	TrianglesOverlap( PTTriangle *t0, PTTriangle *t1 );
PANO13_IMPEX int 	LinesIntersect( PTLine *s0, PTLine *s1) ; 
PANO13_IMPEX double 	PTDistance( PTPoint *s0, PTPoint *s1 );
PANO13_IMPEX int 	PTPointInRectangle(  PTPoint *p, PTLine *r );
PANO13_IMPEX int 	PTElementOf(  double x, double a, double b );
PANO13_IMPEX int 	PTNormal( double *a, double *b, double *c, PTLine *s );
PANO13_IMPEX int 	PTGetLineCrossing( PTLine *s0, PTLine *s1, PTPoint *ps );
PANO13_IMPEX int 	ReduceTriangles( AlignInfo *g, int nIm );
PANO13_IMPEX double 	PTAreaOfTriangle( PTTriangle *t );
PANO13_IMPEX int 	normalToTriangle( CoordInfo *n, CoordInfo *v, triangle *t );




PANO13_IMPEX double GetBlendfactor( int d, int s, int feather );





PANO13_IMPEX void execute_stack		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	

PANO13_IMPEX int execute_stack_new               ( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );   

PANO13_IMPEX int resize				( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );		
PANO13_IMPEX int shear				( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int shearInv				( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int horiz				( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int vert				( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int radial				( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int radial_brown			( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	

PANO13_IMPEX int tiltForward				( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int tiltInverse				( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	

PANO13_IMPEX int persp_sphere		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int persp_rect			( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	


PANO13_IMPEX int rect_pano			( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int pano_rect			( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int pano_erect			( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int erect_pano			( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int sphere_cp_erect	( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int sphere_tp_erect	( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int erect_sphere_cp	( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int rect_sphere_tp		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int sphere_tp_rect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int sphere_cp_pano		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int rect_erect			( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int erect_rect			( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int plane_transfer_to_camera( double x_dest, double y_dest, double * x_src, double * y_src, void * params);
PANO13_IMPEX int plane_transfer_from_camera( double x_dest, double y_dest, double * x_src, double * y_src, void * params);
PANO13_IMPEX int erect_sphere_tp	( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int mirror_erect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int mercator_erect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int erect_mercator		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int lambert_erect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int erect_lambert		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int erect_lambertazimuthal( double x_dest,double  y_dest, double* x_src, double* y_src, void* params);
PANO13_IMPEX int lambertazimuthal_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params);
PANO13_IMPEX int erect_hammer( double x_dest,double  y_dest, double* x_src, double* y_src, void* params);
PANO13_IMPEX int hammer_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params);
PANO13_IMPEX int transmercator_erect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int erect_transmercator		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int sinusoidal_erect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int erect_sinusoidal		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int stereographic_erect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int erect_stereographic		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int albersequalareaconic_erect	( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int erect_albersequalareaconic	( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int albersequalareaconic_distance	( double *x_src, void* params );
PANO13_IMPEX int millercylindrical_erect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int erect_millercylindrical		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int panini_erect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int erect_panini		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int equipanini_erect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int erect_equipanini		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );

PANO13_IMPEX int panini_general_erect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int erect_panini_general		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX Image * setup_panini_general	( struct MakeParams * pmp );
PANO13_IMPEX int maxFOVs_panini_general	( double *params, double *fovs );

PANO13_IMPEX int arch_erect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int erect_arch		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );

PANO13_IMPEX int biplane_erect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int erect_biplane		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int biplane_distance ( double width, double b, void* params );
PANO13_IMPEX int triplane_erect		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int erect_triplane		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int triplane_distance ( double width, double b, void* params );

PANO13_IMPEX int mirror_sphere_cp	( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int mirror_pano		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	
PANO13_IMPEX int sphere_cp_mirror	( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );	

PANO13_IMPEX int sphere_tp_pano		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int pano_sphere_tp		( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int sphere_tp_mirror( double x_dest,double  y_dest, double* x_src, double* y_src, void* params);
PANO13_IMPEX int mirror_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params);
PANO13_IMPEX int sphere_tp_equisolid( double x_dest,double  y_dest, double* x_src, double* y_src, void* params);
PANO13_IMPEX int equisolid_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params);
PANO13_IMPEX int sphere_tp_orthographic( double x_dest,double  y_dest, double* x_src, double* y_src, void* params);
PANO13_IMPEX int orthographic_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params);

PANO13_IMPEX int sphere_tp_thoby( double x_dest,double  y_dest, double* x_src, double* y_src, void* params);
PANO13_IMPEX int thoby_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params);



#define THOBY_K1_PARM 1.47
#define THOBY_K2_PARM 0.713


PANO13_IMPEX int rotate_erect		( double x_dest, double y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int inv_radial			( double x_dest, double y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int inv_radial_brown		( double x_dest, double y_dest, double* x_src, double* y_src, void* params );

PANO13_IMPEX int vertical			( double x_dest, double y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int inv_vertical		( double x_dest, double y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int deregister			( double x_dest, double y_dest, double* x_src, double* y_src, void* params );
PANO13_IMPEX int tmorph				( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );

PANO13_IMPEX int shift_scale_rotate ( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );



unsigned char radlum            ( unsigned char srcPixel, int xc, int yc, void *params );
//Kekus 16 bit 2003/Nov/18
unsigned short radlum16         ( unsigned short srcPixel, int xc, int yc, void *params );// , long bitsPerComponent);
//Kekus.

extern TrformStr 		*gTrPtr;
extern sPrefs			*gsPrPtr;




// Endian stuff: Read and write numbers from and to memory (ptr)

#ifdef PT_BIGENDIAN
#define	LONGLONGNUMBER( number, ptr )		*ptr++ = ((char*)(&number))[0];	\
														*ptr++ = ((char*)(&number))[1];	\
														*ptr++ = ((char*)(&number))[2];	\
														*ptr++ = ((char*)(&number))[3];	\
														*ptr++ = ((char*)(&number))[4];	\
														*ptr++ = ((char*)(&number))[5];	\
														*ptr++ = ((char*)(&number))[6];	\
														*ptr++ = ((char*)(&number))[7];	

	#define NUMBERLONGLONG( number, ptr )		((char*)(&number))[0] = *ptr++;	\
														((char*)(&number))[1] = *ptr++;	\
														((char*)(&number))[2] = *ptr++;	\
														((char*)(&number))[3] = *ptr++;	\
														((char*)(&number))[4] = *ptr++;	\
														((char*)(&number))[5] = *ptr++;	\
														((char*)(&number))[6] = *ptr++;	\
														((char*)(&number))[7] = *ptr++;	

#define	LONGNUMBER( number, ptr )					*ptr++ = ((char*)(&number))[0];	\
														*ptr++ = ((char*)(&number))[1];	\
														*ptr++ = ((char*)(&number))[2];	\
														*ptr++ = ((char*)(&number))[3];	

	#define NUMBERLONG( number, ptr )					((char*)(&number))[0] = *ptr++;	\
														((char*)(&number))[1] = *ptr++;	\
														((char*)(&number))[2] = *ptr++;	\
														((char*)(&number))[3] = *ptr++;	

	#define	SHORTNUMBER( number, ptr )					*ptr++ = ((char*)(&number))[0];	\
														*ptr++ = ((char*)(&number))[1];	\

	#define NUMBERSHORT( number, ptr )					((char*)(&number))[0] = *ptr++;	\
														((char*)(&number))[1] = *ptr++;	\

#else
	#define	LONGLONGNUMBER( number, ptr )		*ptr++ = ((char*)(&number))[7];	\
														*ptr++ = ((char*)(&number))[6];	\
														*ptr++ = ((char*)(&number))[5];	\
														*ptr++ = ((char*)(&number))[4];	\
														*ptr++ = ((char*)(&number))[3];	\
														*ptr++ = ((char*)(&number))[2];	\
														*ptr++ = ((char*)(&number))[1];	\
														*ptr++ = ((char*)(&number))[0];	

	#define NUMBERLONGLONG( number, ptr )		((char*)(&number))[7] = *ptr++;	\
														((char*)(&number))[6] = *ptr++;	\
														((char*)(&number))[5] = *ptr++;	\
														((char*)(&number))[4] = *ptr++;	\
														((char*)(&number))[3] = *ptr++;	\
														((char*)(&number))[2] = *ptr++;	\
														((char*)(&number))[1] = *ptr++;	\
														((char*)(&number))[0] = *ptr++;	

	#define	LONGNUMBER( number, ptr )					*ptr++ = ((char*)(&number))[3];	\
														*ptr++ = ((char*)(&number))[2];	\
														*ptr++ = ((char*)(&number))[1];	\
														*ptr++ = ((char*)(&number))[0];	

	#define NUMBERLONG( number, ptr )					((char*)(&number))[3] = *ptr++;	\
														((char*)(&number))[2] = *ptr++;	\
														((char*)(&number))[1] = *ptr++;	\
														((char*)(&number))[0] = *ptr++;	

#define	SHORTNUMBER( number, ptr )					*ptr++ = ((char*)(&number))[1];	\
														*ptr++ = ((char*)(&number))[0];	\

	#define NUMBERSHORT( number, ptr )					((char*)(&number))[1] = *ptr++;	\
														((char*)(&number))[0] = *ptr++;	\



#endif // PT_BIGENDIAN

//TODO:  JMW These File i/o macros are to be replaced in code with the error catching functions below
#if USEWRITEREADMACROS
#define WRITEUCHAR( theChar )       ch = theChar; count = 1; mywrite(fnum,count,&ch);

#define WRITESHORT( theShort )      svar = theShort; d = data; SHORTNUMBER( svar, d ); \
                                    count = 2; mywrite  (fnum,count,data);

#define WRITEINT32( theLong )       var = theLong; d = data; LONGNUMBER( var, d ); \
                                    count = 4; mywrite  (fnum,count,data);

#define READINT32( theLong )                count = 4; myread(src,count,data);  \
                                            d = data; NUMBERLONG( var, d );     \
                                            theLong = var;
                                    
#define WRITEINT64( theLong )       var64 = theLong; d = data; LONGLONGNUMBER( var64, d ); \
                                    count = 8; mywrite  (fnum,count,data);

#define READINT64( theLong )                count = 8; myread(src,count,data);  \
                                            d = data; NUMBERLONGLONG( var64, d );     \
                                            theLong = var64;
                                    
#define READSHORT( theShort )               count = 2; myread(src,count,data);  \
                                            d = data; NUMBERSHORT( svar, d );   \
                                            theShort = svar;

#define READUCHAR( theChar )                count = 1; myread(src,count,&ch); theChar = ch; 
#endif
// Cross platform file functions

#ifdef __Mac__

	#include <Carbon/Carbon.h> // Kekus Digital<Files.h>
	#include "sys_mac.h"
	
	#define			file_spec							short
	#define			nfile_spec							short
	#define			myopen( path, perm, fspec )			( FSpOpenDF( path, perm, &fspec ) != noErr )
	#define			mywrite( fspec, count, data )		FSWrite	(fspec, &count, data) 
	#define 		myread(  fspec, count, data )		FSRead  (fspec, &count, data) 
	#define         myclose( fspec )					FSClose (fspec )
	#define			mycreate( path, creator, type )		FSpCreate( path, creator, type,0)
	#define			mydelete( path )					FSpDelete( path )
	#define			myrename( path, newpath )			FSpRename (path, (newpath)->name)
	#define			write_text							fsWrPerm
	#define			write_bin							fsWrPerm
	#define			read_text							fsRdPerm
	#define			read_bin							fsRdPerm
	#define			read_write_text						fsRdWrPerm
			
#else // __Mac__, use ANSI-filefunctions
	#define		file_spec			FILE*
	#define		nfile_spec			int
	#define		myopen( path, perm, fspec )	( (fspec = fopen( (path)->name, perm )) == NULL)
	#define		mywrite( fspec, count, data )	count = fwrite( data, 1, count, fspec)
	#define 	myread( fspec, count, data )	count = fread( data, 1, count, fspec ) 
	#define         myclose( fspec )		fclose (fspec )
	#define		mycreate( path, creator, type )		
	#define		mydelete( path )		remove((path)->name )
	#define		myrename( path, newpath )	rename ((path)->name, (newpath)->name)
	#define		write_text			"w"
	#define		write_bin			"wb"
	#define		read_text			"r"
	#define		read_bin			"rb"
	#define		read_write_text			"rw"
	#define		append_bin			"ab"
    
    // Ippei hack. OSX with GCC+ANSI mode.
    #ifdef MAC_OS_X_VERSION_10_4
        // MacOSX 10.4 has those functions predefined in Carbon API.
        #include <Carbon/Carbon.h> // CoreServices/TextUtils.h
    #else
        #define		p2cstr( x )	
        #define		c2pstr( x )
    #endif

#endif

/* ENDIAN aware file i/o funtions.  Used for reading and writing photoshop files */
PANO13_IMPEX Boolean panoWriteUCHAR(file_spec fnum, uint8_t   theChar );
PANO13_IMPEX Boolean panoWriteSHORT(file_spec fnum, uint16_t  theShort );
PANO13_IMPEX Boolean panoWriteINT32(file_spec fnum, uint32_t   theLong );
PANO13_IMPEX Boolean panoWriteINT64(file_spec fnum, int64_t theLongLong );
PANO13_IMPEX Boolean panoWriteINT32or64(file_spec fnum, int64_t theLongLong, Boolean bBig );
PANO13_IMPEX Boolean panoReadUCHAR (file_spec fnum, uint8_t  *pChar );
PANO13_IMPEX Boolean panoReadSHORT (file_spec fnum, uint16_t *pShort );
PANO13_IMPEX Boolean panoReadINT32 (file_spec fnum, uint32_t  *pLong );
PANO13_IMPEX Boolean panoReadINT64 (file_spec fnum, int64_t  *pLongLong );
PANO13_IMPEX Boolean panoReadINT32or64(file_spec fnum, int64_t  *pLongLong, Boolean bBig );


#define PANO_DEFAULT_PIXELS_PER_RESOLUTION  150.0
#define PANO_DEFAULT_TIFF_RESOLUTION_UNITS  RESUNIT_INCH
// this is the best compression available in all systems
// better than PACKBITS
#define PANO_DEFAULT_TIFF_COMPRESSION       COMPRESSION_DEFLATE

PANO13_IMPEX void panoMetadataFree(pano_ImageMetadata * metadata);
PANO13_IMPEX int panoMetadataCopy(pano_ImageMetadata * to, pano_ImageMetadata * from);
PANO13_IMPEX int panoROIRowInside(pano_CropInfo * cropInfo, int row);
PANO13_IMPEX void panoMetadataSetCompression(pano_ImageMetadata * metadata, char *compressionName);
PANO13_IMPEX int panoMetadataCopy(pano_ImageMetadata * to, pano_ImageMetadata * from);
PANO13_IMPEX void panoMetadataFree(pano_ImageMetadata * metadata);

PANO13_IMPEX void panoMetadataSetAsCropped(pano_ImageMetadata * metadata, 
			      int croppedWidth, 
			      int croppedHeight,
			      int roiLeft, 
			      int roiRight);

PANO13_IMPEX void panoMetadataResetSize(pano_ImageMetadata * metadata, 
			   int width, int height);

PANO13_IMPEX int panoReadJPEG(Image * im, fullPath * sfile);

#ifndef PAN_DEBUG_METADATA
PANO13_IMPEX void panoDumpMetadata(pano_ImageMetadata * metadata, char *message);
#else 
#define panoDumpMetadata(a,b)  ;
#endif


#endif


// number of different temporary files
#define MAX_TEMP_TRY    1000000


