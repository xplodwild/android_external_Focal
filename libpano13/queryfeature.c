/* queryfeature.c
   Feature querying functionality
   See queryfeature.h
*/
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "version.h"
#include "queryfeature.h"
#include "filter.h"

typedef struct {char* name; int value;}    TIntFeature;
typedef struct {char* name; double value;} TDoubleFeature;
typedef struct {char* name; char* value;}  TStringFeature;

// Fulvio Senore June.2004 changed the check to work with microsoft compiler
#ifdef _MSC_VER
//#ifdef MSVS
#define snprintf _snprintf
#endif

/***************** Feature tables: *************************/
TIntFeature intFeatures[] ={
  {"CPErrorIsDistSphere",1},       // optimizer reports angular control point errors
  {"NumLensTypes",5},              // source lens types 0..4
  {"NumPanoTypes",8},              // pano lens types 0..7
  {"CanCropOutside",1},
  {"CanHaveNegativeCP",1},
  {"AntiAliasingFilter",1},
  {"NumFilter",24},
  {"SetProgressFcn",1}             // setProgressFcn and setInfoDlgFcn are available
};

TDoubleFeature doubleFeatures[] ={
  {"MaxFFOV",MAX_FISHEYE_FOV}
};

TStringFeature stringFeatures[]={
  // Version info:
  {PTVERSION_NAME_FILEVERSION,PTVERSION_FILEVERSION},         // "FileVersion"
  {PTVERSION_NAME_LONG,LONGVERSION},                          // "LongVersion"
  {PTVERSION_NAME_LEGALCOPYRIGHT,PTVERSION_LEGALCOPYRIGHT},   // "LegalCopyright"
  {PTVERSION_NAME_COMMENT,PTVERSION_COMMENT},                 // "Comments"
  // Source lens type names
  // If a lens type is unavailable, set its name to ""
  {"LensType0","Normal (rectilinear)"},
  {"LensType1","Cylindrical"},
  {"LensType2","Fisheye Equidistance Circular"},
  {"LensType3","Fisheye Equidistance Full Frame"},
  {"LensType4","Equirectangular"},
  {"LensType5",""},//"_spherical_cp"},
  {"LensType6",""},//"_spherical_tp"},
  {"LensType7","Mirror"},
  {"LensType8","Fisheye Orthographic"},
  {"LensType9",""},//"_cubic"},
  {"LensType10","Fisheye Stereographic"},  // fisheye stereographic
  {"LensType11",""},//"_mercator"},
  {"LensType12",""},//"_trans_mercator"},
  {"LensType13",""},//"_trans_panorama"},
  {"LensType14",""},//"_sinusoidal"},
  {"LensType15",""},//"_lambert"},
  {"LensType16",""},//"_lambertazimuthal"},
  {"LensType17",""},//"_albersequalareaconic"},
  {"LensType18",""},//"_millercylindrical"},
  {"LensType19","Fisheye Equisolid"},

  // Source lens type crop format (C)ircle or (R)ectangle:
  {"LensMask0","R"},
  {"LensMask1","R"},
  {"LensMask2","C"},
  {"LensMask3","R"},
  {"LensMask4","R"},
  {"LensMask7","C"},
  {"LensMask8","C"},
  {"LensMask10","C"},
  {"LensMask19","C"},
  // Pano lens type names 
  // If a lens type is unavailable, set its name to ""
  {"PanoType0","Normal (rectilinear)"},
  {"PanoType1","Cylindrical"},
  {"PanoType2","Equirectangular"},
  {"PanoType3","Full Frame"},
  {"PanoType4","Stereographic"},
  {"PanoType5","Mercator"},
  {"PanoType6","Transverse mercator"},
  {"PanoType7","Sinusoidal"},
  {"PanoType8","Lambert Cylindrical Equal Area"},
  {"PanoType9","Lambert Azimuthal Equal Area"},
  {"PanoType10","Albers Conical Equal Area"},
  {"PanoType11","Miller Cylindrical"},
  {"PanoType12","Panini"},
  {"PanoType13","Architectural"},
  {"PanoType14","Orthographic"},
  {"PanoType15","Equisolid"},
  {"PanoType16","Equirectangular Panini"},
  {"PanoType17","Biplane"},
  {"PanoType18","Triplane"},
  {"PanoType19","Panini General"},
  {"PanoType20", "Thoby Projection"},
  {"PanoType21", "Hammer-Aitoff Equal Area"},

  // Filter Types

  //   fix: Fixed Windowsize
  //   aa: Antialiasing filter with adaptive filter size
  // Filter Names 
  {"FilterType0","fix"},
  {"FilterName0","Poly3"},
  {"FilterType1","fix"},
  {"FilterName1","Spline16"},
  {"FilterType2","fix"},
  {"FilterName2","Spline36"},
  {"FilterType2","fix"},
  {"FilterName3","Sinc256"},
  {"FilterType4","fix"},
  {"FilterName4","Spline64"},
  {"FilterType5","fix"},
  {"FilterName5","Bilinear"},
  {"FilterType6","fix"},
  {"FilterName6","Nearest neighbor"},
  {"FilterType7","fix"},
  {"FilterName7","Sinc1024"},
  {"FilterType8","aa"},
  {"FilterName8","Box"},
  {"FilterType9","aa"},
  {"FilterName9","Bartlett/Triangle"},
  {"FilterType10","aa"},
  {"FilterName10","Hermite"},
  {"FilterType11","aa"},
  {"FilterName11","Hanning"},
  {"FilterType12","aa"},
  {"FilterName12","Hamming"},
  {"FilterType13","aa"},
  {"FilterName13","Blackmann"},
  {"FilterType14","aa"},
  {"FilterName14","Gaussian 1/sqrt(2)"},
  {"FilterType15","aa"},
  {"FilterName15","Gaussian 1/2"},
  {"FilterType16","aa"},
  {"FilterName16","Quadardic"},
  {"FilterType17","aa"},
  {"FilterName17","Cubic"},
  {"FilterType18","aa"},
  {"FilterName18","Catmull-Rom"},
  {"FilterType19","aa"},
  {"FilterName19","Mitchell"},
  {"FilterType20","aa"},
  {"FilterName20","Lanczos2"},
  {"FilterType21","aa"},
  {"FilterName21","Lanczos3"},
  {"FilterType22","aa"},
  {"FilterName22","Blackman/Bessel"},	
  {"FilterType23","aa"},
  {"FilterName23","Blackman/sinc"},
#if 0
  // WE NO longer support need to list them. It should be enough to list the version

  // Patches that have been applied
  {"Patch200510a", "Rob Platt, Do not process unchanged color channels for CA correction"},
  {"BMPrev", "Jim Watters, correctly open BMP files created with rows in reverse order"},
  {"Tiff32", "Thomas Rauscher, Load and save TIFF 32-bit with IEEE floats, http://www.pano2qtvr.com/dll_patch/"},
  {"AntiAliasing", "Thomas Rauscher, New antialiasing filter for adjust, http://www.pano2qtvr.com/dll_patch/"},
  {"HDRFile", "Thomas Rauscher, Load and save Radiance HDR files, http://www.pano2qtvr.com/dll_patch/"},
  {"Patch200505", "Douglas Wilkins, Correct behaviour when mode = _usedata"},
#ifdef HasJava
  {"Patch200504a", "Douglas Wilkins, Java support enabled"},
#else
  {"Patch200504a", "Douglas Wilkins, Java support disabled"},
#endif
  {"Patch200502a", "Joost Nieuwenhuijse, Crop outside of image, http://www.ptgui.com"},
  {"Patch200410a", "Jim Watters, JPEG optimization, http://photocreations.ca/panotools"},
  {"FastTransform01", "Fulvio Senore, Fast transform, http://www.fsoft.it/panorama/pano12.htm"},
  {"Patch200407a", "Rik Littlefield, Kevin Kratzke, & Jim Watters, Fix multiple bugs - PSD, 16bit"},
  {"MaskFromFocus_001", "Rik Littlefield, added mask-from-focus, http://www.janrik.net/ptools/"},
  {"Patch200405a", "Rik Littlefield, Improved optimizer, http://www.janrik.net/ptools/"},
  {"Patch200403a", "Kevin Kratzke, Radial Shift fix, http://www.kekus.com"},
  {"Patch200312a", "Jim Watters, Updated PSD format, http://photocreations.ca/panotools"},
  {"Patch200309a", "Jim Watters, Allowed linking of y, p, & r values, http://photocreations.ca/panotools"},
  {"Patch200308a", "Jim Watters, Improved Radial Luminance, http://photocreations.ca/panotools"}
#endif
};
/***************** end feature tables **********************/


int queryFeatureInt(const char *name, int *result)
{
  int i;
  int count = sizeof( intFeatures ) / sizeof( intFeatures[0] );
  for(i=0; i < count; i++)
  {
    if(strcmp(name,intFeatures[i].name)==0)
    {
      *result=intFeatures[i].value;
      return 1;
    }
  }
  return 0;
}

int queryFeatureDouble(const char *name, double *result)
{
  int i;
  int count = sizeof( doubleFeatures ) / sizeof( doubleFeatures[0] );
  for(i=0; i < count; i++)
  {
    if(strcmp(name,doubleFeatures[i].name)==0)
    {
      *result=doubleFeatures[i].value;
      return 1;
    }
  }
  return 0;
}

int queryFeatureString(const char *name,char *result, const int bufsize)
{
  int intvalue;
  double doublevalue;
  size_t i, length=0, count = (sizeof( stringFeatures ) / sizeof( stringFeatures[0] ));
  const size_t tmp_len=200;
  
  // Fulvio Senore, August 2004
  // allocates a dummy buffer for the calls to snprintf
  // the original code passed NULL to snprintf() but it caused problems (asserts) when compiling 
  // with the microsoft compiler that links with the debug libraries to avoid crashes
  char *cpTmp = malloc( tmp_len + 1 );
  cpTmp[tmp_len] = '\0';

  for(i=0; i < count; i++)
  {
    if(strcmp(name,stringFeatures[i].name)==0)
    {
      if(result != NULL)
      {
        strncpy(result, stringFeatures[i].value, (size_t)bufsize);
      }
      length=strlen(stringFeatures[i].value);
      break;
    }
  }
  if(length <= 0)
  {
    // there's no string value with the specified name
    // Let's check the int values too:
    for(i=0; i < sizeof( intFeatures ) / sizeof( intFeatures[0] ); i++)
    {
      if(queryFeatureInt(name, &intvalue))
      {
        // length=snprintf(NULL,0,"%d",intvalue);
        length=snprintf(cpTmp,tmp_len,"%d",intvalue);
        if(result != NULL)
        {
          snprintf(result,(size_t)bufsize,"%d",intvalue);
        }
        break;
      }
    }
  }
  if(length <= 0)
  {
    // Let's check the double values too:
    for(i=0; i < sizeof( doubleFeatures ) / sizeof( doubleFeatures[0] ); i++)
    {
      if(queryFeatureDouble(name, &doublevalue))
      {
//        length=snprintf(NULL,0,"%0.f",doublevalue);
        length=snprintf(cpTmp,tmp_len,"%0.f",doublevalue);
        if(result != NULL)
        {
          snprintf(result,(size_t)bufsize,"%0.f",doublevalue);
        }
        break;
      }
    }
  }
  // make sure that the copied string always is NULL terminated, even if truncated
  // (except if the buffer holds only zero bytes):
  if( result && ((int)length >= bufsize) && (bufsize > 0) )
  {
    result[bufsize-1]=0;
  }
  free( cpTmp );
  return length;
}

int queryFeatureCount()
{
  return sizeof( intFeatures ) / sizeof( intFeatures[0] )+ 
	     sizeof( doubleFeatures ) / sizeof( doubleFeatures[0] ) + 
		 sizeof( stringFeatures ) / sizeof( stringFeatures[0] );
}

void queryFeatures(int index,char** name,Tp12FeatureType* type)
{
  if(index < (sizeof( intFeatures ) / sizeof( intFeatures[0] )))
  {
    if(name) *name=intFeatures[index].name;
    if(type) *type=p12FeatureInt;
  }
  else
  {
    index -= sizeof( intFeatures ) / sizeof( intFeatures[0] );
    if(index < sizeof( doubleFeatures ) / sizeof( doubleFeatures[0] ))
    {
      if(name) *name=doubleFeatures[index].name;
      if(type) *type=p12FeatureDouble;
    }
    else
    {
      index -= sizeof( doubleFeatures ) / sizeof( doubleFeatures[0] );
      if(index < sizeof( stringFeatures ) / sizeof( stringFeatures[0] ))
      {
        if(name) *name=stringFeatures[index].name;
        if(type) *type=p12FeatureString;
      }
      else
      {
        if(type) *type=p12FeatureUnknown;
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////


char *panoFormatNames[] = {
    "Rectilinear",
    "Cylindrical",
    "Equirectangular",
    "Fisheye",
    "Stereographic",
    "Mercator",
    "Trans Mercator",
    "Sinusoidal",
    "Lambert Cylindrical Equal Area",
    "Lambert Equal Area Azimuthal",
    "Albers Equal Area Conic",
    "Miller Cylindrical",
    "Panini",
    "Architectural",
    "Orthographic",
    "Equisolid",
    "Equirectangular Panini",
    "Biplane",
    "Triplane",
    "Panini General",
    "Thoby Projection",
    "Hammer-Aitoff Equal Area",
};

static int panoFormatID[] = {
    _rectilinear,
    _panorama,
    _equirectangular,
    _fisheye_ff,
    _stereographic,
    _mercator,
    _trans_mercator,
    _sinusoidal,
    _lambert,
    _lambertazimuthal,
    _albersequalareaconic,
    _millercylindrical,
    _panini,
    _architectural,
    _orthographic,
    _equisolid,
    _equipanini,
    _biplane,
    _triplane,
    _panini_general,
    _thoby,
    _hammer,
    };


int panoProjectionFormatCount(void)
{
    // Return the number of Projection formats available in the library
    assert(sizeof(panoFormatNames) == PANO_FORMAT_COUNT * sizeof(typeof (panoFormatNames[0])));
    return PANO_FORMAT_COUNT;
}

int panoProjectionFeaturesQuery(int projection, pano_projection_features *features)
{
    // Return static information on the characteristics of each of the projections 
    // in the library.  This now includes min, max and defualt parameter values.
	// All data are zero by default.

    int i;

    assert(features != NULL);
    assert(sizeof(panoFormatNames) == PANO_FORMAT_COUNT * sizeof(char*));


    if (projection < 0 || projection >= PANO_FORMAT_COUNT) 
	return 0;


    // Set defaults
    bzero(features, sizeof (*features));

    features->projection = projection;
    features->internalFormat = panoFormatID[projection];
    features->maxHFOV = 360;
    features->maxVFOV = 180;
    features->name = panoFormatNames[projection];
    switch (projection) {
    case PANO_FORMAT_RECTILINEAR:
	features->maxVFOV = 179;
	features->maxHFOV = 179;
	break;
    case PANO_FORMAT_PANORAMA:
	features->maxVFOV = 179;
	break;
    case PANO_FORMAT_EQUIRECTANGULAR:
    case PANO_FORMAT_MILLER_CYLINDRICAL:
    case PANO_FORMAT_ARCHITECTURAL:
	break;
    case PANO_FORMAT_PANINI:
	features->maxVFOV = 179;
	features->maxHFOV = 359;
        break;
    case PANO_FORMAT_EQUI_PANINI:
	features->maxVFOV = 179;
	features->maxHFOV = 359;
        break;
    case PANO_FORMAT_PANINI_GENERAL:
	features->maxVFOV = 160;
	features->maxHFOV = 320;
	features->numberOfParameters = 3;
	features->parm[0].name = "Cmpr";
	features->parm[1].name = "Tops";
	features->parm[2].name = "Bots";
    features->parm[0].minValue = 0;
    features->parm[0].maxValue = 150;
	features->parm[0].defValue = 100;
    features->parm[1].minValue = -100;
    features->parm[1].maxValue = 100;
    features->parm[2].minValue = -100;
    features->parm[2].maxValue = 100;

        break;
    case PANO_FORMAT_FISHEYE_FF:
    case PANO_FORMAT_THOBY:
	features->maxVFOV = 360;
	features->maxHFOV = 360;
	break;
    case PANO_FORMAT_ORTHOGRAPHIC:
	features->maxVFOV = 180;
	features->maxHFOV = 180;
	break;
    case PANO_FORMAT_EQUISOLID:
	features->maxVFOV = 360;
	features->maxHFOV = 360;
	break;
    case PANO_FORMAT_STEREOGRAPHIC:
	features->maxHFOV = 359;
	features->maxVFOV = 359;
	break;
    case PANO_FORMAT_MERCATOR:
	features->maxVFOV = 179;
	break;
    case PANO_FORMAT_TRANS_MERCATOR:
	features->maxHFOV = 179;
	features->maxVFOV = 360;
	break;
    case PANO_FORMAT_SINUSOIDAL:
    case PANO_FORMAT_LAMBERT_EQUAL_AREA_CONIC:
    case PANO_FORMAT_LAMBERT_AZIMUTHAL:
    case PANO_FORMAT_HAMMER:
	break;
    case PANO_FORMAT_ALBERS_EQUAL_AREA_CONIC:
	features->numberOfParameters = 2;
	features->parm[0].name = "phi1";
	features->parm[1].name = "phi2";
	for (i=0;i<2;i++) {
	    features->parm[i].minValue = -90;
	    features->parm[i].maxValue = +90;
	}
	features->parm[0].defValue = 0;
	features->parm[1].defValue = 60;

	break;
	case PANO_FORMAT_BIPLANE:
        features->maxVFOV = 179;
        features->maxHFOV = 359;
		features->numberOfParameters = 1;
		features->parm[0].name = "alpha";
		features->parm[0].minValue=1;
		features->parm[0].maxValue=179;
		features->parm[0].defValue=45;
		break;
	case PANO_FORMAT_TRIPLANE:
	    features->maxVFOV = 179;
        features->maxHFOV = 359;
        features->numberOfParameters = 1;
		features->parm[0].name = "alpha";
		features->parm[0].minValue=1;
		features->parm[0].maxValue=120;
		features->parm[0].defValue=60;
		break;
    default:
	assert(0); // A projection is missing!
	return 0;
    }
    return 1;
}

int queryFOVLimits( int projection,		/* projection index */
				    double * params, /* length depends on projection */
					double * lims	/* [0] = maxhfov, [1] = maxvfov */
				  )
{
	pano_projection_features pf;
	int ok = panoProjectionFeaturesQuery(projection, &pf);
	lims[0] = lims[1] = 0;
	if( !ok ) return 0;
  // default fixed values
	lims[0] = pf.maxHFOV;
	lims[1] = pf.maxVFOV;
  // compute dynamic values
    switch (projection) {
    default:
	break;
    case PANO_FORMAT_PANINI_GENERAL:
		maxFOVs_panini_general	(params, lims );
    break;
    case PANO_FORMAT_BIPLANE:
        lims[0] = params[0] + 179;
        break;
    case PANO_FORMAT_TRIPLANE:
        lims[0] = 2 * params[0] + 179;
        break;
    case PANO_FORMAT_ALBERS_EQUAL_AREA_CONIC:
	break;
    }

	return 1;
}



