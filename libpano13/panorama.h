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



#ifndef PANORAMA_H
#define PANORAMA_H

#if defined _WIN32 && defined PANO13_DLL
#if defined pano13_EXPORTS
#define PANO13_IMPEX __declspec(dllexport)
#else
#define PANO13_IMPEX __declspec(dllimport)
#endif
#else
#define PANO13_IMPEX
#endif

#include "pt_stdint.h"
#include "version.h"
#include "panotypes.h"

// MRDL: Replaced BIGENDIAN with PT_BIGENDIAN to eliminate conflict with 
// BIGENDIAN defined in winsock2.h distributed with MingW 2.0

// Determine which machine we are using. Macs are set to PT_BIGENDIAN, all others not

// If you need PT_BIGENDIAN, and don't use MacOS, define it here:
//#define PT_BIGENDIAN                  1

typedef unsigned char Boolean;

// Create a definition if we're on a Windows machine:
#ifndef __Ansi__
  #if (defined(MSDOS) || defined(_CONSOLE) || defined(__DOS__) || defined(__MSDOS__))
    #define __Ansi__      1
  #endif
#endif

#ifndef __Win__
  #if (defined(_WINDOWS) || defined(WINDOWS))
    #define __Win__     1
  #endif
#endif

#ifndef __Mac_OSX__
    #if defined(__APPLE_CC__)
        #define __Mac_OSX__			1
        #if (defined(__ppc__) || defined(__ppc64__))
            #define PT_BIGENDIAN		1
        #elif defined(__i386__)
            #undef PT_BIGENDIAN
        #endif
    #endif
#endif

// Use FSSpec on Macs as Path-specifyers, else strings
#define PATH_SEP							'/'

#ifdef _WIN32
    #ifndef __NO_SYSTEM__
        #include <windows.h>            // including this causes problems with libjpeg
    #endif

    #define MAX_PATH_LENGTH		260
                // was MAX_PATH
    #undef  PATH_SEP
    #define PATH_SEP							'\\'

#else
    #define MAX_PATH_LENGTH		512
#endif

// I really want to get rid of this one. it is legacy from OS 9
typedef struct
{
    char name[MAX_PATH_LENGTH];
} fullPath;


// Some important defaults (perhaps to be moved somewhere else later

// Enumerates for TrFormStr.tool

enum
{                               // Panorama Tools
    _perspective,
    _correct,
    _remap,
    _adjust,
    _interpolate,
    _sizep,                     // dummy for size-preferences
    _version,                   // dummy for version
    _panright,                  // Pan Controls
    _panleft,
    _panup,
    _pandown,
    _zoomin,
    _zoomout,
    _apply,
    _getPano,
    _increment
};

// Enumerates for TrFormStr.mode

enum
{                               // Modes
    _interactive,               // display dialogs and do Xform
    _useprefs,                  // load saved prefs and do Xform/ no dialogs
    _setprefs,                  // display dialogs and set preferences, no Xform        
    _usedata,                   // use supplied data in TrFormStr.data, do Xform
    _honor_valid = 8,           // Use only pixels with alpha channel set
    _show_progress = 16,        // Interpolator displays progress bar
    _hostCanResize = 32,        // o-no; 1-yes (Photoshop: no; GraphicConverter: yes)
    _destSupplied = 64,         // Destination image allocated by plug-in host
    _wrapX = 128                // Wrap image horizontally (if HFOV==360 degrees)
};


// Enumerates for Image.dataformat

enum
{
    _RGB,
    _Lab,
    _Grey
};

// Enumerates for TrFormStr.interpolator

enum
{                               // Interpolators
    _poly3 = 0,                 // Third order polynomial fitting 16 nearest pixels
    _spline16 = 1,              // Cubic Spline fitting 16 nearest pixels
    _spline36 = 2,              // Cubic Spline fitting 36 nearest pixels
    _sinc256 = 3,               // Sinc windowed to 8 pixels
    _spline64,                  // Cubic Spline fitting 64 nearest pixels
    _bilinear,                  // Bilinear interpolation
    _nn,                        // Nearest neighbor
    _sinc1024,
    // Thomas Rauscher: New antialiasing filter. 
    // Plots of the functions are available at http://www.pano2qtvr.com/dll_patch/
    _aabox,                     // Antialiasing: Box
    _aatriangle,                // Antialiasing: Bartlett/Triangle Filter
    _aahermite,                 // Antialiasing: Hermite Filter
    _aahanning,                 // Antialiasing: Hanning Filter
    _aahamming,                 // Antialiasing: Hamming Filter
    _aablackman,                // Antialiasing: Blackmann Filter
    _aagaussian,                // Antialiasing: Gaussian 1/sqrt(2) Filter (blury)
    _aagaussian2,               // Antialiasing: Gaussian 1/2 Filter (sharper)
    _aaquadratic,               // Antialiasing: Quadardic Filter
    _aacubic,                   // Antialiasing: Cubic Filter
    _aacatrom,                  // Antialiasing: Catmull-Rom Filter
    _aamitchell,                // Antialiasing: Mitchell Filter
    _aalanczos2,                // Antialiasing: Lanczos2 Filter
    _aalanczos3,                // Antialiasing: Lanczos3 Filter
    _aablackmanbessel,          // Antialiasing: Blackman/Bessel Filter
    _aablackmansinc             // Antialiasing: Blackman/sinc Filter
};

// Corrections

struct correct_Prefs
{                               //  Preferences structure for tool correct
    uint32_t magic;            //  File validity check, must be 20
    int radial;                 //  Radial correction requested?
    double radial_params[3][5]; //  3 colors x (4 coeffic. for 3rd order polys + correction radius)
    int vertical;               //  Vertical shift requested ?
    double vertical_params[3];  //  3 colors x vertical shift value
    int horizontal;             //  horizontal tilt ( in screenpoints)
    double horizontal_params[3];        //  3 colours x horizontal shift value
    int shear;                  //  shear correction requested?
    double shear_x;             //  horizontal shear values
    double shear_y;             //  vertical shear values

    int tilt;                  //  tilt correction requested?
    double tilt_x;             //  tilt on x values
    double tilt_y;             //  tilt on y values
    double tilt_z;             //  tilt on z values
    double tilt_scale;         //  scale for tilting

    int trans;                  //  translation of camera plane requested?
    double trans_x;            //  x component of translation vector
    double trans_y;            //  y component of translation vector
    double trans_z;             //  z component of translation vector
    double trans_yaw;          // yaw of remapping plane for translation
    double trans_pitch;        // pitch of remapping plane for translation

    int test;                  //  these parameters are for testing new projections
    double test_p0;            //  and make it easier for others to experiment
    double test_p1;            //  
    double test_p2;            // 
    double test_p3;            // 


    int resize;                 //  scaling requested ?
    uint32_t width;             //  new width
    uint32_t height;            //  new height
    int luminance;              //  correct luminance variation?
    double lum_params[3];       //  parameters for luminance corrections
    int correction_mode;        //  0 - radial correction;1 - vertical correction;2 - deregistration
    int cutFrame;               //  remove frame? 0 - no; 1 - yes
    int fwidth;
    int fheight;
    int frame;
    int fourier;                //  Fourier filtering requested?
    int fourier_mode;           //  _faddBlurr vs _fremoveBlurr
    fullPath psf;               //  Point Spread Function, full path/fsspec to psd-file
    int fourier_nf;             //  Noise filtering: _nf_internal vs _nf_custom
    fullPath nff;               //  noise filtered file: full path/fsspec to psd-file
    double filterfactor;        //  Hunt factor
    double fourier_frame;       //  To correct edge errors
};

typedef struct correct_Prefs cPrefs;

enum
{
    correction_mode_radial = 0,
    correction_mode_vertical = 1,
    correction_mode_deregister = 2,
    correction_mode_morph = 4
};


enum
{
    _faddBlurr,
    _fremoveBlurr,
    _nf_internal,
    _nf_custom,
    _fresize,
    _flogtransform
};


enum
{                               // Enumerates for Image.format
    _rectilinear = 0,           // (Standand) FOV (rectilinear) =  2 * arctan (frame size/(focal length * 2))
    _panorama = 1,              // Cylindrical
    _fisheye_circ = 2,          // fisheye-equidistance Circular
    _fisheye_ff = 3,            // fisheye-equidistance Full Frame
    _equirectangular = 4,
    _spherical_cp = 5,          // Fisheye-Horizontal is an image shot with the camera held horizontally. The equator is now in the center of the image.
    _spherical_tp = 6,          // Fisheye-vertical is an image shot with the camera held vertically up.  The panorama is extracted from the circumpherence of the image. 
    _mirror = 7,                // convex mirror. This is the reflection of a convex, spherical image. The horizontal field of view is calculated using the formula HFov = 2*arcsin(radius of mirror/radius of curvature of mirror)
    _orthographic = 8,          // fisheye-orthographic FOV  (orthogonal fisheye) = 2 * arcsin (frame size/(focal length *2)
    _cubic = 9,
    _stereographic = 10,        // fisheye stereographic FOV (stereographic fisheye) = 4 * arctan (frame size/(focal length * 4))
    _mercator = 11,
    _trans_mercator = 12,
    _trans_panorama = 13,
    _sinusoidal = 14,
    _lambert    = 15,
    _lambertazimuthal  = 16,
    _albersequalareaconic = 17,
    _millercylindrical = 18,
    _panini = 19,
    _architectural = 20,
    _equisolid   = 21,          // fisheye-equisolid  FOV (equisolid fisheye) = 4 * arcsin (frame size/(focal length * 4))
    _equipanini = 22,
    _biplane = 23,
   _triplane = 24,
    _panini_general = 25,
    _thoby   = 26,             // generalizes the model found in modern fisheye lenses. It is
                               // parametrizable but it defaults to the Nikkor 10.5 fisheye lens
    _hammer = 27,
};

enum
{                               // Enumerates external number of panorama f<index>
    PANO_FORMAT_RECTILINEAR = 0,
    PANO_FORMAT_PANORAMA = 1,
    PANO_FORMAT_EQUIRECTANGULAR = 2,
    PANO_FORMAT_FISHEYE_FF = 3,
    PANO_FORMAT_STEREOGRAPHIC = 4,
    PANO_FORMAT_MERCATOR = 5,
    PANO_FORMAT_TRANS_MERCATOR = 6,
    PANO_FORMAT_SINUSOIDAL = 7,
    PANO_FORMAT_LAMBERT_EQUAL_AREA_CONIC = 8,
    PANO_FORMAT_LAMBERT_AZIMUTHAL = 9,
    PANO_FORMAT_ALBERS_EQUAL_AREA_CONIC = 10,
    PANO_FORMAT_MILLER_CYLINDRICAL = 11,
    PANO_FORMAT_PANINI = 12,
    PANO_FORMAT_ARCHITECTURAL = 13,
    PANO_FORMAT_ORTHOGRAPHIC = 14,
    PANO_FORMAT_EQUISOLID = 15,
    PANO_FORMAT_EQUI_PANINI = 16,
    PANO_FORMAT_BIPLANE = 17,
    PANO_FORMAT_TRIPLANE = 18,
    PANO_FORMAT_PANINI_GENERAL = 19,
    PANO_FORMAT_THOBY   = 20,
    PANO_FORMAT_HAMMER  = 21,
};

#define PANO_FORMAT_COUNT 22

enum
{                               // Enumerates external number of image f<index>
    IMAGE_FORMAT_RECTILINEAR = 0,
    IMAGE_FORMAT_PANORAMA = 1,
    IMAGE_FORMAT_FISHEYE_EQUIDISTANCECIRC = 2,
    IMAGE_FORMAT_FISHEYE_EQUIDISTANCEFF = 3,
    IMAGE_FORMAT_EQUIRECTANGULAR = 4,
    IMAGE_FORMAT_MIRROR = 7,
    IMAGE_FORMAT_FISHEYE_ORTHOGRAPHIC = 8,
    IMAGE_FORMAT_FISHEYE_STEREOGRAPHIC = 10,
    IMAGE_FORMAT_FISHEYE_EQUISOLID = 21,
    IMAGE_FORMAT_FISHEYE_THOBY = PANO_FORMAT_THOBY,
};
#define IMAGE_FORMAT_COUNT 10

// A large rectangle

typedef struct
{
    int32_t top;
    int32_t bottom;
    int32_t left;
    int32_t right;
} PTRect;

typedef struct
{
    uint32_t full_width;
    uint32_t full_height;
    uint32_t cropped_width;
    uint32_t cropped_height;
    uint32_t x_offset;
    uint32_t y_offset;
} CropInfo;

typedef struct
{
    uint32_t fullWidth;
    uint32_t fullHeight;
    uint32_t croppedWidth;
    uint32_t croppedHeight;
    uint32_t xOffset;
    uint32_t yOffset;
} pano_CropInfo;

typedef struct
{
    uint16_t type;
    uint16_t predictor;
} pano_TiffCompression;

typedef struct
{
    uint32_t size;
    char *data;
} pano_ICCProfile;

typedef struct
{
    // Full size of image
    uint32_t imageWidth;
    uint32_t imageHeight;

    int isCropped;

    float xPixelsPerResolution;
    float yPixelsPerResolution;
    uint16_t resolutionUnits;


    uint16_t samplesPerPixel;
    uint16_t bitsPerSample;
    int bytesPerLine;           // Equal to the scanlinesize

    uint32_t rowsPerStrip;

    pano_TiffCompression compression;

    pano_ICCProfile iccProfile;
    pano_CropInfo cropInfo;

    // other metadata
    char *copyright;
    char *datetime;
    char *imageDescription;
    char *artist;
    uint16_t imageNumber;        // saved in the page number TIFF field
    uint16_t imageTotalNumber;   // total number of images

    // These fields are computed
    int bytesPerPixel;          // This is a common value to use
    int bitsPerPixel;           // This is a common value to use
} pano_ImageMetadata;

// THe following constants define the number of parameters used by a projection

// THe first is the number provided by the user. In most cases it is
// zero, sometimes 1 and sometimes 2.
// The second is the number of internally used parameters. THis is
// used for optimization purposes, as some projections require to compute
// the same value over and over again.


// This are the maximum number of parameters accepted by a given projection
#define PANO_PROJECTION_MAX_PARMS 6
// This are the maximum number of internal parameters used by a given projection
#define PANO_PROJECTION_PRECOMPUTED_VALUES 10


struct Image
{
    // Pixel data
    uint32_t width;
    uint32_t height;
    uint32_t bytesPerLine;
    uint32_t bitsPerPixel;      // Must be 24 or 32
    size_t dataSize;
    unsigned char **data;
    int32_t dataformat;        // rgb, Lab etc
    int32_t format;            // Projection: rectilinear etc
    int formatParamCount;       // Number of format parameters.
    double formatParam[PANO_PROJECTION_MAX_PARMS];  // Parameters for format.
    int precomputedCount;   // number of values precomputed for a given pano
    double precomputedValue[PANO_PROJECTION_PRECOMPUTED_VALUES]; // to speed up pano creation
    double hfov;
    double yaw;
    double pitch;
    double roll;
    cPrefs cP;                  // How to correct the image
    char name[MAX_PATH_LENGTH];
    PTRect selection;
    CropInfo cropInformation; // TO BE DEPRECATED

    pano_ImageMetadata metadata;
};

typedef struct Image Image;



struct TrformStr                // This structure holds all image information
{
    Image          *src;        // Source image, must be supplied on entry
    Image          *dest;       // Destination image data, valid if success = 1 
    int32_t        success;    // 0 - no, 1 - yes 


    int32_t        tool;       // Panorama Tool requested
    int32_t        mode;       // how to run transformation
    void           *data;       // data for tool requested.
    // Required only if mode = _usedata; then it
    // must point to valid preferences structure
    // for requested tool (see filter.h).

    int32_t        interpolator;// Select interpolator
    double          gamma;      // Gamma value for internal gamma correction
    int             fastStep;   // 0 no fast Transformation (default), FAST_TRANSFORM_STEP_MORPH, FAST_TRANSFORM_STEP_NORMAL
};

typedef struct TrformStr TrformStr;


// Useful for looping through images

#define LOOP_IMAGE( image, action ) { 	int x,y,bpp=(image)->bitsPerPixel/8; \
					unsigned char *idata;										\
					for(y=0; y<(image)->height; y++){							\
						idata = *((image)->data) + y * (image)->bytesPerLine;	\
						for(x=0; x<(image)->width;x++, idata+=bpp){				\
							action;} } }


// These structs are to be used to query the features of the different projection formats

typedef struct
{
    double minValue;  // used only if float
    double maxValue;
	double defValue;	// default
    char *name; // name of the parameter (for the purpose of legibility)
} pano_projection_parameter;



typedef struct
{
    int projection;
    int internalFormat; // internal id for Image.format
    double maxVFOV; // units in degrees
    double maxHFOV;
    char *name;
    int numberOfParameters;
    //  so far we dont have more than 3 parameters
    pano_projection_parameter parm[PANO_PROJECTION_MAX_PARMS]; 
} pano_projection_features;

PANO13_IMPEX int panoProjectionFeaturesQuery(int projection, pano_projection_features *features);
PANO13_IMPEX int panoProjectionFormatCount(void);
/** APIs to read dynamic features that depend on projection parameters
    projection argument is an index, same as for queryFeatures()
**/
PANO13_IMPEX int queryFOVLimits( int projection,	
				    double * params, /* length depends on projection */
					double * lims	/* [0] = maxhfov, [1] = maxvfov */
				  );

//void filter_main();

#include "PTcommon.h"

#endif // PANORAMA_H
