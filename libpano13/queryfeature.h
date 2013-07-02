/* queryfeature.h
   Feature querying functions
   Initial version by Joost Nieuwenhuijse - joost@newhouse.nl
   GPL etc..

   These functions can be used to determine properties of this specific pano13 library.
   Features are name/value pairs. The name is string and the value can be either an
   integer, double or string. Boolean values are encoded as integers (0=false, 1=true)

   Functions queryFeatureCount() and queryFeatures() can be used to obtain a list
   of features. Use queryFeatureInt(), queryFeatureDouble() and queryFeatureString()
   to determine the value of a specific feature. If the feature is known,
   queryFeatureXxx() returns nonzero and sets the *result value.

   queryFeatureString returns the string length of the feature value. The string will
   be copied to the result buffer. Caller should allocate and free the result buffer.
   Value is always NULL terminated (except if bufsize=0). Use
   queryFeatureString(name,NULL,0) to determine the strlen of the feature value.
   In addition to string values, queryFeatureString also returns the int and double
   features converted into a string.

   Usage:
     int i,bufsize,numfeatures;
     char *name;
     char *value;
     Tp12FeatureType type;

     numfeatures=queryFeatureCount();
     for(i=0; i < numfeatures;i++)
     {
       queryFeatures(i,&name,&type);
       bufsize=queryFeatureString(name,NULL,0)+1;
       value=(char*)malloc(bufsize);
       queryFeatureString(name,value,bufsize);
       // ... do something with the value ...
       free(value);
     }
*/

#ifndef _QUERYFEATURE_H
#define _QUERYFEATURE_H
#include "panorama.h"

typedef enum {p12FeatureUnknown=0,p12FeatureInt=1,p12FeatureDouble=2,p12FeatureString=3} Tp12FeatureType;

PANO13_IMPEX int queryFeatureCount();
PANO13_IMPEX void queryFeatures(int index,char** name,Tp12FeatureType* type);
PANO13_IMPEX int queryFeatureInt(const char *name, int *result);
PANO13_IMPEX int queryFeatureDouble(const char *name, double *result);
PANO13_IMPEX int queryFeatureString(const char *name,char *result, const int bufsize);

/** defined if progress and output dialogs can be overridden by using
 *  setProgressFcn, setInfoDlgFcn and setErrorFcn
 */
#define PANO12_FEATURE_DLG_FCN 1

#endif
