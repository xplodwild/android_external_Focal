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

/* ---- Revision history ----

  May 2004, Rik Littlefield, reworked fcnPano and related functions as follows:
               1) For normal control points, allows exposing latitude and longitude
                  error components separately to the optimizer.  This trades
                  faster convergence for slight loss of stability.  This
                  behavior can be changed at runtime by calling setFcnPanoNperCP().
                  (new capability)
               2) Optimize distance^2 instead of distance^4 for hor, vert, and
                  line control points (bug fix)
               3) Scale errors by change in average fov.  This stabilizes
                  fov optimization and allows its use in more cases with partial panos.
                  (new capability)
               4) Improve accuracy of angular distance calculation by using asin
                  instead of acos (results improvement)
               5) Consistently report errors in units of pixels scaled to current
                  panorama size (feature change)
               6) Report rms error during optimization (bug fix)
*/
#include <math.h>
#include "filter.h"
#include "f2c.h"
#include <float.h>
#include <assert.h>
#include "PTcommon.h"

#define C_FACTOR        100.0

static  AlignInfo       *optInfo;       // This struct holds all informations for the optimization

static double initialAvgFov;   // these three for fov stabilization
static double avgfovFromSAP;
static int needInitialAvgFov;

#define ADJUST_LOG_FILENAME "PToolsLog.txt"  // file name for logging, if enabled
#define ADJUST_LOGGING_ENABLED 0

FILE* adjustLogFile = 0;


void                    ColCorrect( Image *im, double ColCoeff[3][2] );
void                    GetColCoeff( Image *src, Image *buf, double ColCoeff[3][2] );
void                    getControlPoints( Image *im, struct controlPoint *cp );
void                    writeControlPoints( struct controlPoint *cp,char* cdesc );
int                     CheckParams( AlignInfo *g );
static int              CheckMakeParams( aPrefs *aP);
//static int            GetOverlapRect( PTRect *OvRect, PTRect *r1, PTRect *r2 );
int                     AddEdgePoints( AlignInfo *gl );
int                     pt_average( uint8_t* pixel, int BytesPerLine, double rgb[3], int bytesPerChannel );
double                  distsqLine(int N0, int N1);



void panoAdjustPrintMakeParams(char *msg, struct MakeParams *mp, Image *im)
{
    printf("-------------%s\n", msg);
    if (mp != NULL) {
        printf("distnace %f\n", mp->distance);
        printf("shear[0] %f\n", mp->shear[0]);
        printf("shear[1] %f\n", mp->shear[1]);
        printf("rot[0] %f\n", mp->rot[0]);
        printf("rot[1] %f\n", mp->rot[1]);
        printf("tilt[0] %f\n", mp->tilt[0]);
        printf("tilt[1] %f\n", mp->tilt[1]);
        printf("tilt[2] %f\n", mp->tilt[2]);
        printf("tilt[3] %f\n", mp->tilt[3]);
        
        printf("trans[0] %f\n", mp->trans[0]);
        printf("trans[1] %f\n", mp->trans[1]);
        printf("trans[2] %f\n", mp->trans[2]);
        printf("trans[3] %f\n", mp->trans[3]);
        printf("trans[4] %f\n", mp->trans[4]);
        
        printf("test[0] %f\n", mp->test[0]);
        printf("test[1] %f\n", mp->test[1]);
        printf("test[2] %f\n", mp->test[2]);
        printf("test[3] %f\n", mp->test[3]);
        
        
        printf("mp->horizontal %f\n", mp->horizontal);
        printf("mp->vertical %f\n", mp->vertical);
    }
    panoPrintImage(msg, im);
    printf("\n\n");

}


void adjust(TrformStr *TrPtr, aPrefs *prefs)
{
        int             destwidth, destheight;
        aPrefs          aP, *aPtr=NULL;
#if 0
        int             nt = 0;         // Morph  parameters
        PTTriangle      *ts=NULL; 
        PTTriangle      *td=NULL; 
#endif
        SetAdjustDefaults(&aP);

        switch( prefs->mode & 7 )// Should we use prefs, or read from script?
        {
                case _insert:
                case _extract:
                        if( prefs->mode & _useScript ){
                                aPtr = readAdjustLine( &(prefs->scriptFile) );
                                if(aPtr==NULL){
                                        PrintError("Error processing script file" );
                                        TrPtr->success = 0;
                                        return;
                                }
                                memcpy(&aP, aPtr, sizeof(aPrefs));
                                free(aPtr); aPtr = &aP;

                                if( (TrPtr->mode & 7) == _usedata ){ // Report panorama format and stitching info back to calling app.
                                        memcpy( &prefs->pano, &aP.pano, sizeof( Image ) );
                                        memcpy( &prefs->sBuf, &aP.sBuf, sizeof( stBuf ) );
                                }

                                TrPtr->interpolator = aP.interpolator;
                                TrPtr->gamma        = aP.gamma;
                TrPtr->fastStep     = aP.fastStep;
                                        
#if 0
                                int readmode = 1;
                                aPtr = &aP;
                                gsPrPtr->interpolator   = TrPtr->interpolator;
                                gsPrPtr->gamma                  = TrPtr->gamma;
                gsPrPtr->fastStep           = TrPtr->fastStep;
                                if( TrPtr->mode & _destSupplied ){
                                        PTRect* p = &TrPtr->dest->selection;
                                        if( !(p->bottom == 0 && p->right == 0) &&
                                            !(p->right == TrPtr->dest->width &&
                                             p->bottom == TrPtr->dest->height) )
                                                readmode = 0;
                                }
                                if( readAdjust( aPtr, &(prefs->scriptFile), readmode, gsPrPtr ) != 0 )
                                {
                                        PrintError("Error processing script file" );
                                        TrPtr->success = 0;
                                        return;
                                }
                                if( (TrPtr->mode & 7) == _usedata ) // Report panorama format and stitching info back to calling app.
                                {
                                        memcpy( &prefs->pano, &aP.pano, sizeof( Image ) );
                                        memcpy( &prefs->sBuf, &aP.sBuf, sizeof( stBuf ) );
                                }
                                // Use modevalues read from script
                                TrPtr->interpolator = gsPrPtr->interpolator;
                                TrPtr->gamma            = gsPrPtr->gamma;
                TrPtr->fastStep     = gsPrPtr->fastStep;
                                
                                // Parse script again, now reading triangles if morphing requested
                                if( aPtr->im.cP.correction_mode & correction_mode_morph )
                                {
                                        char*                           script;
                                        AlignInfo                       ainf;
                                        int                                     nIm, nPts; // Number of image being processed
                                        Image                           im[2];
                                        
                                        script = LoadScript( &(prefs->scriptFile) );
                                        if( script != NULL )                                    // We can read the scriptfile
                                        {       
                                                nIm = numLines( script, '!' ) - 1;
                                                
                                                if( nIm < 0)
                                                        nIm = numLines( script, 'o' ) - 1;
                                        
                                                // Set ainf
                                                ainf.nt         = 0;
                                                ainf.t          = NULL;
                                                ainf.numIm      = 2;
                                                ainf.im         = im;
                                                memcpy( &ainf.pano, &aP.pano, sizeof( Image ));
                                                memcpy( &ainf.im[0], &aP.pano, sizeof( Image ));
                                                memcpy( &ainf.im[1], &aP.pano, sizeof( Image ));
                                                
                                                nPts = ReadMorphPoints( script, &ainf, nIm );
                                                if(nPts > 0) // Found Points
                                                {
                                                        AddEdgePoints( &ainf );
                                                        TriangulatePoints( &ainf, 1 );
                                                        nt = ainf.nt;
                                                        if(nt > 0)
                                                        {
                                                                SortControlPoints       ( &ainf, 1 );
                                                                SetSourceTriangles      ( &ainf, 1, &td  );
                                                                SetDestTriangles    ( &ainf, 1, &ts  );
                                                        }
                                                }
                                                if(ainf.numPts > 0) free(ainf.cpt);
                                                free( script );
                                        }
                                }
#endif
                        }else{
                                aPtr = prefs;
                        }
                         break;
                default:
                        break;
        }
        switch( prefs->mode & 7)
        {
                case _insert:                   // Create a panoramic image using src; merge with buffer if required
                        // Find brightest rectangle if this is a circular fisheye image
                        {
                        Image ImCrop, *theSrc=NULL;
                        // Initialise at least the data pointer since cutTheFrame may not do it
                        ImCrop.data = NULL;
                        
                        if( aPtr->im.format ==_fisheye_circ     && aPtr->im.cP.cutFrame )
                        {
                                int fwidth = TrPtr->src->width, fheight = TrPtr->src->height;
                                
                                if( aPtr->im.cP.frame ) // subtract framewidth from width/height
                                {
                                        fwidth = TrPtr->src->width - aPtr->im.cP.frame;
                                        if( aPtr->im.cP.frame < fwidth ) fwidth -= aPtr->im.cP.frame;
                                        if( aPtr->im.cP.frame < fheight) fheight-= aPtr->im.cP.frame;
                                }
                                else
                                {
                                        if( aPtr->im.cP.fwidth > 0)
                                                fwidth = aPtr->im.cP.fwidth;
                                        if( aPtr->im.cP.fheight > 0)
                                                fheight = aPtr->im.cP.fheight;
                                }
                                        
                                if( cutTheFrame( &ImCrop, TrPtr->src, fwidth, fheight, TrPtr->mode & _show_progress ) != 0 )
                                {
                                        PrintError("Error Cropping Image");
                                        TrPtr->success = 0;
                                        return;
                                }
                                theSrc = TrPtr->src;
                                TrPtr->src = &ImCrop;
                                
                        }
                        // Image params are set as src 
                        aPtr->im.width  = TrPtr->src->width;
                        aPtr->im.height = TrPtr->src->height;
                        
                        // Pano is set to buffer, if merging requested; else as prefs
                        if( *aPtr->sBuf.srcName != 0 )
                        {
                                if (LoadBufImage( &(aPtr->pano), aPtr->sBuf.srcName, 0) != 0 )
                                {
                                        PrintError( "Error loading Buffer; trying without" );
                                }
                        }
                                                
                        if( aPtr->pano.width == 0 && aPtr->im.hfov != 0.0)
                        {
                                aPtr->pano.width = (aPtr->im.width * aPtr->pano.hfov / aPtr->im.hfov);
                                aPtr->pano.width/=10; aPtr->pano.width*=10;
                        }
                        if( aPtr->pano.height == 0 )
                                aPtr->pano.height = aPtr->pano.width/2;

                        destheight                              = aPtr->pano.height;
                        destwidth                               = aPtr->pano.width;
                        
                        if( destheight == 0 || destwidth == 0 )
                        {
                                PrintError("Please set Panorama width/height" );
                                TrPtr->success = 0;
                                goto _insert_exit;
                        }
                        
                
                        if( SetDestImage( TrPtr, destwidth, destheight) != 0)
                        {
                                PrintError("Could not allocate %ld bytes",TrPtr->dest->dataSize );
                                TrPtr->success = 0;
                                goto _insert_exit;
                        }
                        TrPtr->mode                             |= _honor_valid;
                        CopyPosition( TrPtr->src,  &(aPtr->im) );
                        CopyPosition( TrPtr->dest, &(aPtr->pano) );
      // JMW 2008/01/07 Alpha is valid data don't override it with blank data
//                      addAlpha( TrPtr->src ); // Add alpha channel to indicate valid data
                        
                        aPtr->mode = prefs->mode; // For checkparam
                        MakePano( TrPtr,  aPtr );
                        
                        if(aPtr->ts) free(aPtr->ts);
                        if(aPtr->td) free(aPtr->td);

                        // Stitch images; Proceed only if panoramic image valid

                        if( TrPtr->success )
                        {
                                if( *(aPtr->sBuf.srcName) != 0 ){ // We have to merge in one images
                                        // Load the bufferimage
                                        if( LoadBufImage( &aPtr->pano, aPtr->sBuf.srcName, 1 ) != 0 )
                                        {
                                                PrintError( "Could not load buffer %s; Keeping Source",aPtr->sBuf.srcName );
                                                goto _insert_exit;
                                        }

                                        if( HaveEqualSize( &aPtr->pano, TrPtr->dest ))
                                        {
        
                                                // At this point we have two valid, equally sized images                                                
                                                // Do Colour Correction on one or both  images
                                                DoColorCorrection( TrPtr->dest, &aPtr->pano, aPtr->sBuf.colcorrect & 3);
                                                
                                                if( merge( TrPtr->dest , &aPtr->pano, aPtr->sBuf.feather, TrPtr->mode & _show_progress, aPtr->sBuf.seam ) != 0 )
                                                {
                                                        PrintError( "Error merging images. Keeping Source" );
                                                }
                                        }
                                        myfree( (void**)aPtr->pano.data );
                                } // src != 0
                                        
                                if( *(aPtr->sBuf.destName) != 0 ) // save buffer image
                                {
                                        if( SaveBufImage( TrPtr->dest, aPtr->sBuf.destName ) != 0 )
                                                PrintError( "Could not save to Buffer. Most likely your disk is full");
                                }
                        } // Tr.success 
                                

                        if( TrPtr->success == 0  && ! (TrPtr->mode & _destSupplied) )   
                                myfree( (void**)TrPtr->dest->data );
                                
                _insert_exit:
                        if( aPtr->im.format ==_fisheye_circ     && aPtr->im.cP.cutFrame )       // There is a cropped source image;
                        {
                                if( ImCrop.data != NULL )
                                        myfree( (void**) ImCrop.data );
                                TrPtr->src = theSrc;
                        }
                        
                        }
                        break;
                
                case _extract:
                                
                        if( aPtr->im.width == 0 )
                        {
                                aPtr->im.width = 500 ;
                        }
                        if(  aPtr->im.height == 0 )
                        {
                                aPtr->im.height = aPtr->im.width * 4 / 5;
                        }
                                
                        // Set pano-params to src-image irrespective of prefs
                        aPtr->pano.width        = TrPtr->src->width;                            //      width of panorama
                        aPtr->pano.height       = TrPtr->src->height;                           //  height of panorama
                        
                        CopyPosition( TrPtr->src, &(aPtr->pano) );
//                      addAlpha( TrPtr->src ); 
                                
                        if( *(aPtr->sBuf.destName) != 0 ) // save buffer image
                        {
                                if( SaveBufImage( TrPtr->src, aPtr->sBuf.destName ) != 0 )
                                        PrintError( "Could not save Buffer Image. Most likely your disk is full");
                        } 
                        
                        // Set up Image Structure in TrPtr struct


                        destheight                      = aPtr->im.height;
                        destwidth                       = aPtr->im.width;


                        if( SetDestImage( TrPtr, destwidth, destheight) != 0)
                        {
                                PrintError("Could not allocate %ld bytes",TrPtr->dest->dataSize );
                                TrPtr->success = 0;
                                return;
                        }

                        CopyPosition( TrPtr->dest, &(aPtr->im) );

                        TrPtr->mode                                     |= _honor_valid;
                        if( aPtr->pano.hfov == 360.0 )
                                TrPtr->mode                             |= _wrapX;
                        
                        aPtr->mode = prefs->mode; // For checkparam
                        ExtractStill( TrPtr,  aPtr );
                                
                                
                        if( TrPtr->success == 0 && ! (TrPtr->mode & _destSupplied))     
                                myfree( (void**)TrPtr->dest->data );
                        break;
                
                case _readControlPoints:
                        {
                                char                    *script, *newscript, cdesc[1000];
                                controlPoint    cp[NUMPTS];                     // List of Control points

                                script = LoadScript( &(prefs->scriptFile) );
                                if( script != NULL )                                    // We can read the scriptfile
                                {
                                        newscript = (char*) malloc( strlen(script) + NUMPTS * 60 ); // One line per pair of points
                                        if( newscript != NULL )
                                        {
                                                readControlPoints( script, cp );                // If this is the second image: get coordinates in first
                                                getControlPoints( TrPtr->src, cp );             // Scan image and find control points
                                                writeControlPoints( cp, cdesc );                // format control point coordinates
                                                
                                                sprintf( newscript, "%s\n%s", script, cdesc );
                                                
                                                if( WriteScript( newscript,&( prefs->scriptFile), 0 ) != 0 )
                                                                                PrintError( "Could not write Scriptfile" );
                                                free( newscript );
                                        }
                                        free( script );
                                }

                        }
                        TrPtr->success = 0;                                                     // Don't destroy image!
                        break;


                case _runOptimizer:
                        // Run Optimizer; Dummy image needed but not changed
                        {
                                char*                           script;
                                OptInfo                         opt;
                                AlignInfo                       ainf;

                                script = LoadScript( &(prefs->scriptFile) );
                                if( script != NULL )                                    // We can read the scriptfile
                                {
                                        if (ParseScript( script, &ainf ) == 0)
                                        {
                                                if( CheckParams( &ainf ) == 0 )                                 // and it seems to make sense
                                                {
                                                        ainf.fcn        = fcnPano;
                                                        
                                                        // optInfo is a static variable that is used in all optimizations
                                                        SetGlobalPtr( &ainf ); // equivalent to optInfo = &ainf;
                                                        
                                                        opt.numVars             = optInfo->numParam;
                                                        opt.numData             = optInfo->numPts;
                                                        opt.SetVarsToX          = SetLMParams;
                                                        opt.SetXToVars          = SetAlignParams;
                                                        opt.fcn                 = optInfo->fcn;
                                                        *opt.message            = 0;
                                                        RunLMOptimizer( &opt );
                                                        optInfo->data                           = opt.message;
                                                        WriteResults( script, &(prefs->scriptFile), optInfo, distSquared ,
                                                                    ( TrPtr->mode & 7 ) != _usedata );
                                                }
                                                DisposeAlignInfo( &ainf );                                      // These were allocated by 'ParseScript()'
                                        }
                                        free( script );
                                }
                        }
                                
                        TrPtr->success = 0;                                                     // Don't destroy Dummy image!
                        break;
                default:
                        TrPtr->success = 0;                                                     
                        break;

        }
}


// Make a pano in TrPtr->dest (must be allocated and all set!)
// using parameters in aPrefs (ignore image parameters in TrPtr !)

void MakePano( TrformStr *TrPtr, aPrefs *aP )
{
        MyMakePano( TrPtr, aP, 1 );
}


/*This function was added by Kekus Digital on 18/9/2002. 
This function takes the parameter 'imageNum' which repesents the index 
of the image that has to be converted.*/
void MyMakePano( TrformStr *TrPtr, aPrefs *aP, int imageNum )
{
        struct  MakeParams      mp,mpinv;
        fDesc        stack[15], fD;             // Parameters for execute 
        fDesc        invstack[15], finvD;               // Invers Parameters for execute 
        void    *morph[3];      

        int     i,k, kstart, kend, color;

        TrPtr->success = 1;
        
        if( CheckMakeParams( aP) != 0)
        {
                TrPtr->success = 0;
                return;
        }


        if(  isColorSpecific( &(aP->im.cP) ) )                  // Color dependent
        {
                kstart  = 1; kend       = 4;
        }
        else                                                                                    // Color independent
        {
                kstart  = 0; kend       = 1;
        }
                                
        for( k = kstart; k < kend; k++ )
        {
                color = k-1; if( color < 0 ) color = 0;
                SetMakeParams( stack, &mp, &(aP->im) , &(aP->pano), color );
                SetInvMakeParamsCorrect( invstack, &mpinv, &(aP->im) , &(aP->pano), color );
                
                if( aP->nt > 0 )        // Morphing requested
                {
                        morph[0] = (void*)aP->td;
                        morph[1] = (void*)aP->ts;
                        morph[2] = (void*)&aP->nt;

                        i=0; while( stack[i].func != NULL && i<14 ) i++;
                        if( i!=14 )
                        {
                                for(i=14; i>0; i--)
                                {
                                        memcpy( &stack[i], &stack[i-1], sizeof( fDesc ));
                                }
                                stack[0].func           = tmorph;
                                stack[0].param          = (void*)morph;
                        }
                }

                if( TrPtr->success != 0)
                {
                        fD.func = execute_stack_new; fD.param = stack;
                        finvD.func = execute_stack_new; finvD.param = invstack;

                        transFormEx( TrPtr,  &fD , &finvD , k, imageNum );
                }
        }
}

// Extract image from pano in TrPtr->src 
// using parameters in prefs (ignore image parameters
// in TrPtr)

void ExtractStill( TrformStr *TrPtr , aPrefs *aP )
{
        struct  MakeParams      mp,mpinv;
        fDesc   stack[15], fD;          // Parameters for execute 
        fDesc   stackinv[15], fDinv;            // Invers Parameters for execute 

        int     k, kstart, kend, color;

        TrPtr->success = 1;

        if( CheckMakeParams( aP) != 0)
        {
                TrPtr->success = 0;
                return;
        }
                

        if( isColorSpecific( &(aP->im.cP) ) )                   // Color dependent
        {
                kstart  = 1; kend       = 4;
        }
        else                                                                                                                    // Color independent
        {
                kstart  = 0; kend       = 1;
        }
                                
        for( k = kstart; k < kend; k++ )
        {
                color = k-1; if( color < 0 ) color = 0;
                SetInvMakeParamsCorrect( stack, &mp, &(aP->im), &(aP->pano), color );
                SetMakeParams( stackinv, &mpinv, &(aP->im), &(aP->pano), color );
                
                if( TrPtr->success != 0)
                {
                        fD.func = execute_stack_new; fD.param = stack;
                        fDinv.func = execute_stack_new; fDinv.param = stackinv;
                        transFormEx( TrPtr, &fD, &fDinv, k, 1 );
                }
        }
}


// Set Makeparameters depending on adjustprefs, color and source image

void SetMakeParams( struct fDesc *stack, struct MakeParams *mp, Image *im , Image *pn, int color )
{
  int         i;
  double      a,b;                        // field of view in rad
  double      tx,ty, tpara;               // temporary variables
/* Joost Nieuwenhuijse, 3 feb 2005: Fix for cropping bug
   If a script containing the 'C' crop parameter was stitched by PTStitcher,
   it would fail if the cropping area is partially outside the source image.

   For 'inside' cropping, PTStitcher apparently pre-crops the images, such that
   *im contains the cropped area of the source image.
   For 'outside' cropping, PTStitcher apparently does nothing. The cropping area
   is stored in im->selection, and im->cp.cutFrame is set, but this information
   was not used at all.

   This is fixed here: All processing is now done based on the width&height of the
   cropped area (instead of the width&height of the image). And an additional horizontal
   and vertical offset are added to compensate for the shift of the center of the
   crop area relative to the center of the image.
*/
  int image_selection_width=im->width;
  int image_selection_height=im->height;
  mp->im = im;
  mp->pn = pn;
  if(im->cP.horizontal)
  {
    mp->horizontal=im->cP.horizontal_params[color];
  }
  else
  {
    mp->horizontal=0;
  }
  if(im->cP.vertical)
  {
    mp->vertical=im->cP.vertical_params[color];
  }
  else
  {
    mp->vertical=0;
  }
  if( (im->selection.left != 0) || (im->selection.top != 0) || (im->selection.bottom != 0) || (im->selection.right != 0) )
  {
    if(im->cP.cutFrame)
    {
      image_selection_width  = im->selection.right  - im->selection.left;
      image_selection_height = im->selection.bottom - im->selection.top;
      mp->horizontal += (im->selection.right  + im->selection.left - (int32_t)im->width)/2.0;
      mp->vertical   += (im->selection.bottom + im->selection.top  - (int32_t)im->height)/2.0;
    }
  }

  a   =    DEG_TO_RAD( im->hfov );    // field of view in rad
  b   =    DEG_TO_RAD( pn->hfov );

  SetMatrix( - DEG_TO_RAD( im->pitch ),
             0.0,
             - DEG_TO_RAD( im->roll ),
             mp->mt,
             0 );

#if 0
        switch (pn->format)
        {
        case _rectilinear:
            mp->distance        = (double) pn->width / (2.0 * tan(b/2.0));
            if(im->format == _rectilinear) // rectilinear image
            {
                mp->scale[0] = ((double)pn->hfov / im->hfov) * 
                                        (a /(2.0 * tan(a/2.0))) * ((double)image_selection_width/(double) pn->width)
                                        * 2.0 * tan(b/2.0) / b; 

            }
            else //  pamoramic or fisheye image
            {
                    mp->scale[0] = ((double)pn->hfov / im->hfov) * ((double)image_selection_width/ (double) pn->width)
                                            * 2.0 * tan(b/2.0) / b; 
            }
            break;
        case _equirectangular:
        case _fisheye_ff:
        case _panorama:
        case _mercator:
        case _sinusoidal:
            // horizontal pixels per degree
            mp->distance        = ((double) pn->width) / b;
            if(im->format == _rectilinear) // rectilinear image
            {
                    mp->scale[0] = ((double)pn->hfov / im->hfov) * (a /(2.0 * tan(a/2.0))) * ((double)image_selection_width)/ ((double) pn->width); 
            }
            else //  pamoramic or fisheye image
            {
                    mp->scale[0] = ((double)pn->hfov / im->hfov) * ((double)image_selection_width)/ ((double) pn->width); 
            }
            break;
        case _stereographic:
        case _trans_mercator:
        default:
            break;
        }
        mp->scale[1]    = mp->scale[0];

        //        printf("\nOrig params: mp->distance: %lf, mp->scale: %lf\n\n", mp->distance, mp->scale[0]);
#endif

 /* Pablo d'Angelo, April 2006.
  * Added more output projection types. Broke mp->distance and mp->scale factor calculation
  * into separate parts, making it easier to add new projection types
 */
  // calculate distance
  switch (pn->format)
  {
    case _rectilinear:
      mp->distance        = (double) pn->width / (2.0 * tan(b/2.0));
      break;
    case _equirectangular:
    case _fisheye_ff:
    case _fisheye_circ:
    case _panorama:
    case _lambert:
    case _mercator:
    case _millercylindrical:
    case _sinusoidal:
    case _mirror:
      // horizontal pixels per degree
      mp->distance        = ((double) pn->width) / b;
      break;
    case _panini: 
      tpara = 1;
      panini_erect(b/2.0, 0.0, &tx, &ty, & tpara);
      mp->distance = pn->width/(2.0*tx);
      break;
    case _equipanini: 
      tpara = 1;
      equipanini_erect(b/2.0, 0.0, &tx, &ty, & tpara);
      mp->distance = pn->width/(2.0*tx);
      break;
    case _panini_general: 
      // call setup_panini_general() to set distanceparam
		pn->precomputedCount = 0;	// clear old settings
		setup_panini_general( mp );
	  // should abort now if it returns NULL
      break;
    case _architectural: 
      tpara = 1;
      arch_erect(b/2.0, 0.0, &tx, &ty, & tpara);
      mp->distance = pn->width/(2.0*tx);
      break;
    case _lambertazimuthal: 
      tpara = 1;
      lambertazimuthal_erect(b/2.0, 0.0, &tx, &ty, & tpara);
      mp->distance = pn->width/(2.0*tx);
      break;
    case _hammer:
      tpara = 1;
      hammer_erect(b/2.0, 0.0, &tx, &ty, & tpara);
      mp->distance = pn->width/(2.0*tx);
      break;
    case _stereographic:
      tpara = 1;
      stereographic_erect(b/2.0, 0.0, &tx, &ty, & tpara);
      mp->distance = pn->width/(2.0*tx);
      break;
    case _trans_mercator:
      tpara = 1;
      transmercator_erect(b/2.0, 0.0, &tx, &ty, &tpara);
      mp->distance = pn->width/(2.0*tx);
      break;
    case _albersequalareaconic:
      mp->distance = 1.0;
      //albersequalareaconic_erect(1.924913116, -PI/2.0, &tx, &ty, mp); //b/2.0
      albersequalareaconic_distance(&tx, mp);
      mp->distance = pn->width/(2.0*tx);
      break;
    case _equisolid:
      mp->distance  = (double) pn->width / (4.0 * sin(b/4.0));
      break;
    case _orthographic:
      mp->distance  = (double) pn->width / (2.0 * sin(b/2.0));
       break;
    case _thoby:
      mp->distance  = (double) pn->width / (2.0 * THOBY_K1_PARM * sin(b * THOBY_K2_PARM/2.0));
       break;
        case _biplane:
          biplane_distance(pn->width,b,mp);
          break;
        case _triplane:
          triplane_distance(pn->width,b,mp);
          break;
    default:
      // unknown
      PrintError ("SetMakeParams: Unsupported panorama projection");
      // no way to report an error back to the caller...
      mp->distance = 1;
      break;
  }

  // calculate final scaling factor, that reverses the mp->distance
  // scaling and applies the required output scaling factor
  // printf("im format %d\n", im->format);
  switch (im->format)
  {
    case _rectilinear:
      // calculate distance for this projection
      mp->scale[0] = (double) image_selection_width / (2.0 * tan(a/2.0)) / mp->distance;
      break;
    case _equirectangular:
    case _panorama:
    case _fisheye_ff:
    case _fisheye_circ:
    case _mercator:
    case _sinusoidal:
      mp->scale[0] = ((double) image_selection_width) / a / mp->distance;
      break;
    case _equisolid:
    case _mirror:
      mp->scale[0] = (double) image_selection_width / (4.0 * sin(a/4.0)) / mp->distance;
      break;
    case _orthographic:
      {
          //generate monotonic scale function to help optimizer
          int t=(int)ceil((a-PI)/(2.0*PI));
          mp->scale[0] = (double) image_selection_width / (2.0 * (2 * t + pow(-1.0, t) * sin(a/2.0))) / mp->distance;
      };
      break;
    case _thoby:
      mp->scale[0] = (double) image_selection_width / (2.0 * THOBY_K1_PARM * sin(a * THOBY_K2_PARM /2.0)) / mp->distance;
      break;
    case _stereographic:
      mp->scale[0] = (double) image_selection_width / (4.0 * tan(a/4.0)) / mp->distance;
      break;
    default:
      PrintError ("SetMakeParams: Unsupported input image projection");
      // no way to report an error back to the caller...
      mp->scale[0] = 1;
      break;
    }
    mp->scale[1]    = mp->scale[0];

    //  printf("new params: mp->distance: %lf, mp->scale: %lf\n\n", mp->distance, mp->scale[0]);

    mp->shear[0]    = im->cP.shear_x / image_selection_height;
    mp->shear[1]    = im->cP.shear_y / image_selection_width;
    mp->rot[0]      = mp->distance * PI;                                // 180 in screenpoints
    mp->rot[1]      = -im->yaw *  mp->distance * PI / 180.0;            // rotation angle in screenpoints
    
    mp->tilt[0] = DEG_TO_RAD(im->cP.tilt_x);
    mp->tilt[1] = DEG_TO_RAD(im->cP.tilt_y);
    mp->tilt[2] = DEG_TO_RAD(im->cP.tilt_z);
    mp->tilt[3] = im->cP.tilt_scale;

    mp->trans[0] = im->cP.trans_x;
    mp->trans[1] = im->cP.trans_y;
    mp->trans[2] = im->cP.trans_z;
    mp->trans[3] = DEG_TO_RAD(im->cP.trans_yaw);
    mp->trans[4] = DEG_TO_RAD(im->cP.trans_pitch);

    mp->test[0] = im->cP.test_p0;
    mp->test[1] = im->cP.test_p1;
    mp->test[2] = im->cP.test_p2;
    mp->test[3] = im->cP.test_p3;

    //    panoAdjustPrintMakeParams("SetmakeParms", mp, im);

    mp->perspect[0] = (void*)(mp->mt);
    mp->perspect[1] = (void*)&(mp->distance);
            
    for(i=0; i<4; i++)
        mp->rad[i]  = im->cP.radial_params[color][i];
    mp->rad[5] = im->cP.radial_params[color][4];

    if( (im->cP.correction_mode & 3) == correction_mode_radial )
        mp->rad[4]  = ( (double)( image_selection_width < image_selection_height ? image_selection_width : image_selection_height) ) / 2.0;
    else
        mp->rad[4]  = ((double) image_selection_height) / 2.0;

    // Joost: removed, see above
    //  mp->horizontal  = im->cP.horizontal_params[color];
    //  mp->vertical  = im->cP.vertical_params[color];

    i = 0;


    // Building the stack
    //
    // - Convert  from panorama projection to equirectangular
    // - Rotate horizontally
    // - Convert to spherical from equirectangular
    // - Apply perspective correction (pitch and roll) in spherical coordinates
    // - Convert to image format (rectilinear, pano, equirectangular)
    // - Scale output image
    // - Do radial correction
    // - Do tilt
    // - Do vertical shift
    // - Do horizontal shift
    // - Do shear


    //////////////////////////////////////////////////////////////////////
    // Convert from output projection to spherical coordinates
    //
    if(pn->format == _rectilinear)                                  // rectilinear panorama
        {
            SetDesc(stack[i],   erect_rect,             &(mp->distance) ); i++;   // Convert rectilinear to equirect
        }
    else if(pn->format == _panorama)
        {
            SetDesc(stack[i],   erect_pano,             &(mp->distance) ); i++;   // Convert panoramic to equirect
        }
    else if(pn->format == _fisheye_circ || pn->format == _fisheye_ff)
        {
          // the sphere coordinates are actually equivalent to the equidistant fisheye projection
            SetDesc(stack[i],   erect_sphere_tp,        &(mp->distance) ); i++; // Convert fisheye to equirect
        }
    else if(pn->format == _equisolid)
        {
            SetDesc(stack[i],   sphere_tp_equisolid,    &(mp->distance) ); i++; // Convert fisheye equisolid to spherical
            SetDesc(stack[i],   erect_sphere_tp,        &(mp->distance) ); i++; // Convert spherical to equirect
        }
    else if(pn->format == _mirror)
        {
            SetDesc(stack[i],   sphere_cp_mirror,       &(mp->distance) ); i++; // Convert mirror to spherical
            SetDesc(stack[i],   erect_sphere_cp,        &(mp->distance) ); i++; // Convert spherical to equirect
        }
    else if(pn->format == _orthographic)
        {
            SetDesc(stack[i],   sphere_tp_orthographic, &(mp->distance) ); i++; // Convert fisheye orthographic to spherical
            SetDesc(stack[i],   erect_sphere_tp,        &(mp->distance) ); i++; // Convert spherical to equirect
        }
    else if(pn->format == _thoby)
        {
            SetDesc(stack[i],   sphere_tp_thoby, &(mp->distance) ); i++; // Convert thoby to spherical
            SetDesc(stack[i],   erect_sphere_tp,        &(mp->distance) ); i++; // Convert spherical to equirect
        }
    else if(pn->format == _mercator)
        {
            SetDesc(stack[i],   erect_mercator,         &(mp->distance) ); i++; // Convert mercator to equirect
        }
    else if(pn->format == _millercylindrical)
        {
            SetDesc(stack[i],   erect_millercylindrical, &(mp->distance) ); i++; // Convert miller to equirect
        }
    else if(pn->format == _panini)
        {
            SetDesc(stack[i],     erect_panini,           &(mp->distance) ); i++; // Convert panini to sphere
        }
    else if(pn->format == _equipanini)
        {
            SetDesc(stack[i],     erect_equipanini,           &(mp->distance) ); i++; // Convert equipanini to sphere
        }
    else if(pn->format == _panini_general)
        {
            SetDesc(stack[i],     erect_panini_general,           mp ); i++; // Convert general panini to sphere
        }
    else if(pn->format == _architectural)
        {
            SetDesc(stack[i],   erect_arch,             &(mp->distance) ); i++; // Convert arch to sphere
        }
    else if(pn->format == _lambert)
        {
            SetDesc(stack[i],   erect_lambert,          &(mp->distance) ); i++; // Convert lambert to equirect
        }
    else if(pn->format == _lambertazimuthal)
        {
            SetDesc(stack[i],   erect_lambertazimuthal, &(mp->distance) ); i++; // Convert lambert to equirect
        }
    else if(pn->format == _hammer)
        {
            SetDesc(stack[i],   erect_hammer, &(mp->distance) ); i++; // Convert hammer to equirect
        }
    else if(pn->format == _trans_mercator)
        {
            SetDesc(stack[i],   erect_transmercator,    &(mp->distance)  ); i++; // Convert transverse mercator to equirect
        }
    else if(pn->format == _stereographic)
        {
            SetDesc(stack[i],   erect_stereographic,    &(mp->distance) ); i++;  // Convert stereographic to equirect
        }
    else if(pn->format == _sinusoidal)
        {
            SetDesc(stack[i],   erect_sinusoidal,       &(mp->distance) ); i++; // Convert sinusoidal to equirect
        }
    else if(pn->format == _albersequalareaconic)
        {
            SetDesc(stack[i],   erect_albersequalareaconic,     mp  ); i++; // Convert albersequalareaconic to equirect
        }
    else if(pn->format == _biplane)
        {
            SetDesc(stack[i], erect_biplane, mp ); i++;  // Convert biplane to equirect
        }
    else if(pn->format == _triplane)
        {
            SetDesc(stack[i], erect_triplane, mp ); i++;  // Convert triplane to equirect
        }
    else if(pn->format == _equirectangular) 
        {
            // no conversion needed     
        } 
    else 
        {
            PrintError("Projection type %d not supported. Assuming equirectangular", pn->format);
        }

    if (im->cP.trans) {
        SetDesc(stack[i], plane_transfer_to_camera, mp);   i++;
    }    

    SetDesc(  stack[i],   rotate_erect,           mp->rot         ); i++; // Rotate equirect. image horizontally
    SetDesc(  stack[i],   sphere_tp_erect,        &(mp->distance) ); i++; // Convert spherical image to equirect.
    SetDesc(  stack[i],   persp_sphere,           mp->perspect    ); i++; // Perspective Control spherical Image

    //////////////////////////////////////////////////////////////////////
    // Convert from spherical coordinates to input projection
    //
    if(im->format      == _rectilinear)                                    // rectilinear image
        {
            SetDesc(stack[i],   rect_sphere_tp,         &(mp->distance) ); i++; // Convert spherical to rectilinear
        }
    else if(im->format == _panorama)                                   //  pamoramic image
        {
            SetDesc(stack[i],   pano_sphere_tp,         &(mp->distance) ); i++; // Convert spherical to pano
        }
    else if(im->format == _equirectangular)                            //  equirectangular image
        {
            SetDesc(stack[i],   erect_sphere_tp,        &(mp->distance) ); i++; // Convert spherical to equirect
        }
    else if (im->format == _fisheye_circ || im->format == _fisheye_ff) 
        {
            ; // no conversion needed. It is already in spherical coordinates
        }
    else if (im->format == _mirror) 
        {
            SetDesc(stack[i],   mirror_sphere_tp,           &(mp->distance) ); i++; // Convert spherical to mirror
        }
    else if (im->format == _stereographic) 
        {
            SetDesc(stack[i],   erect_sphere_tp,           &(mp->distance) ); i++; // Convert spherical to equirectangular
            SetDesc(stack[i],   stereographic_erect,       &(mp->distance) ); i++; // Convert equirectangular to stereographic
        }
    else if (im->format == _orthographic) 
        {
            SetDesc(stack[i],   orthographic_sphere_tp,           &(mp->distance) ); i++; // Convert spherical to orthographic
        }
    else if (im->format == _thoby) 
        {
            SetDesc(stack[i],   thoby_sphere_tp,           &(mp->distance) ); i++; // Convert spherical to thoby
        }
    else if (im->format == _equisolid) 
        {
            SetDesc(stack[i],   erect_sphere_tp,           &(mp->distance) ); i++; // Convert spherical to equirectangular
            SetDesc(stack[i],   lambertazimuthal_erect,       &(mp->distance) ); i++; // Convert equirectangular to stereographic
        }
    else 
        {
            PrintError("Invalid input projection %d. Assumed fisheye.", im->format);
        }
        

    SetDesc(  stack[i],   resize,                 mp->scale       ); i++; // Scale image

    //////////////////////////////////////////////////////////////////////
    // Apply lens corrections
    //

    if( im->cP.radial )
        {
            switch( im->cP.correction_mode & 3 )
                {
                case correction_mode_radial:    SetDesc(stack[i],radial,mp->rad);     i++; break;
                case correction_mode_vertical:  SetDesc(stack[i],vertical,mp->rad);   i++; break;
                case correction_mode_deregister:SetDesc(stack[i],deregister,mp->rad); i++; break;
                }
        }
    if (im->cP.tilt) {
        SetDesc(stack[i],   tiltInverse,                   mp);   i++;
    }

    if (mp->vertical != 0.0)
        {
            SetDesc(stack[i],   vert,                   &(mp->vertical));   i++;
        }
    if (mp->horizontal != 0.0)
        {
            SetDesc(stack[i],   horiz,                  &(mp->horizontal)); i++;
        }
    if( im->cP.shear )
        {
            SetDesc( stack[i],  shear,                  mp->shear       ); i++;
        }

    stack[i].func  = (trfn)NULL;

    // print stack for debugging
#if 0
    printf( "Rotate params: %lg  %lg\n" , mp->rot[0], mp->rot[1]);
    printf( "Distance     : %lg\n" , mp->distance);
    printf( "Perspect params: %lg  %lg  %lg\n",a, beta , gammar );      
    if(aP->format       == _rectilinear)                                    // rectilinear image
        {
            printf( "Rectilinear\n" );      
        }
    else if (aP->format     == _panorama)                                   //  pamoramic image
        {
            printf( "Panorama\n" );
        }
    else
        {
            printf( "Fisheye\n" );      
        }

    printf( "Scaling     : %lg\n" , mp->scale[0]);

    if(  aP->correct )
        {
            printf( "Correct:\n" );     
            if( aP->c_prefs.tilt )
                {
                    printf( "Tilt: %lg\n", mp->tilt );    
                }
            if( aP->c_prefs.shear )
                {
                    printf( "Shear: %lg\n", mp->shear );    
                }
            if ( aP->c_prefs.horizontal )
                {
                    printf( "horiz:%lg\n", mp->horizontal );  
                }
            if (  aP->c_prefs.vertical)
                {
                    printf( "vert:%lg\n", mp->vertical );  
                }
            if( aP->c_prefs.radial )
                {
                    printf( "Polynomial:\n" );      
                    if( aP->c_prefs.isScanningSlit )
                        {
                            printf( "Scanning Slit:\n" );   
                        }
                    else
                        {
                            printf( "Radial:\n" );      
                            printf( "Params: %lg %lg %lg %lg %lg\n", mp->rad[0],mp->rad[1],mp->rad[2],mp->rad[3],mp->rad[4] );      
                        }
                }
        }

#endif
}



// Set inverse Makeparameters depending on adjustprefs, color and source image
// This code is executed when optimization
void  SetInvMakeParams( struct fDesc *stack, struct MakeParams *mp, Image *im , Image *pn, int color )
{

    int     i;
    double    a,b;              // field of view in rad
    double      tx,ty,tpara;

    a =  DEG_TO_RAD( im->hfov );  // field of view in rad   
    b =  DEG_TO_RAD( pn->hfov );

    mp->im = im;
    mp->pn = pn;

    SetMatrix( DEG_TO_RAD( im->pitch ), 
               0.0, 
               DEG_TO_RAD( im->roll ), 
               mp->mt, 
               1 );

    // dangelo: added mercator, sinusoidal and stereographic projection
    switch (pn->format)
        {
        case _rectilinear:
            mp->distance        = (double) pn->width / (2.0 * tan(b/2.0));
            break;
        case _equirectangular:
        case _fisheye_ff:
        case _fisheye_circ:
        case _panorama:
        case _lambert:
        case _mercator:
        case _millercylindrical:
        case _sinusoidal:
        case _mirror:
            // horizontal pixels per rads
            mp->distance        = ((double) pn->width) / b;
            break;
        case _lambertazimuthal:
            tpara = 1;
            lambertazimuthal_erect(b/2.0, 0.0, &tx, &ty, & tpara);
            mp->distance = pn->width/(2.0*tx);
            break;
        case _hammer:
            tpara = 1;
            hammer_erect(b/2.0, 0.0, &tx, &ty, & tpara);
            mp->distance = pn->width/(2.0*tx);
            break;
        case _panini:
            tpara = 1;
            panini_erect(b/2.0, 0.0, &tx, &ty, & tpara);
            mp->distance = pn->width/(2.0*tx);
            break;
        case _equipanini:
            tpara = 1;
            equipanini_erect(b/2.0, 0.0, &tx, &ty, & tpara);
            mp->distance = pn->width/(2.0*tx);
            break;
        case _panini_general:
		  // call setup_panini_general() to set distanceparam
			setup_panini_general( mp );
		  // should abort now if it returns NULL
            break;
        case _architectural:
            tpara = 1;
            arch_erect(b/2.0, 0.0, &tx, &ty, & tpara);
            mp->distance = pn->width/(2.0*tx);
            break;
        case _stereographic:
            tpara = 1;
            stereographic_erect(b/2.0, 0.0, &tx, &ty, & tpara);
            mp->distance = pn->width/(2.0*tx);
            break;
        case _trans_mercator:
            tpara = 1;
            transmercator_erect(b/2.0, 0.0, &tx, &ty, & tpara);
            mp->distance = pn->width/(2.0*tx);
            break;
        case _albersequalareaconic:
            mp->distance = 1.0;
            //albersequalareaconic_erect(1.924913116, -PI/2.0, &tx, &ty, mp);  //b/2.0
            albersequalareaconic_distance(&tx, mp);
            mp->distance = pn->width/(2.0*tx);
            break;
        case _equisolid:
            mp->distance  = (double) pn->width / (4.0 * sin(b/4.0));
            break;
        case _orthographic:
            mp->distance  = (double) pn->width / (2.0 * sin(b/2.0));
            break;
        case _thoby:
            mp->distance  = (double) pn->width / (2.0 * THOBY_K1_PARM * sin(b * THOBY_K2_PARM/2.0));
            break;
        case _biplane:
            biplane_distance(pn->width,b,mp);
            break;
        case _triplane:
            triplane_distance(pn->width,b,mp);
            break;    default:
            // unknown
            PrintError ("SetInvMakeParams: Unsupported panorama projection");
            // no way to report an error back to the caller...
            mp->distance = 1;
            break;
        }

    // calculate final scaling factor, that reverses the mp->distance
    // scaling and applies the required output scaling factor
    switch (im->format)
        {
        case _rectilinear:
            // calculate distance for this projection
            mp->scale[0] = (double) im->width / (2.0 * tan(a/2.0)) / mp->distance;
            break;
        case _equirectangular:
        case _panorama:
        case _fisheye_ff:
        case _fisheye_circ:
        case _mercator:
        case _sinusoidal:
            mp->scale[0] = ((double) im->width) / a / mp->distance;
            break;
        case _equisolid:
        case _mirror:
            mp->scale[0] = (double) im->width / (4.0 * sin(a/4.0)) / mp->distance;
            break;
        case _orthographic:
            {
                //generate monotonic scale function to help optimizer
                int t=(int)ceil((a-PI)/(2.0*PI));
                mp->scale[0] = (double) im->width / (2.0 * (2 * t + pow(-1.0, t) * sin(a/2.0))) / mp->distance;
            };
            break;
        case _thoby:
            mp->scale[0] = (double) im->width / (2.0 * THOBY_K1_PARM * sin(a * THOBY_K2_PARM/2.0)) / mp->distance;
            break;
        case _stereographic:
            mp->scale[0] = (double) im->width / (4.0 * tan(a/4.0)) / mp->distance;
            break;
        default:
            PrintError ("SetInvMakeParams: Unsupported input image projection");
            // no way to report an error back to the caller...
            mp->scale[0] = 1;
            break;
        }
    mp->scale[1]    = mp->scale[0];

    mp->shear[0]  = im->cP.shear_x / im->height;
    mp->shear[1]  = im->cP.shear_y / im->width;
  

  mp->tilt[0] = DEG_TO_RAD(im->cP.tilt_x);
  mp->tilt[1] = DEG_TO_RAD(im->cP.tilt_y);
  mp->tilt[2] = DEG_TO_RAD(im->cP.tilt_z);
  mp->tilt[3] = im->cP.tilt_scale;
  
  mp->trans[0] = im->cP.trans_x;
  mp->trans[1] = im->cP.trans_y;
  mp->trans[2] = im->cP.trans_z;
  mp->trans[3] = DEG_TO_RAD(im->cP.trans_yaw);
  mp->trans[4] = DEG_TO_RAD(im->cP.trans_pitch);

  mp->test[0] = im->cP.test_p0;
  mp->test[1] = im->cP.test_p1;
  mp->test[2] = im->cP.test_p2;
  mp->test[3] = im->cP.test_p3;

  //  panoAdjustPrintMakeParams("Inverse 20",mp,im);

  mp->scale[0] = 1.0 / mp->scale[0];
  mp->scale[1]  = mp->scale[0];
  mp->horizontal  = -im->cP.horizontal_params[color];
  mp->vertical  = -im->cP.vertical_params[color];
  for(i=0; i<4; i++)
    mp->rad[i]  = im->cP.radial_params[color][i];
  mp->rad[5] = im->cP.radial_params[color][4];
  
  switch( im->cP.correction_mode & 3 )
    {
    case correction_mode_radial: mp->rad[4] = ((double)(im->width < im->height ? im->width : im->height) ) / 2.0;break;
    case correction_mode_vertical: 
    case correction_mode_deregister: mp->rad[4] = ((double) im->height) / 2.0;break;
    }

  mp->rot[0]    = mp->distance * PI;                // 180 in screenpoints
  mp->rot[1]    = im->yaw *  mp->distance * PI / 180.0;       //    rotation angle in screenpoints

  mp->perspect[0] = (void*)(mp->mt);
  mp->perspect[1] = (void*)&(mp->distance);

  //  panoAdjustPrintMakeParams("Invert 30",mp,im);

  i = 0;  // Stack counter
    

  if( im->cP.shear )
  {
    SetDesc( stack[i],shearInv,      mp->shear   ); i++;
  }
    
  if ( im->cP.horizontal )
  {
    SetDesc(stack[i],horiz,       &(mp->horizontal)); i++;
  }

  if (  im->cP.vertical)
  {
    SetDesc(stack[i],vert,        &(mp->vertical));   i++;
  }

  if( im->cP.tilt )
  {
    SetDesc( stack[i],tiltForward,      mp   ); i++;
  }

  // Perform radial correction

  if(   im->cP.radial )
  {
    switch( im->cP.correction_mode & 3)
    {
      case correction_mode_radial:   SetDesc(stack[i],inv_radial,mp->rad);  i++; break;
      case correction_mode_vertical: SetDesc(stack[i],inv_vertical,mp->rad);  i++; break;
      case correction_mode_deregister: break;
    }
  }
  
  SetDesc(  stack[i], resize,       mp->scale   ); i++; // Scale image

  //  printf("values %d %d\n", i, im->format);

  
  if(im->format     == _rectilinear)                  // rectilinear image
  {
    SetDesc(stack[i], sphere_tp_rect,   &(mp->distance) ); i++; // Convert rectilinear to spherical
  }
  else if (im->format   == _panorama)                 //  pamoramic image
  {
    SetDesc(stack[i], sphere_tp_pano,   &(mp->distance) ); i++; // Convert panoramic to spherical
  }
  else if (im->format   == _equirectangular)          //  equirectangular image
  {
    SetDesc(stack[i], sphere_tp_erect,  &(mp->distance) ); i++; // Convert equirectangular to spherical
  }
  else if (im->format   == _mirror)                   //  Mirror image
  {
    SetDesc(stack[i],   sphere_tp_mirror,        &(mp->distance) ); i++; // Convert mirror to spherical
  }
  else if (im->format   == _equisolid)                //  Fisheye equisolid image
  {

    SetDesc(stack[i], erect_lambertazimuthal,  &(mp->distance) ); i++; // Convert lambert to equirectangular
    SetDesc(stack[i], sphere_tp_erect,  &(mp->distance) ); i++; // Convert equirectangular to spherical
    //SetDesc(stack[i], sphere_tp_equisolid,  &(mp->distance) ); i++; // Convert equisolid to spherical
  }
  else if (im->format   == _orthographic)             //  Fisheye orthographic image
  {
    SetDesc(stack[i], sphere_tp_orthographic,  &(mp->distance) ); i++; // Convert orthographic to spherical
  }
  else if (im->format   == _thoby)             //  thoby projected image
  {
    SetDesc(stack[i], sphere_tp_thoby,  &(mp->distance) ); i++; // Convert thoby to spherical
  }
  else if (im->format   == _stereographic)             //  Fisheye stereographic image
  {
    SetDesc(stack[i], erect_stereographic,  &(mp->distance) ); i++; // Convert stereographic to spherical
    SetDesc(stack[i], sphere_tp_erect,  &(mp->distance) ); i++; // Convert equirectangular to spherical
  }

  //  printf("values %d %d\n", i, im->format);  
  SetDesc(  stack[i], persp_sphere,   mp->perspect  ); i++; // Perspective Control spherical Image
  SetDesc(  stack[i], erect_sphere_tp,  &(mp->distance) ); i++; // Convert spherical image to equirect.
  SetDesc(  stack[i], rotate_erect,   mp->rot     ); i++; // Rotate equirect. image horizontally

  if( im->cP.trans )
  {
    SetDesc( stack[i], plane_transfer_from_camera,      mp   ); i++;
  }

  // THESE ARE ALL FORWARD transforms
  if(pn->format == _rectilinear)                  // rectilinear panorama
  {
    SetDesc(stack[i], rect_erect,   &(mp->distance) ); i++; // Convert equirectangular to rectilinear
  }
  else if(pn->format == _panorama)
  {
    SetDesc(stack[i], pano_erect,   &(mp->distance) ); i++; // Convert equirectangular to Cylindrical panorama
  }
  else if(pn->format == _fisheye_circ || pn->format == _fisheye_ff )
  {
    SetDesc(stack[i], sphere_tp_erect,    &(mp->distance) ); i++; // Convert equirectangular to spherical
  }
  else if(pn->format == _mercator)
  {
    SetDesc(stack[i], mercator_erect,   &(mp->distance) ); i++; // Convert equirectangular to mercator
  }
  else if(pn->format == _millercylindrical)
  {
    SetDesc(stack[i], millercylindrical_erect,    &(mp->distance) ); i++; // Convert equirectangular to miller cylindrical
  }
  else if(pn->format == _panini)
  {
    SetDesc(stack[i], panini_erect,  &(mp->distance) ); i++; // Convert panini to sphere
  }
  else if(pn->format == _equipanini)
  {
    SetDesc(stack[i], equipanini_erect,  &(mp->distance) ); i++; // Convert equi panini to sphere
  }
  else if(pn->format == _panini_general)
  {
    SetDesc(stack[i],  panini_general_erect,  mp ); i++; // Convert general panini to sphere
  }
  else if(pn->format == _architectural)
  {
    SetDesc(stack[i], arch_erect,   &(mp->distance) ); i++; // Convert arch to sphere
  }
  else if(pn->format == _lambert)
  {
    SetDesc(stack[i], lambert_erect,    &(mp->distance) ); i++; // Convert equirectangular to lambert
  }
  else if(pn->format == _lambertazimuthal)
  {
    SetDesc(stack[i], lambertazimuthal_erect,   &(mp->distance) ); i++; // Convert equirectangular to lambert azimuthal
  }
  else if(pn->format == _hammer)
  {
    SetDesc(stack[i], hammer_erect,   &(mp->distance) ); i++; // Convert equirectangular to hammer
  }
  else if(pn->format == _trans_mercator)
  {
    SetDesc(stack[i], transmercator_erect,    &(mp->distance) ); i++; // Convert equirectangular to transverse mercator
  }
  else if(pn->format == _mirror)
  {
    SetDesc(stack[i], mirror_erect,    &(mp->distance) ); i++; // Convert equirectangular to mirror
  }
  else if(pn->format == _stereographic)
  {
    SetDesc(stack[i], stereographic_erect,    &(mp->distance) ); i++; // Convert equirectangular to stereographic
  }
    else if(pn->format == _sinusoidal)
  {
    SetDesc(stack[i], sinusoidal_erect,   &(mp->distance) ); i++; // Convert equirectangular to sinusoidal
  }
  else if(pn->format == _albersequalareaconic)
  {
    SetDesc(stack[i], albersequalareaconic_erect,   mp  ); i++; // Convert equirectangular to albersequalareaconic
  }
  else if(pn->format == _equisolid )
  {
    SetDesc(stack[i], sphere_tp_erect,    &(mp->distance) ); i++; // Convert equirectangular to spherical
    SetDesc(stack[i], equisolid_sphere_tp,    &(mp->distance) ); i++; // Convert spherical to equisolid
  }
  else if(pn->format == _orthographic )
  {
    SetDesc(stack[i], sphere_tp_erect,    &(mp->distance) ); i++; // Convert equirectangular to spherical
    SetDesc(stack[i], orthographic_sphere_tp,    &(mp->distance) ); i++; // Convert spherical to orthographic
  }
  else if(pn->format == _thoby )
  {
    SetDesc(stack[i], sphere_tp_erect,    &(mp->distance) ); i++; // Convert equirectangular to spherical
    SetDesc(stack[i], thoby_sphere_tp,    &(mp->distance) ); i++; // Convert spherical to thoby
  }
  else if(pn->format == _biplane)
  {
    SetDesc(stack[i], biplane_erect, mp ); i++;  // Convert equirectangular to biplane
  }
  else if(pn->format == _triplane)
  {
    SetDesc(stack[i], triplane_erect, mp ); i++;  // Convert equirectangular to biplane
  }  else if(pn->format == _equirectangular) 
  {
    // no conversion needed   
  }
  else 
  {
    PrintError("Projection type %d not supported, using equirectangular", pn->format);
  }
  
  stack[i].func = (trfn)NULL;
}

void SetInvMakeParamsCorrect( struct fDesc *stack, struct MakeParams *mp, Image *im , Image *pn, int color )
{
/* Thomas Rauscher, Sep 2005: Transfered the changes of Joost Nieuwenhuijse for MakeParams
   to the inverse function. This has broken the optimizer, now there are two functions.
*/

  Image imSel; /* create a tempory copy of the image to manipulate */
  memcpy( &imSel, im, sizeof(Image));

  if(im->cP.horizontal)
    {
        mp->horizontal = im->cP.horizontal_params[color];
    }
    else
    {
        mp->horizontal = 0;
    }

        if(im->cP.vertical)
    {
        mp->vertical = im->cP.vertical_params[color];
    }
    else
    {
        mp->vertical = 0;
    }

        if( (im->selection.left != 0) || (im->selection.top != 0) || (im->selection.bottom != 0) || (im->selection.right != 0) )
    {
        if(im->cP.cutFrame)
        {
                        imSel.width = im->selection.right  - im->selection.left;
                        imSel.height = im->selection.bottom - im->selection.top;

                        mp->horizontal += (im->selection.right  + im->selection.left - im->width)/2.0;
                        mp->vertical   += (im->selection.bottom + im->selection.top  - im->height)/2.0;

                        imSel.cP.horizontal_params[color] = mp->horizontal;
                        imSel.cP.vertical_params[color]   = mp->vertical;
        }
    }

        SetInvMakeParams( stack, mp, &imSel, pn, color );
}
        


// Add an alpha channel to the image, assuming rectangular or circular shape
// subtract frame 

void addAlpha( Image *im ){
        register int            x,y,c1;
        int                     framex, framey;
        register unsigned char  *src;
        
        src = *(im->data);
        framex = 0; framey = 0;
        
        if( im->cP.cutFrame ){
                if( im->cP.frame < 0 || im->cP.fwidth < 0 || im->cP.fheight < 0 ){      // Use supplied alpha channel
                        return;
                }
                
                if( im->cP.frame != 0 ){
                        framex = (im->width/2   > im->cP.frame ? im->cP.frame : 0);
                        framey = (im->height/2  > im->cP.frame ? im->cP.frame : 0);
                }
                else{
                        if( im->width > im->cP.fwidth )
                                framex = (im->width - im->cP.fwidth) / 2;
                        if( im->height > im->cP.fheight )
                                framey = (im->height - im->cP.fheight) / 2;
                }
        }


        if( im->bitsPerPixel == 32 || im->bitsPerPixel == 64 ) // leave 24/48 bit images unchanged
        {
                if( im->format != _fisheye_circ )               // Rectangle valid
                {
                        int yend = im->height - framey;
                        int xend = im->width  - framex;
                        
                        if( im->bitsPerPixel == 32 )
                        {
                                if( 0 != framey || 0 != framex )
                                {
                                        for(y = 0; y < im->height; y++)
                                        {
                                                c1 = y * im->bytesPerLine;
                                                for(x = 0; x < im->width; x++)
                                                        src[ c1 + 4 * x ] = 0;
                                        }
                                }
                                for(y = framey; y < yend; y++)
                                {
                                        c1 = y * im->bytesPerLine;
                                        for(x = framex; x < xend; x++)
                                                src[ c1 + 4 * x ] = UCHAR_MAX;
                                }
                        }
                        else // im->bitsPerPixel == 64
                        {
                                if( 0 != framey || 0 != framex )
                                {
                                        for(y = 0; y < im->height; y++)
                                        {
                                                c1 = y * im->bytesPerLine;
                                                for(x = 0; x < im->width; x++)
                                                        *((uint16_t*)(src + c1 + 8 * x )) = 0;
                                        }
                                }
                                for(y = framey; y < yend; y++)
                                {
                                        c1 = y * im->bytesPerLine;
                                        for(x = framex; x < xend; x++)
                                                *((uint16_t*)(src + c1 + 8 * x )) = USHRT_MAX;
                                }
                        }
                }
                else if( im->format == _fisheye_circ ) // Circle valid
                {
                        int topCircle   = ( im->height - im->width ) / 2;       // top of circle
                        int botCircle   = topCircle + im->width ;                       // bottom of circle
                        int r                   = ( im->width / 2 );                            // radius of circle
                        int x1, x2, h;
                        
                        if( im->bitsPerPixel == 32 )
                        {
                                for(y = 0; y < im->height  ; y++) 
                                {
                                        if( (y < topCircle) || (y > botCircle) )  // Always invalid
                                        {
                                                for(x = 0; x < im->width; x++)
                                                        src[y * im->bytesPerLine + 4 * x] = 0;
                                        }
                                        else
                                        {
                                                h       =       y - im->height/2;
                                                if( h*h > r*r ) h = r;

                                                x1 = (int) (r - sqrt( r*r - h*h ));
                                                if( x1 < 0 ) x1 = 0;
                                                x2 = (int) (r + sqrt( r*r - h*h ));
                                                if( x2 > im->width ) x2 = im->width;
                        
                                                for(x = 0; x < x1; x++)
                                                        src[y * im->bytesPerLine + 4 * x] = 0;
                                                for(x = x1; x < x2; x++)
                                                        src[y * im->bytesPerLine + 4 * x] = UCHAR_MAX;
                                                for(x = x2; x < im->width; x++)
                                                        src[y * im->bytesPerLine + 4 * x] = 0;
                                        }
                                }
                        }
                        else // im->bitsPerPixel == 64
                        {
                                for(y = 0; y < im->height  ; y++) 
                                {
                                        if( (y < topCircle) || (y > botCircle) )  // Always invalid
                                        {
                                                for(x = 0; x < im->width; x++)
                                                        *((uint16_t*)(src + y * im->bytesPerLine + 8 * x)) = 0;
                                        }
                                        else
                                        {
                                                h       =       y - im->height/2;
                                                if( h*h > r*r ) h = r;

                                                x1 = (int) (r - sqrt( r*r - h*h ));
                                                if( x1 < 0 ) x1 = 0;
                                                x2 = (int) (r + sqrt( r*r - h*h ));
                                                if( x2 > im->width ) x2 = im->width;
                        
                                                for(x = 0; x < x1; x++)
                                                        *((uint16_t*)(src + y * im->bytesPerLine + 8 * x)) = 0;
                                                for(x = x1; x < x2; x++)
                                                        *((uint16_t*)(src + y * im->bytesPerLine + 8 * x)) = USHRT_MAX;
                                                for(x = x2; x < im->width; x++)
                                                        *((uint16_t*)(src + y * im->bytesPerLine + 8 * x)) = 0;
                                        }
                                }
                        }
                } // mode
        }       // pixelsize
}


// Angular Distance for control point "num".
// Function distSphere computes an exact angular distance and the
// corresponding components in longitude/latitude directions.
// These are returned in a slightly strange manner (distance as the
// function result, components in a global array) to avoid changing the
// calling sequence of distSphere, which might unnecessarily break
// other code that we don't know about.

double distanceComponents[2];

double distSphere( int num ){
        double          x, y ;  // Coordinates of control point in panorama
        double          w2, h2;
        int j;
        Image sph;
        int n[2];
        struct  MakeParams      mp;
        struct  fDesc           stack[15];
        CoordInfo b[2];
        CoordInfo cp;
        double lat[2], lon[2];  // latitude & longitude
        double dlon;
        double dangle;
        double dist;
        double radiansToPixelsFactor;

        // Factor to convert angular error in radians to equivalent in pixels
        
        radiansToPixelsFactor = optInfo->pano.width / (optInfo->pano.hfov * (PI/180.0));
        
        // Get image position in imaginary spherical image
        
        SetImageDefaults( &sph );
        
        sph.width                       = 360;
        sph.height                      = 180;
        sph.format                      = _equirectangular;
        sph.hfov                        = 360.0;
        
        n[0] = optInfo->cpt[num].num[0];
        n[1] = optInfo->cpt[num].num[1];
        
        // Calculate coordinates using equirectangular mapping to get longitude/latitude

        for(j=0; j<2; j++){
                SetInvMakeParams( stack, &mp, &optInfo->im[ n[j] ], &sph, 0 );
                
                h2      = (double)optInfo->im[ n[j] ].height / 2.0 - 0.5;
                w2      = (double)optInfo->im[ n[j] ].width  / 2.0 - 0.5;
                
                
                execute_stack_new(      (double)optInfo->cpt[num].x[j] - w2,            // cartesian x-coordinate src
                                                (double)optInfo->cpt[num].y[j] - h2,            // cartesian y-coordinate src
                                                &x, &y, stack);

                x = DEG_TO_RAD( x ); 
                y = DEG_TO_RAD( y ) + PI/2.0;

                // x is now in the range -PI to +PI, and y is 0 to PI
                lat[j] = y;
                lon[j] = x;

                b[j].x[0] =   sin(x) * sin( y );
                b[j].x[1] =   cos( y );
                b[j].x[2] = - cos(x) * sin(y);
        }

        dlon = lon[0]-lon[1];
        if (dlon < -PI) dlon += 2.0*PI;
        if (dlon > PI) dlon -= 2.0*PI;
        distanceComponents[0] = (dlon*sin(0.5*(lat[0]+lat[1]))) * radiansToPixelsFactor;
        distanceComponents[1] = (lat[0]-lat[1]) * radiansToPixelsFactor;

        // The original acos formulation (acos(SCALAR_PRODUCT(&b[0],&b[1]))
        // is inaccurate for angles near 0, because it essentially requires finding eps
        // based on the value of 1-eps^2/2.  The asin formulation is much more
        // accurate under these conditions.

        CROSS_PRODUCT(&b[0],&b[1],&cp);
        dangle = asin(ABS_VECTOR(&cp));
        if (SCALAR_PRODUCT(&b[0],&b[1]) < 0.0) dangle = PI - dangle;
        dist = dangle * radiansToPixelsFactor;
        
        // Diagnostics to help debug various calculation errors.
        // Do not delete this code --- it has been needed surprisingly often.
#if 0   
        {       double olddist;
                olddist = acos( SCALAR_PRODUCT( &b[0], &b[1] ) ) * radiansToPixelsFactor;
//              if (adjustLogFile != 0 && abs(dist-olddist) > 1.0) {
                if (adjustLogFile != 0 && num < 5) {
                        fprintf(adjustLogFile,"***** DIST ***** dCoord = %g %g, lonlat0 = %g %g, lonlat1 = %g %g, dist=%g, olddist=%g, sumDcoordSq=%g, distSq=%g\n",
                                                                  distanceComponents[0],distanceComponents[1],lon[0],lat[0],lon[1],lat[1],dist,olddist,
                                                                  distanceComponents[0]*distanceComponents[0]+distanceComponents[1]*distanceComponents[1],dist*dist);
                }
        }
#endif

        return dist;
}



void pt_getXY(int n, double x, double y, double *X, double *Y){
        struct  MakeParams      mp;
        struct  fDesc           stack[15];
        double h2,w2;

        SetInvMakeParams( stack, &mp, &optInfo->im[ n ], &optInfo->pano, 0 );
        h2      = (double)optInfo->im[ n ].height / 2.0 - 0.5;
        w2      = (double)optInfo->im[ n ].width  / 2.0 - 0.5;


        execute_stack_new(      x - w2, y - h2, X, Y, stack);
}

// Return distance of points from a line
// The line through the two farthest apart points is calculated
// Returned is the sum distance squared of the other two points from the line
double distsqLine(int N0, int N1){
        double x[4],y[4], del, delmax, A, B, C, mu, d0, d1;
        int n0, n1, n2=-1, n3=-1, i, k;

        pt_getXY(optInfo->cpt[N0].num[0], (double)optInfo->cpt[N0].x[0], (double)optInfo->cpt[N0].y[0], &x[0], &y[0]);
        pt_getXY(optInfo->cpt[N0].num[1], (double)optInfo->cpt[N0].x[1], (double)optInfo->cpt[N0].y[1], &x[1], &y[1]);
        pt_getXY(optInfo->cpt[N1].num[0], (double)optInfo->cpt[N1].x[0], (double)optInfo->cpt[N1].y[0], &x[2], &y[2]);
        pt_getXY(optInfo->cpt[N1].num[1], (double)optInfo->cpt[N1].x[1], (double)optInfo->cpt[N1].y[1], &x[3], &y[3]);

        delmax = 0.0;
        n0 = 0; n1 = 1;

        for(i=0; i<4; i++){
                for(k=i+1; k<4; k++){
                        del = (x[i]-x[k])*(x[i]-x[k])+(y[i]-y[k])*(y[i]-y[k]);
                        if(del>delmax){
                                n0=i; n1=k; delmax=del;
                        }
                }
        }
        if(delmax==0.0) return 0.0;

        for(i=0; i<4; i++){
                if(i!= n0 && i!= n1){
                        n2 = i;
                        break;
                }
        }
        for(i=0; i<4; i++){
                if(i!= n0 && i!= n1 && i!=n2){
                        n3 = i;
                }
        }


        A=y[n1]-y[n0]; B=x[n0]-x[n1]; C=y[n0]*(x[n1]-x[n0])-x[n0]*(y[n1]-y[n0]);

        mu=1.0/sqrt(A*A+B*B);

        d0 = (A*x[n2]+B*y[n2]+C)*mu;
        d1 = (A*x[n3]+B*y[n3]+C)*mu;
        distanceComponents[0] = d0;
        distanceComponents[1] = d1;

        return d0*d0 + d1*d1;

}


// Calculate the distance of Control Point "num" between two images
// in final pano.  This is the old distSquared.

double rectDistSquared( int num ) 
{
        double          x[2], y[2];                             // Coordinates of control point in panorama
        double          w2, h2;
        int j, n[2];
        double result;

        struct  MakeParams      mp;
        struct  fDesc           stack[15];

        

        n[0] = optInfo->cpt[num].num[0];
        n[1] = optInfo->cpt[num].num[1];
        
        // Calculate coordinates x/y in panorama

        for(j=0; j<2; j++)
        {
                SetInvMakeParams( stack, &mp, &optInfo->im[ n[j] ], &optInfo->pano, 0 );
                
                h2      = (double)optInfo->im[ n[j] ].height / 2.0 - 0.5;
                w2      = (double)optInfo->im[ n[j] ].width  / 2.0 - 0.5;
                

                execute_stack_new(      (double)optInfo->cpt[num].x[j] - w2,            // cartesian x-coordinate src
                                                (double)optInfo->cpt[num].y[j] - h2,            // cartesian y-coordinate src
                                                &x[j], &y[j], stack);
                // test to check if inverse works
#if 0
                {
                        double xt, yt;
                        struct  MakeParams      mtest;
                        struct  fDesc           stacktest[15];
                        SetMakeParams( stacktest, &mtest, &optInfo->im[ n[j] ], &optInfo->pano, 0 );
                        execute_stack_new(      x[j],           // cartesian x-coordinate src
                                                        y[j],           // cartesian y-coordinate src
                                                &xt, &yt, stacktest);
                        
                        printf("x= %lg, y= %lg,  xb = %lg, yb = %lg \n", optInfo->cpt[num].x[j], optInfo->cpt[num].y[j], xt+w2, yt+h2);  
                        
                }
#endif
        }
        
        
//      printf("Coordinates 0:   %lg:%lg        1:      %lg:%lg\n",x[0] + g->pano->width/2,y[0]+ g->pano->height/2, x[1] + g->pano->width/2,y[1]+ g->pano->height/2);


        // take care of wrapping and points at edge of panorama
        
        if( optInfo->pano.hfov == 360.0 )
        {
                double delta = abs( x[0] - x[1] );
                
                if( delta > optInfo->pano.width / 2 )
                {
                        if( x[0] < x[1] )
                                x[0] += optInfo->pano.width;
                        else
                                x[1] += optInfo->pano.width;
                }
        }


        switch( optInfo->cpt[num].type )                // What do we want to optimize?
        {
                case 1:                 // x difference
                        result = ( x[0] - x[1] ) * ( x[0] - x[1] );
                        break;
                case 2:                 // y-difference
                        result =  ( y[0] - y[1] ) * ( y[0] - y[1] );
                        break;
                default:
                        result = ( y[0] - y[1] ) * ( y[0] - y[1] ) + ( x[0] - x[1] ) * ( x[0] - x[1] ); // square of distance
                        distanceComponents[0] = y[0] - y[1];
                        distanceComponents[1] = x[0] - x[1];

                        break;
        }
        

        return result;
}

/// huber() is an M-Estimator function. Using an M-Estimator might
/// work better if the control points contain outliers (eg. from autopano).
/// this implementation accepts normal, non squared errors, and return non-squared errors,
/// contrary to the definition in the literature (where the square is
/// included in the function)

static double fcnPanoHuberSigma = 0; // sigma for Huber M-estimator. 0 disables M-estimator

void setFcnPanoHuberSigma(double sigma)
{
    fcnPanoHuberSigma = sigma;
}

double huber(double x, double sigma)
{
    if (abs(x) < sigma)
        return x;
    else
        return sqrt(2.0*sigma*abs(x) - sigma*sigma);
}



/// (function distSquared2 has been removed -- it was unused and redundant)

/// EvaluateControlPointErrorAndComponents is the central point-of-contact
/// for determining the error for a specified control point pair.
///
/// Arguments are
///   num               identifies the control point pair
///   *errptr           returns a single error measure (distance)
///   errComponent[*]   returns two components of that error, as nearly orthogonal
///                       as possible
/// Return value is a success flag: 0 = successful, otherwise not.

int     EvaluateControlPointErrorAndComponents ( int num, double *errptr, double errComponent[2]) {
        int j;
        int result;
        switch(optInfo->cpt[num].type){
                case 0: // normal control points
                        // Jim May 2004. 
                        // Optimizing cylindrical and rectilinear projection by calculating 
                        // distance error in pixel coordinates of the rendered image.
                        // When using angular (spherical) distance for these projections, 
                        // larger errors are generated the further control points are from 
                        // the center.
                        // In theory by optimizing in pixel coordinates all errors will be 
                        // distributed over the image.  This is true.
                        // In practice I have found that optimize large field of view 
                        // rectilinear projection images failed to resolve nicely if the 
                        // parameters were not very close to start with.  I leave the 
                        // code here for others to play with and maybe get better results.
                /*  if(optInfo->pano.format == _rectilinear || g->pano.format == _panorama)
                        {
                                *errptr = sqrt(rectDistSquared(num));
                                errComponent[0] = distanceComponents[0];
                                errComponent[1] = distanceComponents[1];
                                result = 0;
                                break;
                        }
                        else //  _equirectangular, fisheye, spherical, mirror
                        {  */
                                *errptr = distSphere(num);
                                errComponent[0] = distanceComponents[0];
                                errComponent[1] = distanceComponents[1];
                                result = 0;
                                break;
                        //}
                case 1: // vertical
                case 2: // horizontal
                                *errptr = sqrt(rectDistSquared(num));
                                errComponent[0] = *errptr;
                                errComponent[1] = 0.0;
                                result = 0;
                                break;
                default:// t+ controls = lines = sets of two control point pairs
                                *errptr = 0.0;  // in case there is no second pair
                                errComponent[0] = 0.0;
                                errComponent[1] = 0.0;
                                result = 1;
                                for(j=0; j<optInfo->numPts; j++){
                                        if(j!=num && optInfo->cpt[num].type == optInfo->cpt[j].type){
                                                *errptr = sqrt(distsqLine(num,j));
//                                              errComponent[0] = *errptr;
//                                              errComponent[1] = 0.0;
                                                errComponent[0] = distanceComponents[0];
                                                errComponent[1] = distanceComponents[1];
                                                result = 0;
                                                break;
                                        }
                                }
                                break;
        }
        return result;
}
        
/// This distSquared is a convenience function, to be passed into
/// WriteResults in the usual fashion, to avoid having to change
/// other codes that call WriteResults.  It replaces the old distSquared,
/// which has been renamed rectDistSquared and is now used only
/// internally by EvaluateControlPointErrorAndComponents.

double  distSquared (int num ) {
        double result;
        double junk[2];
        EvaluateControlPointErrorAndComponents (num, &result, junk);
        return result*result;
}

/// Function fcnPano calculates a vector of control points errors,
/// for use during optimization.  See lmdif.c and optimize.c for
/// a description of its arguments.  The helper functions that appear here
/// allow to control the new capability, while preserving also the
/// old calling sequences.

int fcnPanoNperCP = 1; // number of functions per control point, 1 or 2

void setFcnPanoNperCP (int i) {
        fcnPanoNperCP = i;
}

int getFcnPanoNperCP() {
        return fcnPanoNperCP;
}

void setFcnPanoDoNotInitAvgFov() { // must be called after iflag = -100 call to fcnPano
        needInitialAvgFov = 0;
}

void forceFcnPanoReinitAvgFov() { // applies to next call to fcnPano
        needInitialAvgFov = 1;
}

int fcnPano(int m, int n, double x[], double fvec[], int *iflag)
{
        int i;
        static int numIt;
        double result;
        int iresult;
        double junk;
        double junk2[2];
        
        if( *iflag == -100 ){ // reset
                numIt = 0;
                needInitialAvgFov = 1;
                infoDlg ( _initProgress, "Optimizing Variables" );
#if ADJUST_LOGGING_ENABLED
                if ((adjustLogFile = fopen(ADJUST_LOG_FILENAME,"a")) <= 0) {
                        PrintError("Cannot Open Log File");
                        adjustLogFile = 0;
                }
#endif
                return 0;
        }
        if( *iflag == -99 ){ // 
                infoDlg ( _disposeProgress, "" );
                if (adjustLogFile != 0) {
                        result = 0.0;
                        for( i=0; i < m; i++)
                        {
                                result += fvec[i]*fvec[i] ;
                        }
                        result = sqrt( result/ (double)m ) * sqrt((double)fcnPanoNperCP); // to approximate total distance vs dx, dy
                        fprintf(adjustLogFile,"At iflag=-99 (dispose), NperCP=%d, m=%d, n=%d, err = %g, x= \n",
                                              fcnPanoNperCP,m,n,result);
                        for (i=0; i<n; i++) {
                                fprintf(adjustLogFile,"\t%20.10g",x[i]);
                        }
                        fprintf(adjustLogFile,"\n   fvec = \n");
                        for (i=0; i<m; i++) {
                                fprintf(adjustLogFile,"\t%20.10g",fvec[i]);
                                if (((i+1) % fcnPanoNperCP) == 0) fprintf(adjustLogFile,"\n");
                        }
                        fprintf(adjustLogFile,"\n");
                        fclose(adjustLogFile);
                }
                return 0;
        }


        if( *iflag == 0 )
        {
                char message[256];
                
                result = 0.0;
                for( i=0; i < m; i++)
                {
                        result += fvec[i]*fvec[i] ;
                }
                result = sqrt( result/ (double)m ) * sqrt((double)fcnPanoNperCP); // to approximate total distance vs dx, dy

                sprintf( message,"Strategy %d\nAverage (rms) distance between Controlpoints \nafter %d iteration(s): %25.15g units", getFcnPanoNperCP(), numIt,result);//average);
                numIt += 1; // 10;
                if( !infoDlg ( _setProgress,message ) )
                        *iflag = -1;

                if (adjustLogFile != 0) {
                        fprintf(adjustLogFile,"At iteration %d, iflag=0 (print), NperCP=%d, m=%d, n=%d, err = %g, x= \n",
                                              numIt,fcnPanoNperCP,m,n,result);
                        for (i=0; i<n; i++) {
                                fprintf(adjustLogFile,"\t%20.10g",x[i]);
                        }
                        fprintf(adjustLogFile,"\n   fvec = \n");
                        for (i=0; i<m; i++) {
                                fprintf(adjustLogFile,"\t%20.10g",fvec[i]);
                                if (((i+1) % fcnPanoNperCP) == 0) fprintf(adjustLogFile,"\n");
                        }
                        fprintf(adjustLogFile,"\n");
                        fflush(adjustLogFile);
                }

                return 0;
        }

        // Set Parameters

        SetAlignParams( x ) ;

        if (needInitialAvgFov) {
                initialAvgFov = avgfovFromSAP;
                needInitialAvgFov = 0;
                if (adjustLogFile != 0) {
                        fprintf(adjustLogFile,"setting initialAvgFov = %g\n",initialAvgFov);
                        fflush(adjustLogFile);
                }
        }

        if (adjustLogFile != 0) {
                fprintf(adjustLogFile,"entering fcnPano, m=%d, n=%d, initialAvgFov=%g, avgfovFromSAP=%g, x = \n",
                                      m,n, initialAvgFov,avgfovFromSAP);
                for (i=0; i<n; i++) {
                        fprintf(adjustLogFile,"\t%20.10g",x[i]);
                }
                fprintf(adjustLogFile,"\n");
                fflush(adjustLogFile);
        }

        // Calculate distances

        iresult = 0;
        for( i=0; i < optInfo->numPts; i++){
                if (fcnPanoNperCP == 1) {
                        EvaluateControlPointErrorAndComponents ( i, &fvec[iresult], &junk2[0]);
        } else {
                        EvaluateControlPointErrorAndComponents ( i, &junk, &fvec[iresult]);
            if (fcnPanoHuberSigma) {
                fvec[iresult] = huber(fvec[iresult], fcnPanoHuberSigma);
                fvec[iresult+1] = huber(fvec[iresult+1], fcnPanoHuberSigma);
            }
                }
                
                // Field-of-view stabilization.  Applying here means that the
                // errors seen by the optimizer may be different from those finally
                // reported, by the same factor for all errors.  This introduces
                // the possibility of confusion for people who are paying really
                // close attention to the optimizer's periodic output versus the
                // final result.  However, it seems like the right thing to do
                // because then the final reported errors will correspond to the
                // user's settings for pano size, total fov etc. 
                
                if ((initialAvgFov / avgfovFromSAP) > 1.0) {
                        fvec[iresult] *= initialAvgFov / avgfovFromSAP;
                }
                iresult += 1;
                if (fcnPanoNperCP == 2) {
                        if ((initialAvgFov / avgfovFromSAP) > 1.0) {
                                fvec[iresult] *= initialAvgFov / avgfovFromSAP;
                        }
                        iresult += 1;
                }               
        }
        
        // If not enough control points are provided, then fill out
        // the function vector with copies of the average error
        // for the actual control points.

        result = 0.0;
        for (i=0; i < iresult; i++) {
                result += fvec[i]*fvec[i];
        }
        result = sqrt(result/(double)iresult);
        for (i=iresult; i < m; i++) {
                fvec[i] = result;
        }

        if (adjustLogFile != 0) {
                result *= sqrt((double)fcnPanoNperCP);
                fprintf(adjustLogFile,"leaving fcnPano, m=%d, n=%d, err=%g, fvec = \n",m,n,result);
                for (i=0; i<m; i++) {
                        fprintf(adjustLogFile,"\t%20.10g",fvec[i]);
                        if (((i+1) % fcnPanoNperCP) == 0) fprintf(adjustLogFile,"\n");
                }
                fprintf(adjustLogFile,"\n");
                fflush(adjustLogFile);
        }

        return 0;
}



// Find Colour correcting polynomial matching the overlap of src and buf
// using least square fit.
// Each RGB-Channel is fitted using the relation  
//      buf = coeff[0] * src + coeff[1]
#if 1
void GetColCoeff( Image *src, Image *buf, double ColCoeff[3][2] ){
        register int            x,y,c1,c2,i, numPts;
        double                  xy[3], xi[3], xi2[3], yi[3], xav[3], yav[3];
        register unsigned char  *source, *buff;
        int                     BitsPerChannel,bpp;


        
        GetBitsPerChannel( src, BitsPerChannel );
        bpp = src->bitsPerPixel/8;
        

        source = *(src->data);
        buff   = *(buf->data);
        
        for(i=0;i<3;i++){
                xy[i] = xi[i] = xi2[i] = yi[i] = 0.0;
        }
        numPts = 0;     

        if( BitsPerChannel == 8 ){
                for( y=2; y<src->height-2; y++){
                        c1 = y * src->bytesPerLine;
                        for( x=2; x<src->width-2; x++){
                                c2 = c1 + x*bpp;
                                if( source[c2] != 0  &&  buff[c2] != 0 ){ // &&   // In overlap region?
                                    //(source[c2] != UCHAR_MAX  ||  buff[c2] != UCHAR_MAX)){ // above seam?
                                        if( pt_average( source+c2, src->bytesPerLine, xav, 1 ) &&
                                            pt_average( buff+c2, src->bytesPerLine, yav, 1 ) ){
                                                numPts++;
                                                for( i=0; i<3; i++){
                                                        xi[i]   += xav[i];
                                                        yi[i]   += yav[i];
                                                        xi2[i]  += xav[i]*xav[i];
                                                        xy[i]   += xav[i]*yav[i];
                                                }
                                        }
                                }
                        }
                }
        }else{//16
                for( y=1; y<src->height-1; y++){
                        c1 = y * src->bytesPerLine;
                        for( x=1; x<src->width-1; x++){
                                c2 = c1 + x*bpp;
                                if( *((uint16_t*)(source + c2)) != 0  &&  *((uint16_t*)(buff + c2)) != 0 ) { //&& // In overlap region?
                                 //( *((uint16_t*)(source + c2)) != USHRT_MAX  ||  *((uint16_t*)(buff + c2)) != USHRT_MAX ) ){ // above seam?
                                        if( pt_average( source + c2, src->bytesPerLine, xav, 2 ) &&
                                            pt_average( buff + c2, src->bytesPerLine, yav, 2 )){
                                                numPts++;
                                                for( i=0; i<3; i++){
                                                        xi[i]   += xav[i];
                                                        yi[i]   += yav[i];
                                                        xi2[i]  += xav[i]*xav[i];
                                                        xy[i]   += xav[i]*yav[i];
                                                }
                                        }
                                }
                        }
                }
        }
                
        
        if( numPts > 0 ){
                for( i=0; i<3; i++){
                        ColCoeff[i][0] = ( numPts * xy[i] - xi[i] * yi[i] ) / ( numPts * xi2[i] - xi[i]*xi[i] );
                        ColCoeff[i][1] = ( xi2[i] * yi[i] - xy[i] * xi[i] ) / ( numPts * xi2[i] - xi[i]*xi[i] );
                }
        }else{
                for( i=0; i<3; i++){
                        ColCoeff[i][0] = 1.0;
                        ColCoeff[i][1] = 0.0;
                }
        }
}
#endif
// Average 9 pixels
int pt_average( uint8_t* pixel, int BytesPerLine, double rgb[3], int bytesPerChannel ){
        int x, y, i;
        uint8_t *px;
        double sum = 1.0 + 4 * 0.5 + 8 * 0.2 + 8 * 0.1 ;//2.6;
#if 0
        double bl[3][3] =      {{ 0.1, 0.3, 0.1}, // Blurr overlap using this matrix
                                { 0.3, 1.0, 0.3},
                                { 0.1, 0.3, 0.1}};

#endif
        double bl[5][5] =      {{ 0.0, 0.1, 0.2, 0.1, 0.0},
                                { 0.1, 0.2, 0.5, 0.2, 0.1},
                                { 0.2, 0.5, 1.0, 0.5, 0.2},
                                { 0.1, 0.2, 0.5, 0.2, 0.1},
                                { 0.0, 0.1, 0.2, 0.1, 0.0}};


        rgb[0] = rgb[1] = rgb[2] = 0.0;
        if( bytesPerChannel != 1 ) return -1;

        for(y=0; y<5; y++){
                for(x=0; x<5; x++){
                        px = pixel + (y-2)*BytesPerLine + x-2;
                        if( *px == 0 ) return 0;
                        rgb[0] +=  *(++px) * bl[y][x];
                        rgb[1] +=  *(++px) * bl[y][x];
                        rgb[2] +=  *(++px) * bl[y][x];
                }
        }
        for( i=0; i<3; i++) rgb[i]/=sum;
        return 0;

}


#if 0

// Backup

// Find Colour correcting polynomial matching the overlap of src and buf
// using least square fit.
// Each RGB-Channel is fitted using the relation  
//      buf = coeff[0] * src + coeff[1]

void GetColCoeff( Image *src, Image *buf, double ColCoeff[3][2] )
{
        register int x,y,c1,c2,i, numPts;
        double xy[3], xi[3], xi2[3], yi[3];
        register unsigned char *source, *buff;
        int             BitsPerChannel,bpp;
        
        GetBitsPerChannel( src, BitsPerChannel );
        bpp = src->bitsPerPixel/8;
        

        source = *(src->data);
        buff   = *(buf->data);
        for(i=0;i<3;i++)
        {
                xy[i] = xi[i] = xi2[i] = yi[i] = 0.0;
        }
        numPts = 0;     

        if( BitsPerChannel == 8 )
        {
                for( y=0; y<src->height; y++)
                {
                        c1 = y * src->bytesPerLine;
                        for( x=0; x<src->width; x++)
                        {
                                c2 = c1 + x*bpp;
                                if( source[c2] != 0  &&  buff[c2] != 0 ) // In overlap region?
                                {
                                        numPts++;
                                        for( i=0; i<3; i++)
                                        {
                                                c2++;
                                                xi[i]   += (double)source[c2];
                                                yi[i]   += (double)buff[c2];
                                                xi2[i]  += ((double)source[c2])*((double)source[c2]);
                                                xy[i]   += ((double)source[c2])*((double)buff[c2]);
                                        }
                                }
                        }
                }
        }
        else // 16
        {
                for( y=0; y<src->height; y++)
                {
                        c1 = y * src->bytesPerLine;
                        for( x=0; x<src->width; x++)
                        {
                                c2 = c1 + x*bpp;
                                if( *((uint16_t*)(source + c2)) != 0  &&  *((uint16_t*)(buff + c2)) != 0 ) // In overlap region?
                                {
                                        numPts++;
                                        for( i=0; i<3; i++)
                                        {
                                                c2++;
                                                xi[i]   += (double) *((uint16_t*)(source + c2));
                                                yi[i]   += (double) *((uint16_t*)(buff + c2));
                                                xi2[i]  += ((double) *((uint16_t*)(source + c2)))*((double) *((uint16_t*)(source + c2)));
                                                xy[i]   += ((double) *((uint16_t*)(source + c2)))*((double) *((uint16_t*)(buff + c2)));
                                        }
                                }
                        }
                }
        }
                
        
        if( numPts > 0 )
        {
                for( i=0; i<3; i++)
                {
                        ColCoeff[i][0] = ( numPts * xy[i] - xi[i] * yi[i] ) / ( numPts * xi2[i] - xi[i]*xi[i] );
                        ColCoeff[i][1] = ( xi2[i] * yi[i] - xy[i] * xi[i] ) / ( numPts * xi2[i] - xi[i]*xi[i] );
                }
        }
        else
        {
                for( i=0; i<3; i++)
                {
                        ColCoeff[i][0] = 1.0;
                        ColCoeff[i][1] = 0.0;
                }
        }
}

#endif


// Colourcorrect the image im using polynomial coefficients ColCoeff
// Each RGB-Channel is corrected using the relation  
//      new = coeff[0] * old + coeff[1]

void ColCorrect( Image *im, double ColCoeff[3][2] )
{
        register int x,y, c1, c2, i;
        register unsigned char* data;
        register double result;
        int bpp, BitsPerChannel;
        
        GetBitsPerChannel( im, BitsPerChannel );
        bpp = im->bitsPerPixel/8;

        data = *(im->data);

        if( BitsPerChannel == 8 )
        {
                for( y=0; y<im->height; y++)
                {
                        c1 = y * im->bytesPerLine;
                        for( x=0; x<im->width; x++ )
                        {
                                c2 = c1 + x * bpp;
                                if( data[ c2 ] != 0 ) // Alpha channel set
                                {
                                        for( i=0; i<3; i++)
                                        {
                                                c2++;
                                                result = ColCoeff[i][0] * data[ c2 ] + ColCoeff[i][1];
                                                DBL_TO_UC( data[ c2 ], result );
                                        }
                                }
                        }
                }
        }
        else // 16
        {
                for( y=0; y<im->height; y++)
                {
                        c1 = y * im->bytesPerLine;
                        for( x=0; x<im->width; x++ )
                        {
                                c2 = c1 + x * bpp;
                                if( *((uint16_t*)(data + c2 )) != 0 ) // Alpha channel set
                                {
                                        for( i=0; i<3; i++)
                                        {
                                                c2++;
                                                result = ColCoeff[i][0] * *((uint16_t*)(data + c2 )) + ColCoeff[i][1];
                                                DBL_TO_US( *((uint16_t*)(data + c2 )) , result );
                                        }
                                }
                        }
                }
        }
}


void SetAdjustDefaults( aPrefs *prefs )
{

    prefs->magic        =   50;                 //  File validity check, must be 50
    prefs->mode         =   _insert;            //  
    
    SetImageDefaults( &(prefs->im) );
    SetImageDefaults( &(prefs->pano) );
    
    SetStitchDefaults( &(prefs->sBuf) );    

    memset( &(prefs->scriptFile), 0, sizeof( fullPath ) );
    
    prefs->nt           = 0;
    prefs->ts           = NULL;
    prefs->td           = NULL;
    
    prefs->interpolator = _spline36;
    prefs->gamma        = 1.0;
    prefs->fastStep     = FAST_TRANSFORM_STEP_NONE;
}

                                

void    DisposeAlignInfo( struct AlignInfo *g )
{
        if(g->im != NULL) free(g->im);
        if(g->opt!= NULL) free(g->opt);
        if(g->cpt!= NULL) free(g->cpt);
        if(g->t  != NULL) free(g->t);
        if(g->cim != NULL) free(g->cim);
}




// Set global preferences structures using LM-params

int     SetAlignParams( double *x )
{
        // Set Parameters
        int i,j,k;
        double sumfov = 0.0;
        
        j = 0;
        for( i=0; i<optInfo->numIm; i++ ){

                if( (k = optInfo->opt[i].yaw) > 0 ){
                        if( k == 1 ){   optInfo->im[i].yaw  = x[j++]; NORM_ANGLE( optInfo->im[i].yaw );
                        }else{  optInfo->im[i].yaw  = optInfo->im[k-2].yaw; }
                }
                if( (k = optInfo->opt[i].pitch) > 0 ){
                        if( k == 1 ){   optInfo->im[i].pitch  =       x[j++]; NORM_ANGLE( optInfo->im[i].pitch );
                        }else{  optInfo->im[i].pitch  =       optInfo->im[k-2].pitch;       }
                }
                if( (k = optInfo->opt[i].roll) > 0 ){
                        if( k == 1 ){   optInfo->im[i].roll  =        x[j++]; NORM_ANGLE( optInfo->im[i].roll );
                        }else{  optInfo->im[i].roll  =        optInfo->im[k-2].roll;        }
                }
                if( (k = optInfo->opt[i].hfov) > 0 ){
                        if( k == 1 ){   
                                optInfo->im[i].hfov  =        x[j++]; 
                                if( optInfo->im[i].hfov < 0.0 )
                                        optInfo->im[i].hfov = - optInfo->im[i].hfov;
                        }else{  optInfo->im[i].hfov  = optInfo->im[k-2].hfov; }
                }
                sumfov += optInfo->im[i].hfov;
                if( (k = optInfo->opt[i].a) > 0 ){
                        if( k == 1 ){ optInfo->im[i].cP.radial_params[0][3]  =  x[j++] / C_FACTOR;
                        }else{  optInfo->im[i].cP.radial_params[0][3] = optInfo->im[k-2].cP.radial_params[0][3];}
                }
                if( (k = optInfo->opt[i].b) > 0 ){
                        if( k == 1 ){ 
                          optInfo->im[i].cP.radial_params[0][2]  =  x[j++] / C_FACTOR;
                        }else{  
                          optInfo->im[i].cP.radial_params[0][2] = optInfo->im[k-2].cP.radial_params[0][2];
                        }
                }
                if( (k = optInfo->opt[i].c) > 0 ){
                        if( k == 1 ){ optInfo->im[i].cP.radial_params[0][1]  =  x[j++] / C_FACTOR;
                        }else{  optInfo->im[i].cP.radial_params[0][1] = optInfo->im[k-2].cP.radial_params[0][1];}
                }
                if( (k = optInfo->opt[i].d) > 0 ){
                        if( k == 1 ){ optInfo->im[i].cP.horizontal_params[0]  = x[j++];
                        }else{  optInfo->im[i].cP.horizontal_params[0] = optInfo->im[k-2].cP.horizontal_params[0];}
                }
                if( (k = optInfo->opt[i].e) > 0 ){
                        if( k == 1 ){ optInfo->im[i].cP.vertical_params[0]  =   x[j++];
                        }else{  optInfo->im[i].cP.vertical_params[0] = optInfo->im[k-2].cP.vertical_params[0];}
                }
                // tilt
                if( (k = optInfo->opt[i].tiltXopt) > 0 ){
                    if( k == 1 ){ optInfo->im[i].cP.tilt_x  = x[j++]; //NORM_ANGLE_RAD(optInfo->im[i].cP.tilt_x);
                    }else{  optInfo->im[i].cP.tilt_x = optInfo->im[k-2].cP.tilt_x;}
                }
                if( (k = optInfo->opt[i].tiltYopt) > 0 ){
                    if( k == 1 ){ optInfo->im[i].cP.tilt_y  = x[j++]; //NORM_ANGLE_RAD(optInfo->im[i].cP.tilt_y);
                        }else{  optInfo->im[i].cP.tilt_y = optInfo->im[k-2].cP.tilt_y;}
                }
                if( (k = optInfo->opt[i].tiltZopt) > 0 ){
                    if( k == 1 ){ optInfo->im[i].cP.tilt_z  =x[j++]; //NORM_ANGLE_RAD(optInfo->im[i].cP.tilt_z);
                        }else{  optInfo->im[i].cP.tilt_z = optInfo->im[k-2].cP.tilt_z;}
                }
                if( (k = optInfo->opt[i].tiltScaleOpt) > 0 ){
                    if( k == 1 ) { 
                        optInfo->im[i].cP.tilt_scale  = x[j++]; 
                        if (optInfo->im[i].cP.tilt_scale == 0) {
                            optInfo->im[i].cP.tilt_scale = 0.001; //make sure it never becomes zero
                        }
                        optInfo->im[i].cP.tilt_scale = fabs(optInfo->im[i].cP.tilt_scale);
                        /*
                        if (optInfo->im[i].cP.tilt_scale > 10) {
                            optInfo->im[i].cP.tilt_scale = 10; //make sure it never gets out of control
                        }
                        */
                    } else{  optInfo->im[i].cP.tilt_scale = optInfo->im[k-2].cP.tilt_scale;}
                }
                // translate
                if( (k = optInfo->opt[i].transXopt) > 0 ){
                        if( k == 1 ){ optInfo->im[i].cP.trans_x  =      x[j++];
                        }else{  optInfo->im[i].cP.trans_x = optInfo->im[k-2].cP.trans_x;}
                }
                if( (k = optInfo->opt[i].transYopt) > 0 ){
                        if( k == 1 ){ optInfo->im[i].cP.trans_y  =      x[j++];
                        }else{  optInfo->im[i].cP.trans_y = optInfo->im[k-2].cP.trans_y;}
                }
                if( (k = optInfo->opt[i].transZopt) > 0 ){
                        if( k == 1 ){ optInfo->im[i].cP.trans_z  =      x[j++];
                        }else{  optInfo->im[i].cP.trans_z = optInfo->im[k-2].cP.trans_z;}
                }
                if( (k = optInfo->opt[i].transYawOpt) > 0 ){
                        if( k == 1 ) { optInfo->im[i].cP.trans_yaw  = x[j++]; //NORM_ANGLE(optInfo->im[i].cP.trans_yaw);
                           while( optInfo->im[i].cP.trans_yaw > optInfo->im[i].yaw + 80) optInfo->im[i].cP.trans_yaw -= 180.0; 
                           while( optInfo->im[i].cP.trans_yaw < optInfo->im[i].yaw - 80) optInfo->im[i].cP.trans_yaw += 180.0;
                        } else { optInfo->im[i].cP.trans_yaw = optInfo->im[k-2].cP.trans_yaw;}
                }
                if( (k = optInfo->opt[i].transPitchOpt) > 0 ){
                        if( k == 1 ){ optInfo->im[i].cP.trans_pitch = x[j++]; //NORM_ANGLE(optInfo->im[i].cP.trans_pitch);
                           while( optInfo->im[i].cP.trans_pitch > optInfo->im[i].pitch + 80) optInfo->im[i].cP.trans_pitch -= 180.0; 
                           while( optInfo->im[i].cP.trans_pitch < optInfo->im[i].pitch - 80) optInfo->im[i].cP.trans_pitch += 180.0;
                        }else{  optInfo->im[i].cP.trans_pitch = optInfo->im[k-2].cP.trans_pitch;}
                }
                // test
                if( (k = optInfo->opt[i].testP0opt) > 0 ){
                        if( k == 1 ){ optInfo->im[i].cP.test_p0  =      x[j++];
                        }else{  optInfo->im[i].cP.test_p0 = optInfo->im[k-2].cP.test_p0;}
                }
                if( (k = optInfo->opt[i].testP1opt) > 0 ){
                        if( k == 1 ){ optInfo->im[i].cP.test_p1  =      x[j++];
                        }else{  optInfo->im[i].cP.test_p1 = optInfo->im[k-2].cP.test_p1;}
                }
                if( (k = optInfo->opt[i].testP2opt) > 0 ){
                        if( k == 1 ){ optInfo->im[i].cP.test_p2  =      x[j++];
                        }else{  optInfo->im[i].cP.test_p2 = optInfo->im[k-2].cP.test_p2;}
                }
                if( (k = optInfo->opt[i].testP3opt) > 0 ){
                        if( k == 1 ){ optInfo->im[i].cP.test_p3  =      x[j++];
                        }else{  optInfo->im[i].cP.test_p3 = optInfo->im[k-2].cP.test_p3;}
                }

                //shear
                if( (k = optInfo->opt[i].shear_x) > 0 ){
                        if( k == 1 ){ optInfo->im[i].cP.shear_x  =      x[j++];
                        }else{  optInfo->im[i].cP.shear_x = optInfo->im[k-2].cP.shear_x;}
                }

                if( (k = optInfo->opt[i].shear_y) > 0 ){
                        if( k == 1 ){ optInfo->im[i].cP.shear_y  =      x[j++];
                        }else{  optInfo->im[i].cP.shear_y = optInfo->im[k-2].cP.shear_y;}
                }

                
                optInfo->im[i].cP.radial_params[0][0] = 1.0 - ( optInfo->im[i].cP.radial_params[0][3]
                                                                                                                + optInfo->im[i].cP.radial_params[0][2]
                                                                                                                + optInfo->im[i].cP.radial_params[0][1] ) ;

        }
        avgfovFromSAP = sumfov / optInfo->numIm;
        if( j != optInfo->numParam )
                return -1;
        else
                return 0;

}

// Set LM params using global preferences structure
// Change to cover range 0....1 (roughly)

int SetLMParams( double *x )
{
        int i,j;
                
        j=0; // Counter for optimization parameters


        for( i=0; i<optInfo->numIm; i++ ){
                if(optInfo->opt[i].yaw == 1)  //  optimize alpha? 0-no 1-yes
                        x[j++] = optInfo->im[i].yaw;

                if(optInfo->opt[i].pitch == 1)  //  optimize pitch? 0-no 1-yes
                        x[j++] = optInfo->im[i].pitch; 

                if(optInfo->opt[i].roll == 1)  //  optimize gamma? 0-no 1-yes
                        x[j++] = optInfo->im[i].roll ; 

                if(optInfo->opt[i].hfov == 1)  //  optimize hfov? 0-no 1-yes
                        x[j++] = optInfo->im[i].hfov ; 

                if(optInfo->opt[i].a == 1)  //  optimize a? 0-no 1-yes
                        x[j++] =  optInfo->im[i].cP.radial_params[0][3] * C_FACTOR; 

                if(optInfo->opt[i].b == 1)   //  optimize b? 0-no 1-yes
                        x[j++] = optInfo->im[i].cP.radial_params[0][2] * C_FACTOR; 

                if(optInfo->opt[i].c == 1)  //  optimize c? 0-no 1-yes
                        x[j++] = optInfo->im[i].cP.radial_params[0][1] * C_FACTOR; 

                if(optInfo->opt[i].d == 1)  //  optimize d? 0-no 1-yes
                        x[j++] = optInfo->im[i].cP.horizontal_params[0] ; 

                if(optInfo->opt[i].e == 1)  //  optimize e? 0-no 1-yes
                        x[j++] = optInfo->im[i].cP.vertical_params[0]  ; 

                // Tilt
                if(optInfo->opt[i].tiltXopt == 1) { //  optimize tilt_x? 0-no 1-yes
                        x[j++] = optInfo->im[i].cP.tilt_x  ;
                }
                if(optInfo->opt[i].tiltYopt == 1)  //  optimize tilt_y? 0-no 1-yes
                        x[j++] = optInfo->im[i].cP.tilt_y  ;

                if(optInfo->opt[i].tiltZopt == 1) { //  optimize tilt_Z? 0-no 1-yes
                        x[j++] = optInfo->im[i].cP.tilt_z  ;
                }
                if(optInfo->opt[i].tiltScaleOpt == 1) { //  optimize tilt_scale? 0-no 1-yes
                        x[j++] = optInfo->im[i].cP.tilt_scale  ;
                }
                // Trans
                if(optInfo->opt[i].transXopt == 1) { //  optimize trans_x? 0-no 1-yes
                        x[j++] = optInfo->im[i].cP.trans_x  ;
                }
                if(optInfo->opt[i].transYopt == 1) { //  optimize trans_y? 0-no 1-yes
                        x[j++] = optInfo->im[i].cP.trans_y  ;
                }
                if(optInfo->opt[i].transZopt == 1) { //  optimize trans_Z? 0-no 1-yes
                        x[j++] = optInfo->im[i].cP.trans_z  ;
                }
                if(optInfo->opt[i].transYawOpt == 1) { //  optimize trans_yaw? 0-no 1-yes
                        x[j++] = optInfo->im[i].cP.trans_yaw  ;
                }
                if(optInfo->opt[i].transPitchOpt == 1) { //  optimize trans_pitch? 0-no 1-yes
                        x[j++] = optInfo->im[i].cP.trans_pitch  ;
                }

                // Test
                if(optInfo->opt[i].testP0opt == 1) {
                    x[j++] = optInfo->im[i].cP.test_p0;  ;
                }
                if(optInfo->opt[i].testP1opt == 1) {
                    x[j++] = optInfo->im[i].cP.test_p1;  ;
                }
                if(optInfo->opt[i].testP2opt == 1) {
                    x[j++] = optInfo->im[i].cP.test_p2;  ;
                }
                if(optInfo->opt[i].testP3opt == 1) {
                    x[j++] = optInfo->im[i].cP.test_p3;  ;
                }

                if(optInfo->opt[i].shear_x == 1)  //  optimize shear_x? 0-no 1-yes
                        x[j++] = optInfo->im[i].cP.shear_x  ;

                if(optInfo->opt[i].shear_y == 1)  //  optimize shear_y? 0-no 1-yes
                        x[j++] = optInfo->im[i].cP.shear_y  ;
        }
        
        if( j != optInfo->numParam )
                return -1;
        else
                return 0;

}



                



#define DX 3
#define DY 14

// Read Control Point Position from flag pasted into image

void getControlPoints( Image *im, controlPoint *cp )
{
        int y, x, cy,cx, bpp, r,g,b,n, nim=0, k,i,np;
        register unsigned char *p,*ch;
        
        
        p = *(im->data);
        bpp = im->bitsPerPixel/8;
        if( bpp == 4 )
        {
                r = 1; g = 2; b = 3;
        }
        else if( bpp == 3 )
        {               
                r = 0; g = 1; b = 2;
        }
        else
        {
                PrintError("Can't read ControlPoints from images with %d Bytes per Pixel", bpp);
                return;
        }
        
        np = 0;
        for(y=0; y<im->height; y++)
        {
                cy = y * im->bytesPerLine;
                for(x=0; x<im->width; x++)
                {
                        cx = cy + bpp * x;
                        if( p[ cx                       + r ]   == 0    && p[ cx                        + g ]   == 255  && p[ cx                        + b ]   == 0   &&
                                p[ cx + bpp     + r ]   == 255  && p[ cx + bpp          + g ]   == 0    && p[ cx + bpp          + b ]   == 0   &&
                                p[ cx + 2*bpp   + r ]   == 0    && p[ cx + 2*bpp        + g ]   == 0    && p[ cx + 2*bpp        + b ]   == 255 &&
                                p[ cx - bpp     + r ]   == 0    && p[ cx - bpp          + g ]   == 0    && p[ cx - bpp          + b ]   == 0 )
                        {
                                if(p[cx + 3*bpp + r ]   == 0    && p[ cx + 3*bpp        + g ]   == 255  && p[ cx + 3*bpp        + b ]   == 255)
                                {       // Control Point
                                        ch = &(p[cx + 4*bpp + r ]);
                                        n = 0;
                                        while( ch[0] == 255 && ch[1] == 0 && ch[2] == 0 )
                                        {
                                                n++;
                                                ch += bpp;
                                        }
                                        if( n >= 0 )
                                        {
                                                k = 0;
                                                if( cp[n].num[0] != -1 )
                                                        k = 1;
                                                cp[n].x[k] = x + DX;
                                                cp[n].y[k] = y + DY;
                                                np++;
                                        }
                                }
                                else if(p[cx+3*bpp +r]  == 255  && p[ cx + 3*bpp        + g ]   == 255  && p[ cx + 3*bpp        + b ]   == 0)
                                {       // Image number
                                        ch = &(p[cx + 4*bpp + r ]);
                                        n = 0;
                                        while( ch[0] == 255 && ch[1] == 0 && ch[2] == 0 )
                                        {
                                                n++;
                                                ch += bpp;
                                        }
                                        if( n >= 0 )
                                        {
                                                nim = n;
                                        }
                                }
                        }
                }
        }
        k = 0;
        if( cp[0].num[0] != -1 )
                k = 1;
        for(i=0; i<np; i++)
                cp[i].num[k] = nim;
        

}
                        
                        
// Write Control Point coordinates into script 

void writeControlPoints( controlPoint *cp,char* cdesc )
{
        int i;
        char line[80];
        
        *cdesc = 0;
        for(i=0; i<NUMPTS && cp[i].num[0] != -1; i++)
        {
                //sprintf( line, "c n%d N%d x%d y%d X%d Y%d\n", cp[i].num[0], cp[i].num[1], 
                sprintf( line, "c n%d N%d x%lf y%lf X%lf Y%lf\n", cp[i].num[0], cp[i].num[1], 
                                                                                                           cp[i].x[0], cp[i].y[0],
                                                                                                           cp[i].x[1], cp[i].y[1]);
                strcat( cdesc, line );
        }
}


void    SetStitchDefaults( struct stitchBuffer *sBuf)
{
        *sBuf->srcName          = 0;
        *sBuf->destName         = 0;
        sBuf->feather           = 10;
        sBuf->colcorrect        = 0;
        sBuf->seam              = _middle;
        sBuf->psdOpacity        = 255;
        sBuf->psdBlendingMode   = PSD_NORMAL;
}

void            SetOptDefaults( optVars *opt )
{
    opt->hfov = opt->yaw = opt->pitch = opt->roll = 0; 
    opt->a = opt->b = opt->c = opt->d = opt->e = 0; 
    opt->tiltXopt = opt->tiltYopt = opt->tiltZopt = opt->tiltScaleOpt = 0;
    opt->transXopt = opt->transYopt = opt->transZopt  = opt->transYawOpt = opt->transPitchOpt = 0;
    opt->testP0opt = opt->testP1opt = opt->testP2opt = opt->testP3opt = 0;
    opt->shear_x = opt->shear_y = 0;
}

void DoColorCorrection( Image *im1, Image *im2, int mode )
{
        double  ColCoeff [3][2];
        int     i;

        switch( mode )
        {
                case 0: 
                        break; // no correction
                case 1: // Correct im1
                        GetColCoeff( im1, im2, ColCoeff );
                        ColCorrect( im1, ColCoeff );
                        break; 
                case 2: // Correct im2
                        GetColCoeff( im1, im2, ColCoeff );
                        // Invert coefficients
                        for( i = 0;  i<3;  i++)
                        {
                                ColCoeff[i][1] = - ColCoeff[i][1] / ColCoeff[i][0];
                                ColCoeff[i][0] = 1.0/ColCoeff[i][0];
                        }
                        ColCorrect( im2, ColCoeff );
                        break; 
                case 3: // Correct both halfs                                                                   
                        GetColCoeff( im1, im2, ColCoeff );
                        for(i = 0; i<3; i++)
                        {
                                ColCoeff[i][1] =  ColCoeff[i][1] / 2.0 ;
                                ColCoeff[i][0] = (ColCoeff[i][0] + 1.0 ) / 2.0;
                        }
                        ColCorrect( im1, ColCoeff );
                        for(i = 0; i<3; i++)
                        {
                                ColCoeff[i][1] = - ColCoeff[i][1] / ColCoeff[i][0];
                                ColCoeff[i][0] = 1.0 / ColCoeff[i][0];
                        }
                        ColCorrect( im2, ColCoeff );
                        break;
                default: break;
        } // switch
}


// Do some checks on Optinfo structure and reject if obviously nonsense

int CheckParams( AlignInfo *g )
{
    int i;
    int         err = -1;
    char        *errmsg[] = {
        "No Parameters to optimize",
        "No input images",
        "No Feature Points",
        "Image width must be positive",
        "Image height must be positive",
        "Field of View must be positive",
        "Field of View must be smaller than 180 degrees in rectilinear Images",
        "Unsupported Image Format (must be 0,1,2,3,4,7,8,10,14 or 19)",
        "Panorama Width must be positive",
        "Panorama Height must be positive",
        "Field of View must be smaller than 180 degrees in rectilinear Panos",
        "Unsupported Panorama Format",
        "Control Point Coordinates must be positive",
        "Invalid Image Number in Control Point Descriptions",
    };
    
    if( g->numParam == 0 )                              err = 0;
    if( g->numIm        == 0 )                          err = 1;
    if( g->numPts       == 0 )                          err = 2;
    
    // Check images
    


    for( i=0; i<g->numIm; i++)
        {

            if (g->im[i].cP.tilt_scale == 0) {
                PrintError("Image [%d] has tilt_scale equal zero [%d]\n", i, g->im[i].cP.tilt_scale);
                return -1;
            }

            if( g->im[i].width  <= 0 )          err = 3;
            if( g->im[i].height <= 0 )          err = 4;
            if( g->im[i].hfov   <= 0.0 )        err = 5;
            if( g->im[i].format == _rectilinear && g->im[i].hfov >= 180.0 )     err = 6;
            if( g->im[i].format != _rectilinear     && g->im[i].format != _panorama &&
                g->im[i].format != _fisheye_circ    && g->im[i].format != _fisheye_ff && 
                g->im[i].format != _equirectangular && g->im[i].format != _orthographic &&
                g->im[i].format != _mirror          && g->im[i].format != _stereographic && 
                g->im[i].format != _equisolid       && g->im[i].format != _thoby)
                err = 7;
        }

    
    // Check Panorama specs
    
    if( g->pano.hfov <= 0.0 )   err = 5;
    if( g->pano.width <=0 )             err = 8;
    if( g->pano.height <=0 )            err = 9;
    if( g->pano.format == _rectilinear && g->pano.hfov >= 180.0 )       err = 10;
    
    
    if( g->pano.format != _rectilinear          && g->pano.format != _panorama           &&
        g->pano.format != _equirectangular      && g->pano.format != _fisheye_ff         &&
        g->pano.format != _stereographic        && g->pano.format != _mercator           &&
        g->pano.format != _trans_mercator       && g->pano.format != _sinusoidal         &&
        g->pano.format != _lambert              && g->pano.format != _lambertazimuthal   &&
        g->pano.format != _albersequalareaconic && g->pano.format != _millercylindrical  && 
        g->pano.format != _panini               && g->pano.format != _architectural      &&
        g->pano.format != _equisolid            && g->pano.format != _equipanini         &&
        g->pano.format != _biplane              && g->pano.format != _triplane &&
        g->pano.format != _panini_general       && g->pano.format != _thoby              &&
        g->pano.format != _orthographic         && g->pano.format != _hammer
        ) err=11;
    
    // Check Control Points
    for( i=0; i<g->numPts; i++)         {
        // Joost: cp coordinates can be possible, no problem!  
        //              if( g->cpt[i].x[0] < 0 || g->cpt[i].y[0] < 0 || g->cpt[i].x[1] < 0 || g->cpt[i].y[1] < 0 )
        //                      err = 12;
        if( g->cpt[i].num[0] < 0 || g->cpt[i].num[0] >= g->numIm ||
            g->cpt[i].num[1] < 0 || g->cpt[i].num[1] >= g->numIm )                      err = 13;
    }
    
    if( err != -1 ) {
        PrintError( errmsg[ err ] );
        return -1;
    }
    else
        return 0;
}
                        

static int              CheckMakeParams( aPrefs *aP)
{
        double im_vfov;
        im_vfov = aP->im.hfov / aP->im.width * aP->im.height;
        
        if( (aP->pano.format == _rectilinear) && (aP->pano.hfov >= 180.0) )
        {
                PrintError("Rectilinear Panorama can not have 180 or more degrees field of view.");
                return -1;
        }
        if( (aP->im.format == _rectilinear) && (aP->im.hfov >= 180.0) )
        {
                PrintError("Rectilinear Image can not have 180 or more degrees field of view.");
                return -1;
        }
        if( (aP->mode & 7) == _insert ){
                if( (aP->im.format == _fisheye_circ ||  aP->im.format == _fisheye_ff) &&
                    (aP->im.hfov > MAX_FISHEYE_FOV && im_vfov > MAX_FISHEYE_FOV) ){
                                PrintError("Fisheye lens processing limited to fov <= %lg", MAX_FISHEYE_FOV);
                                return -1;
                }
        }

        return 0;
        
}


                        

// return 0, if overlap exists, else -1
/*
static int GetOverlapRect( PTRect *OvRect, PTRect *r1, PTRect *r2 )
{
        OvRect->left    = max( r1->left, r2->left );
        OvRect->right   = min( r1->right, r2->right );
        OvRect->top             = max( r1->top, r2->top );
        OvRect->bottom  = min( r1->bottom, r2->bottom );
        
        if( OvRect->right > OvRect->left && OvRect->bottom > OvRect->top )
                return 0;
        else
                return -1;
}
*/


void SetGlobalPtr( AlignInfo *p )
{
	optInfo = p;
}


AlignInfo* GetGlobalPtr(void)
{
        return optInfo;
}

void GetControlPointCoordinates(int i, double *x, double *y, AlignInfo *gl )
{
        double          w2, h2;
        int j, n[2];

        struct  MakeParams      mp;
        struct  fDesc           stack[15];

        

        n[0] = gl->cpt[i].num[0];
        n[1] = gl->cpt[i].num[1];
        
        // Calculate coordinates x/y in panorama

        for(j=0; j<2; j++)
        {
                SetInvMakeParams( stack, &mp, &gl->im[ n[j] ], &gl->pano, 0 );
                
                h2      = (double)gl->im[ n[j] ].height / 2.0 - 0.5;
                w2      = (double)gl->im[ n[j] ].width  / 2.0 - 0.5;
                

                execute_stack_new(      (double)gl->cpt[i].x[j] - w2,           // cartesian x-coordinate src
                                                (double)gl->cpt[i].y[j] - h2,           // cartesian y-coordinate src
                                                &x[j], &y[j], stack);

                h2      = (double)gl->pano.height / 2.0 - 0.5;
                w2      = (double)gl->pano.width  / 2.0 - 0.5;
                x[j] += w2;
                y[j] += h2;
        }
}


int AddEdgePoints( AlignInfo *gl )
{
        void *tmp;

        tmp =  realloc( gl->cpt, (gl->numPts+4) * sizeof( controlPoint ) );
        if( tmp == NULL )       return -1;
        gl->numPts+=4; gl->cpt = (controlPoint*)tmp; 

        gl->cpt[gl->numPts-4].num[0] = 0;
        gl->cpt[gl->numPts-4].num[1] = 1;
        gl->cpt[gl->numPts-4].x[0] = gl->cpt[gl->numPts-4].x[1] = -9.0 * (double)gl->pano.width;
        gl->cpt[gl->numPts-4].y[0] = gl->cpt[gl->numPts-4].y[1] = -9.0 * (double)gl->pano.height;

        gl->cpt[gl->numPts-3].num[0] = 0;
        gl->cpt[gl->numPts-3].num[1] = 1;
        gl->cpt[gl->numPts-3].x[0] = gl->cpt[gl->numPts-3].x[1] = 10.0 * (double)gl->pano.width;
        gl->cpt[gl->numPts-3].y[0] = gl->cpt[gl->numPts-3].y[1] = -9.0 * (double)gl->pano.height;

        gl->cpt[gl->numPts-2].num[0] = 0;
        gl->cpt[gl->numPts-2].num[1] = 1;
        gl->cpt[gl->numPts-2].x[0] = gl->cpt[gl->numPts-2].x[1] = -9.0 * (double)gl->pano.width;
        gl->cpt[gl->numPts-2].y[0] = gl->cpt[gl->numPts-2].y[1] = 10.0 * (double)gl->pano.height;

        gl->cpt[gl->numPts-1].num[0] = 0;
        gl->cpt[gl->numPts-1].num[1] = 1;
        gl->cpt[gl->numPts-1].x[0] = gl->cpt[gl->numPts-1].x[1] = 10.0 * (double)gl->pano.width;
        gl->cpt[gl->numPts-1].y[0] = gl->cpt[gl->numPts-1].y[1] = 10.0 * (double)gl->pano.height;

        return 0;
}


