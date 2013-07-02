/*
 *  PTcommon.c
 *
 *  Many of the routines are based on the program PTStitcher by Helmut
 *  Dersch.
 * 
 *  Copyright Helmut Dersch and Daniel M. German
 *  
 *  Dec 2005
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *  Author: Daniel M German dmgerman at uvic doooot ca
 * 
 */

#include "filter.h"
#include "PTcommon.h"
#include "ColourBrightness.h"
#include "pttiff.h"
#include "file.h"
#include "PTcommon.h"
#include "ptstitch.h" 

#include <assert.h>
#include <float.h>
#include <math.h>

//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/types.h>
//#include <dirent.h>
//#include <unistd.h>
//#include <stdint.h>
//#include <math.h>

#ifndef _MSC_VER
#include <unistd.h>
#else
#define isnan _isnan
#include "tools/compat_win32/getopt.h"
#endif


// Uncomment following line to enable testing of inverses in getROI
//#define PANO_TEST_INVERSE


int panoFlattenTIFF(fullPath * fullPathImages, int counterImageFiles,
                    fullPath * outputFileName, int removeOriginals);


//declare functions
void getCropInformationFromTiff(TIFF * tif, CropInfo * c);
void getROI(TrformStr * TrPtr, aPrefs * aP, PTRect * ROIRect);

int ptQuietFlag = 0;

void InsertFileName(fullPath * fp, char *fname)
{
    char *c = strrchr((char *) (fp->name), PATH_SEP);
    if (c != NULL)
        c++;
    else
        c = fp->name;
    strcpy(c, fname);
}


int panoPSDCreate(fullPath * fullPathImages, int numberImages,
                  fullPath * outputFileName, pano_flattening_parms *flatteningParms)
{
    Image *ptrImage;
    int i;
    stBuf stitchInfo;
    fullPath tempFile;
    char tempString[128];
    Image image;
    Boolean bBig = FALSE;


    assert(numberImages > 0);
    assert(fullPathImages != NULL);
    assert(outputFileName != NULL);

    if (ptQuietFlag == 0) {
        Progress(_initProgress, "Converting TIFF to PSD");
        sprintf(tempString, "%d", 100 / numberImages);
        Progress(_setProgress, tempString);
    }

    // Process background of PSD
    SetImageDefaults(&image);

    if (panoTiffRead(&image, fullPathImages[0].name) == 0) {

      PrintError("Could not read TIFF image No 0 %s", fullPathImages[0].name);
        if (ptQuietFlag == 0)
            Progress(_disposeProgress, tempString);

        return -1;
    }

    // Check to see if we need to create PSB instead of PSD file
    if(image.height > 30000 || image.width > 30000 || flatteningParms->forceBig == 1)
      bBig = TRUE;

    if (!(image.bitsPerPixel == 64 || image.bitsPerPixel == 32)) {
        PrintError("Image type not supported (%d bits per pixel)\n",
                   image.bitsPerPixel);
        return -1;
    }

    // New versions of Photoshop can handle multilayer 16bit files
    // Add an option to down sample to 8bit only if user request
    if (numberImages > 1 && image.bitsPerPixel != 32) {
        if (image.bitsPerPixel == 64 && flatteningParms->force8bit == 1) {
            //PrintError
            //    ("Panotools is not able to save 16bit PSD images. Downsampling to 8 bit");
            TwoToOneByte(&image);       //we need to downsample to 8 bit if we are provided 16 bit images
        }
    }

    if (numberImages == 1) {
        if (writePS(&image, outputFileName, bBig) != 0) {
            PrintError("Could not write PSD-file");
            if (ptQuietFlag != 0)
            Progress(_disposeProgress, tempString);
            return -1;
        }
        return 0;
    }

    //////////////////////////////////////////////////////////////////////
    //current algorithm is O(n2) which means it will take _forever_ to  create 
    // a huge Photoshop files with many layers.

    //Write out the first image as the base layer in the PSD file

    if (writePSwithLayer(&image, outputFileName,bBig) != 0) {
        PrintError("Could not write PSD-file");
        if (ptQuietFlag != 0)
            Progress(_disposeProgress, tempString);
        return -1;
    }
    panoImageDispose(&image);

    ptrImage = &image;

    //Now iterate over all other images and add them as layers to the PSD file
    for (i = 1; i < numberImages; i++) {

        if (ptQuietFlag == 0) {
            sprintf(tempString, "%d", i * 100 / numberImages);
            if (Progress(_setProgress, tempString) == 0) {
                remove(outputFileName->name);
                return -1;
            }
        }

        if (panoTiffRead(ptrImage, fullPathImages[i].name) == 0) {

            PrintError("Could not read TIFF image No &d", i);
            if (ptQuietFlag == 0)
                Progress(_disposeProgress, tempString);
            return -1;
        }

        // We can't process 16 bit TIFFs. We have to downsample to 8 bit if necessary
        if (image.bitsPerPixel == 64 && flatteningParms->force8bit == 1)
            TwoToOneByte(ptrImage);

        // Create a new file with the result PSD, then delete the current one

        strcpy(tempFile.name, outputFileName->name);

        if (panoFileMakeTemp(&tempFile) == 0) {
            PrintError("Could not make Tempfile");
            return -1;

        }

        stitchInfo.seam = 1;
        stitchInfo.feather = 0;
        if (flatteningParms->stacked) 
          stitchInfo.psdOpacity = (unsigned char) (255.0/ (i + 1));
        else
          stitchInfo.psdOpacity = 255;

        stitchInfo.psdBlendingMode = flatteningParms->psdBlendingMode;

        if (addLayerToFile(ptrImage, outputFileName, &tempFile, &stitchInfo) != 0) {
            PrintError("Could not write Panorama File");
            return -1;
        }

        remove(outputFileName->name);
        rename(tempFile.name, outputFileName->name);

        panoImageDispose(ptrImage);
    }

    if (!ptQuietFlag) {
        Progress(_setProgress, "100");
        Progress(_disposeProgress, tempString);
    }

    return 0;
}



#ifdef __Win__
//void InsertFileName( fullPath *fp, char *fname ){
// char *c = strrchr((char*)(fp->name), PATH_SEP);
// if(c != NULL) c++;
// else c = fp->name;
// strcpy( c, fname );
//}   
#endif


void ARGtoRGBAImage(Image * im)
{
    int right;
    int left;
    int bottom;
    int top;
    int width;
    int i;


    if (im->selection.bottom == 0 && im->selection.right == 0) {

        top = 0;
        left = 0;
        bottom = im->height;
        right = im->width;


    }
    else {

        top = im->selection.top;
        bottom = im->selection.bottom;
        left = im->selection.left;
        right = im->selection.right;
    }

    width = right - left;

    //fprintf(stderr, "\nWidth %10d Top: %10d bottom %10d Right %10d Left %10d-------", width, top, bottom, right, left);

    assert(width >= 0);
    assert(bottom >= top);

    for (i = 0; i < bottom - top; i++) {

        ARGBtoRGBA(*(im->data) + i * im->bytesPerLine, width,
                   im->bitsPerPixel);

    }                           // for 

}



void Clear_Area_Outside_Selected_Region(Image * image)
{
    // This function clears (i.e. sets to zero) the area outside the 
    // selection region 

    int right;
    int left;
    int bottom;
    int top;
    //  int width;
    //  int var24;
    int bytesPerPixel;          // 32
    unsigned char *dataPtr;
    unsigned char *pixelPtr;

    int currentRow;
    int currentColumn;

    // Only works for 8/16 bit per channel (32/64 bits per pixel) images
    assert(image->bitsPerPixel == 32 || image->bitsPerPixel == 64);

    top = image->selection.top;
    bottom = image->selection.bottom;
    left = image->selection.left;
    right = image->selection.right;

    if (bottom == 0)
        bottom = image->height;

    if (right == 0)
        right = image->width;

    if (image->bitsPerPixel == 32) {
        bytesPerPixel = 4;
    }
    else if (image->bitsPerPixel == 64) {
        bytesPerPixel = 8;
    }
    else {
        PrintError("Invalid bits per pixel in image %d", image->bitsPerPixel);
        exit(1);
    }


    if (image->format == _fisheye_circ || image->format == _thoby) {

        // TODO
        // This routine works only in fisheyes in portrait mode
        // it probably fails in landscape mode
    
        int horCenter, verCenter;
        int horRadious;
        int horRadious2;

        horCenter = (left + right) / 2;
        verCenter = (top + bottom) / 2;

        // Compute the horizontal width divided by 2, 
        // let us call it horizontal radios

        horRadious = (right - left) / 2;
        // Square it, so we don't have to compute this 
        // every time
        horRadious2 = horRadious * horRadious;

        dataPtr = *(image->data);

        // Scan the image from top to bottom
        for (currentRow = 0; currentRow < image->height; currentRow++) {
            int verDistance;
            int verDistance2;

            // The algorith it simple. Find the distance of each from from the 
            // center of the image. If the point is farther than horRadious
            // then set mask to zero

            currentColumn = 0;
            pixelPtr = dataPtr;

            // Compute the square of the vertical distance to this row from center
            verDistance = (currentRow - verCenter);
            verDistance2 = verDistance * verDistance;

            for (currentColumn = 0; currentColumn < image->width; currentColumn ++) {
                int horDistance;
                int horDistance2;

                // Compute square of distance of this point to center 
                // the old Pythagoras way
                // distance^2 = horDistance^2  + verDistance^2

                horDistance = (currentColumn - horCenter);
                horDistance2 = horDistance * horDistance;

                if (horDistance2 + verDistance2 > horRadious2) {

                    // Point falls outside the circle defined its horizontal maximum distance

                    // Set mask to zero
                    if (bytesPerPixel == 4)
                        *pixelPtr = 0;     
                    else if (bytesPerPixel == 8) {
                        *pixelPtr = 0;     
                        *(pixelPtr+1) = 0;     
                    }

                }
                pixelPtr +=  bytesPerPixel;

            } // for column
            dataPtr += image->bytesPerLine;
        }  // for row
        return;
    }



    // Clear the area at above the image
    dataPtr = *(image->data);

    for (currentRow = 0; currentRow < top; currentRow++) {
        pixelPtr = dataPtr;

        for (currentColumn = 0; currentColumn < image->width; currentColumn++) {
            assert(sizeof(int) == bytesPerPixel);
            memset(pixelPtr, 0, bytesPerPixel);
            pixelPtr += bytesPerPixel;
        }

        dataPtr += image->bytesPerLine;
    }

    // Clear area below the picture
    dataPtr = bottom * image->bytesPerLine + *(image->data);

    for (currentRow = bottom; currentRow < image->height; currentRow++) {
        pixelPtr = dataPtr;
        for (currentColumn = 0; currentColumn < image->width; currentColumn++) {
            memset(pixelPtr, 0, bytesPerPixel);
            pixelPtr += bytesPerPixel;
        }

        dataPtr += image->bytesPerLine;

    }                           //  for (    ;  %currentColumn < image->width ; currentColumn++,pixelPtr += bytesPerPixel) {


    /* Clear the area to the left of the picture */

    dataPtr = *(image->data);
    for (currentRow = 0; currentRow < image->height; currentRow++) {

        pixelPtr = dataPtr;
        for (currentColumn = 0; currentColumn < left; currentColumn++) {
            memset(pixelPtr, 0, bytesPerPixel);
            pixelPtr += bytesPerPixel;
        }

        dataPtr += image->bytesPerLine;
    }

    /* Clear the area to the right of the picture */

    dataPtr = *(image->data);

    for (currentRow = 0; currentRow < image->height; currentRow++) {

        pixelPtr = dataPtr + bytesPerPixel * right;

        for (currentColumn = right; currentColumn < image->width;
             currentColumn++) {

            memset(pixelPtr, 0, bytesPerPixel);

            pixelPtr += bytesPerPixel;

        }

        dataPtr += image->bytesPerLine;

    }

    return;

}


/**
 * This function computes the minimal rectangle needed to encompass
 * the region of the output image (TrPtr->dest) that will be populated with 
 * data from the input image (TrPtr->src) using the options specified
 * in aP.  The ROIRect is populated with the left/right/top/bottom values
 * that define this ROI within the output image
 */
void getROI(TrformStr * TrPtr, aPrefs * aP, PTRect * ROIRect)
{
    struct MakeParams mpinv;
    fDesc invstack[15], finvD;
    struct MakeParams mp;
    fDesc stack[15], fD;
    int color = 0;

    int x, y, x_jump;
    double x_d, y_d;            // Cartesian Coordinates of point in source (i.e. input) image
    double Dx, Dy;              // Coordinates of corresponding point in destination (i.e. output) image
    double Dx2, Dy2;            // Coordinates of corresponding point in destination (i.e. output) image

    double w2 = (double) TrPtr->dest->width / 2.0 - 0.5;        //half destination image width
    double h2 = (double) TrPtr->dest->height / 2.0 - 0.5;       //half destination image height
    double sw2 = (double) TrPtr->src->width / 2.0 - 0.5;        //half source image width
    double sh2 = (double) TrPtr->src->height / 2.0 - 0.5;       //half source image height

    //Set initial values for ROI to be adjusted during this function
    ROIRect->left = TrPtr->dest->width - 1;
    ROIRect->right = 0;
    ROIRect->top = TrPtr->dest->height - 1;
    ROIRect->bottom = 0;

    //The "forward" transform allows us to map pixel
    //coordinates in the output image to their location in the source image.
    // We use it to test the inverse functions of libpano
    printf("-------------------------------------------\n");
    SetMakeParams( stack, &mp, &(aP->im) , &(aP->pano), color );
    fD.func = execute_stack_new; 
    fD.param = stack;

    //The "inverse" transform allows us to map pixel coordinates in each source image
    //to their location in the output image.
    //    printf("INV-------------------------------------------\n");
    SetInvMakeParams(invstack, &mpinv, &(aP->im), &(aP->pano), color);

    //    printf("-------------------------------------------\n");
    finvD.func = execute_stack_new;
    finvD.param = invstack;

    //iterate over edges of input image and compute left/right/top/bottom-most coordinate
    //in output image
    //For equirectangular output projection covering 360/180, iterating over the 
    //edges of each input image isn't sufficient to determine ROI because an 
    //an interior point in an input image can be at the edge of ROI.  More research 
    //needed here, but for now include some representative interior points as well.
    for (y = 0; y <= TrPtr->src->height; y += 1) {
        
        x_jump = (y==0 || y==TrPtr->src->height || abs(y - TrPtr->src->height/2)<=5) ? 1 : TrPtr->src->width/2; 

                for (x = 0; x <= TrPtr->src->width; x += x_jump) {
                        //convert source coordinates to cartesian coordinates (i.e. origin at center of image)
                        x_d = (double) x - sw2 ;
                        y_d = (double) y - sh2 ;

                        //Map the source image cartesian coordinate to the destination image cartesian coordinate
                        finvD.func( x_d, y_d, &Dx, &Dy, finvD.param);
                        
#ifdef PANO_TEST_INVERSE
                        fD.func( Dx, Dy, &Dx2, &Dy2, fD.param);
                        {
                            int newX, newY;
                            if (!isnan(Dx2) && !isnan(Dy2)) {

                                newX = (int)(Dx2 + 0.5 + sw2);
                                newY = (int)(Dy2 + 0.5 + sh2);

                                #ifdef XXXXX
                                if (newX != x || newY != y) {
                                    printf("  X,Y: %7.1f,%7.1f (%5d,%5d) -> OUT: %9.1f, %9.1f inv -> %9.1f %9.1f (%5d, %5d) -- error %5d,%5d\n", 
                                           x_d, y_d, 
                                           x,y,
                                           Dx, Dy, 
                                           Dx2, Dy2, 
                                           newX, newY,
                                           newX - x,
                                           newY - y
                                           );
                                }
                                #endif
                                // If this assertion fails, there is an error. The question is, how big? See the values above.
                                // it is possible that the error is so small that it does not matter.
/*
                                assert(fabs(newX-x) <= 20.0);
                                assert(fabs(newY-y) <= 20.0);
*/
                            }

                        }
                          //printf("  IN1: %f,%f -> OUT: %f, %f inv -> %f %f    (%d, %d)\n", x_d, y_d, Dx, Dy, Dx2, Dy2, (int)(Dx +0.5), (int)(Dy+0.5));
                          //printf("  IN3: %d,%d -> OUT: %f, %f   (%d, %d)\n", x, y, Dx, Dy, (int)(Dx +0.5), (int)(Dy+0.5));
#endif
                        
                        //Convert destination cartesian coordinate back to destination "screen" coordinates (i.e. origin at top left of image)
                        Dx += w2;
                        Dy += h2;

                        //Expand ROI if necessary
                        //I've observed that in some cases, the mapping function returns
                        //a value of "-1.#IND00".  This is not a number, and probably indicates
                        //a divide by zero error somewhere in the mapping function.  This should
                        //be solved, but, for now, discard this value and keep going
                        if (!isnan(Dx)) {
                            if ((int32_t)Dx < ROIRect->left) ROIRect->left = (int32_t)(Dx + 0.5);
                            if ((int32_t)Dx > ROIRect->right) ROIRect->right = (int32_t)(Dx + 0.5);
                        }
                        if (!isnan(Dy)){                
                            if ((int32_t)Dy < ROIRect->top) ROIRect->top = (int32_t)(Dy + 0.5);
                            if ((int32_t)Dy > ROIRect->bottom) ROIRect->bottom = (int32_t)(Dy + 0.5);
                        }
                }
        }
        
    // printf("ROI1: %ld,%ld - %ld, %ld\n", ROIRect->left, ROIRect->top, ROIRect->right, ROIRect->bottom);

        // If the destination is wrapped and the ROI is at least 95% of the width
        // Then ensure it is of the full width so no missing data is removed from either side.
        if ( (TrPtr->mode & _wrapX) &&
             ( (ROIRect->right-ROIRect->left) > (TrPtr->dest->width * 0.95) ) )
        {
          ROIRect->left  = 0;
          ROIRect->right = TrPtr->dest->width-1;
        }

        // Test for a zenith shot
        if(ROIRect->top != 0)
        {
          Dx = 0;
          Dy = - h2;
          fD.func( Dx, Dy, &Dx2, &Dy2, fD.param);
          {
            if (!isnan(Dx2) && !isnan(Dy2)) {

              x_d = (int)(Dx2 + 0.5 + sw2);
              y_d = (int)(Dy2 + 0.5 + sh2);
              // if x_d, y_d is inside the image then this is a zenith shot and should extent to the top
              if (y_d >= 0 && y_d <= TrPtr->src->height &&
                  x_d >= 0 && x_d <= TrPtr->src->width )
              {
                ROIRect->top = 0;
              }
            }
          }
        }

        // Test for a nadir shot
        if(ROIRect->bottom != TrPtr->dest->height-1)
        {
          Dx = 0;
          Dy = h2;
          fD.func( Dx, Dy, &Dx2, &Dy2, fD.param);
          {
            if (!isnan(Dx2) && !isnan(Dy2)) {

              x_d = (int)(Dx2 + 0.5 + sw2);
              y_d = (int)(Dy2 + 0.5 + sh2);
              // if x_d, y_d is inside the image then this is a zenith shot and should extent to the top
              if (y_d >= 0 && y_d <= TrPtr->src->height &&
                  x_d >= 0 && x_d <= TrPtr->src->width )
              {
                ROIRect->bottom = TrPtr->dest->height-1;
              }
            }
          }
        }

        //Reduce ROI if it extends beyond boundaries of final panorama region
        if (ROIRect->left    < 0) ROIRect->left =0;
        if (ROIRect->top     < 0) ROIRect->top  =0;
        if (ROIRect->right   > (TrPtr->dest->width-1))  ROIRect->right    = TrPtr->dest->width-1;  
        if (ROIRect->bottom  > (TrPtr->dest->height-1)) ROIRect->bottom   = TrPtr->dest->height-1;
        
        //        printf("ROI2: %ld,%ld - %ld, %ld\n", ROIRect->left, ROIRect->top, ROIRect->right, ROIRect->bottom);
}


// NO LONGER NEEDED

/**
 * Populates the CropInfo struct with data about cropping of 
 * the TIFF file specified by filename
 */
void getCropInformation(char *filename, CropInfo * c)
{

    TIFF *tif = TIFFOpen(filename, "r");
    if (tif == NULL) {
        PrintError("getCropInformation: Could not open TIFF file");
    }
    else {
        getCropInformationFromTiff(tif, c);
        TIFFClose(tif);
    }

}


#if 0 

void setFullSizeImageParameters(pt_tiff_parms * imageParameters,
                                CropInfo * crop_info)
{
    // Update the imageParameters so that the dimensions reflect the
    // the size of the full-sized output image, (recorded in the crop_info struct)
    imageParameters->imageLength = crop_info->full_height;
    imageParameters->imageWidth = crop_info->full_width;
    imageParameters->bytesPerLine =
        imageParameters->imageWidth * (imageParameters->bitsPerPixel / 8);
}

#endif


int panoCreatePanorama(fullPath ptrImageFileNames[], int counterImageFiles,
                       fullPath * panoFileName, fullPath * scriptFileName)
{

    Image *currentImagePtr;
    aPrefs *prefs;
    int var01;
    int var00;
    int colourCorrection;
    int panoProjection = 0;

    int lines;
    fullPath *fullPathImages;
    int loopCounter;
    char var40[8];
    char *tempString;           // It looks like a char *temp;          
    char outputFileName[512];
#if 0
    VRPanoOptions defaultVRPanoOptions;
#endif
    char tmpStr[64];            // string
    fullPath currentFullPath;
    fullPath panoName;          // according to documention: QTVR, PNG, PICT, TIFF, etc plus options...*/
    fullPath tempScriptFile;
    char output_file_format[256];
    Image resultPanorama;       //Output Image
    Image image1;               //Input Image

    FILE *regFile;
    char *regScript;
    unsigned int regLen;
    unsigned int regWritten;

    int feather = 0;


    pano_Tiff *tiffFile;             //Output file...will be written during this function
    TrformStr transform;        //structure holds pointers to input and output images and misc other info

    int ebx;

    int croppedTIFFIntermediate = 0;
    int croppedWidth = 0, croppedHeight = 0;
    PTRect ROIRect;
    unsigned int outputScanlineNumber = 0;

    pano_ImageMetadata metadata;

    /* Variables */
    colourCorrection = 0;       // can have values of 1 2 or 3
    var00 = 0;
    var01 = 0;

    //Copy script line for line into a new temporary file
    memcpy(&tempScriptFile, scriptFileName, sizeof(fullPath));
    if (panoFileMakeTemp(&tempScriptFile) == 0) {
        PrintError("Unable to create temporary file");
        goto mainError;
    }

    panoTiffSetErrorHandler();

    if ((regFile = fopen(tempScriptFile.name, "w")) == NULL) {
        PrintError("Could not open temporary Scriptfile");
        goto mainError;
    }

    if ((regScript = LoadScript(scriptFileName)) == 0) {
        PrintError("Could not load ScriptFile");
        fclose(regFile);
        goto mainError;
    }

    regLen = strlen(regScript);

    // Write script to temp file
    regWritten = fwrite(regScript, 1, regLen, regFile);

    // Make sure script was written completely
    if (regWritten != strlen(regScript)) {
        PrintError("Could not write temporary script");
        fclose(regFile);
        goto mainError;
    }
    
    fclose(regFile);

    //Initialize members to zero
    SetImageDefaults(&image1);
    SetImageDefaults(&resultPanorama);

    //transform structure holds input and output images, and some miscellaneous other information
    memset(&transform, 0, sizeof(TrformStr));
    transform.src = &image1;    // Input image
    transform.dest = &resultPanorama;   // Output image
    transform.mode = _honor_valid;      // How to run transformation
    transform.success = 1;      // 1 success 0 failure

    //Allocate space to hold fully qualified names of input images
    if ((fullPathImages = malloc(counterImageFiles * 512)) == NULL) {
        PrintError("Not enough memory");
        goto mainError;
    }

    // This is the main processing loop...it iterates over each input image
    // and maps the pixels in these input images into the output image(s)
    for (loopCounter = 0; loopCounter < counterImageFiles; loopCounter++) {

        // TODO 

        // the original PTstitcher logic  is strange

        // It processes a lot of data more than once. This part should really be done once for all images


        currentImagePtr = &image1;

        // Read the next adjust line (contains yaw, pitch, roll and other information)
        // for one input image from the script file
        if ((prefs = readAdjustLine(&tempScriptFile)) == 0) {
            PrintError("Could not read Scriptfile");
            goto mainError;
        }

        // printf("*********cut frame: should be zero for S and no crop %d\n",  prefs->im.cP.cutFrame);

        //New for PTMender...PTMender uses "cropped" TIFFs as its intermediate file 
        //format for all processing.  In contrast, PTStitcher used full-size TIFF
        //images for all intermediate processing.  PTMender can still write "uncropped" 
        //final as needed.
        //
        //To the end user, PTMender appears to behave similarly to PTStitcher for 
        //TIFF_m and TIFF_mask formats, outputting a one "full-size" TIFF for each 
        //layer.  However, the internal processing is done on cropped TIFFs which
        //speeds things up considerably.
        //
        //An important improvement over PTStitcher is that the user can also explicitly 
        //requests cropped output for multi layer TIFF output by inlcluding 
        //"r:CROP" as part of the "p" line (e.g. n"TIFF_m r:CROP")
        //
        //Using cropped TIFF as the intermediate format significantly speeds up 
        //processing, with larger panos showing more dramatic increases in speed.  
        //It should also mean that the creation of the "flattened" formats will 
        //be significantly more memory-friendly, as the masking steps and PSD 
        //assembly steps won't need to load images the size of the output file 
        //into memory at once.  Unless the PTMender is fed extremely large input 
        //images, all memory constraints should now be a thing of the past (MRDL - May 2006).

        //croppedTIFFIntermediate determines if all intermediate processing is done
        //with cropped or full size TIFF.  There probably isn't much of a reason
        //to ever disable this feature, other than for testing/debugging purposes.



        colourCorrection = prefs->sBuf.colcorrect;
        // This is a strange value:
        // colourCorrection == (i & 3) + (i+1)*4;
        // where i is the number of the reference image
        
        assert(colourCorrection >= 0
               && colourCorrection < (counterImageFiles + 1) * 4);
        if (prefs->pano.cP.radial != 0) {
            
            var00 = prefs->pano.cP.radial_params[0][2]; // what is this for, I have NO idea.
            var00++;

        }                       // begins 804a00e


        if (prefs->pano.cP.horizontal != 0) {

          // Colour correction in color only, not brightness

            var01 = prefs->pano.cP.horizontal_params[0];        // 0x75c //[3] 3 colours x horizontal shift value
            // at this point var01 contains the index to the image to use as anchor fo

            var01++;

        }

        // Projection format for final panorama
        panoProjection = prefs->pano.format;

        // Copy output pano name to panoName
        memcpy(&panoName, &prefs->pano.name, sizeof(fullPath));
        //memcpy(&global5640, &prefs->sBuf, sizeof(stBuf));
        
        //panoName.name contains the n"XXX" value from the script "p" lines (e.g. n"TIFF_m" or n"QTVR w400 h300 c1")
        tempString = panoName.name;
        --tempString;           /* nextWord does ++ before testing anything, this guarantess proper execution */
        nextWord(output_file_format, &tempString);
        
        if (strcmp(output_file_format, "TIFF_m") == 0 ) {
            // CHeck if we are suppose to do cropped or uncropped
            croppedTIFFIntermediate = 1;
            if(strcmp(tempString, "") != 0) {
              nextWord(output_file_format, &tempString);
              if (strcmp(output_file_format, "") == 0 || 
                  strcmp(output_file_format, "r:CROP") == 0
                  ) {
                  // DO uncropped by default unless we are fisheye
                  if (prefs->im.format == _fisheye_circ) {
                      PrintError("Cropped output is unsupported for circular fisheye lenses. Ignored");
                  } 
              } else if (strcmp(output_file_format, "r:UNCROP") == 0) {
                  croppedTIFFIntermediate = 0;
              } else {
                  PrintError("Unsupported option in TIFF_m output (%s). Ignored", output_file_format);
              }
            }
            // DO uncropped by default unless we are fisheye
            if (prefs->im.format == _fisheye_circ) {
                croppedTIFFIntermediate = 0;
            } 
        } else {
            PrintError("No support for this ouput image format (%s). Output will be TIFF_m", output_file_format);
        }
        // enable this to avoid cropped tiffs. usually for testing
        //croppedTIFFIntermediate = 0;



        transform.interpolator  = prefs->interpolator;
        transform.gamma         = prefs->gamma;
        transform.fastStep      = prefs->fastStep;
        if( prefs->pano.hfov == 360.0 )
          transform.mode       |= _wrapX;
        
        if (ptQuietFlag == 0) {
            sprintf(tmpStr, "Converting Image %d / %d", (loopCounter + 1),
                    counterImageFiles);
            Progress(_initProgress, tmpStr);
        }

        //Read input image into transform.src
        if (panoImageRead(currentImagePtr, &ptrImageFileNames[loopCounter]) == 0) {
            PrintError("Could not read input image [%s]", ptrImageFileNames[loopCounter].name);
            goto mainError;
        }

        // printf("Ended reading INPUT image\n");

        //This "masks" the input image so that some pixels are excluded from 
        //transformation routine during pixel remapping/interpolation 

        if (prefs->im.cP.cutFrame != 0) {       // remove frame? 0 - no; 1 - yes
            // THIS CODE is executed in crop C type only, but not in S type
            //    printf("To crop image\n");
            if (CropImage(currentImagePtr, &(prefs->im.selection)) == 0) {
                prefs->im.selection.left = 0;
                prefs->im.selection.right = 0;
                prefs->im.selection.bottom = 0;
                prefs->im.selection.top = 0;
            }
        }
        //setup width/height of input image
        prefs->im.width = image1.width;
        prefs->im.height = image1.height;

        //Try to set reasonable values for output pano width and/or height if not 
        //specified as part of input (Do this only when processing first image in script)
        if (loopCounter == 0) {

            feather = prefs->sBuf.feather;
            if (prefs->pano.width == 0) {
                // if the pano did not set the width, then try to set it
                if (prefs->im.hfov != 0.0) {
                    prefs->pano.width = prefs->im.width * prefs->pano.hfov / prefs->im.hfov;
                    prefs->pano.width /= 10;    // Round to multiple of 10
                    prefs->pano.width *= 10;
                }
            }

            if (prefs->pano.height == 0)
                prefs->pano.height = prefs->pano.width / 2;
            
            resultPanorama.height = prefs->pano.height;
            resultPanorama.width = prefs->pano.width;
            
            if (resultPanorama.height == 0 || resultPanorama.width == 0) {
                PrintError("Please set Panorama width/height");
                goto mainError;
            }
        }                       //End attempt at setting reasonable values for pano width/height
        
        
        //printf("to set metadata\n");
        
        //////////////////////////////////////////////////////////////////////
        // Set metadata for output file

        panoMetadataCopy(&metadata, &image1.metadata);
        
        // The size of the image will change, so we have to update all the
        // fields accordingly.
        panoMetadataResetSize(&metadata,
                              resultPanorama.width,
                              resultPanorama.height);

        metadata.imageNumber = loopCounter;
        metadata.imageTotalNumber = counterImageFiles;
        metadata.imageDescription = strdup(regScript);


        // Set output width/height for output file 
        if (croppedTIFFIntermediate) {
            getROI(&transform, prefs, &ROIRect);
            //Dimensions determine size of TIFF file
            croppedWidth = (ROIRect.right - ROIRect.left) + 1;
            croppedHeight = (ROIRect.bottom - ROIRect.top) + 1;
            
            panoMetadataSetAsCropped(&metadata, 
                                     croppedWidth, croppedHeight,
                                     ROIRect.left, ROIRect.top);
        }
        
        panoMetadataSetCompression(&metadata, prefs->pano.name);

        //The resultPanorama.selection determines which region of the output image
        //is iterated over during the main pixel-remapping processing logic.  Much
        //of the image will be empty (black space) for any given input image.  However,
        //if cropped output is selected, then only the region of interest (ROI) into
        //which this input image will be mapped is processed...this significantly
        //speeds up processing
        if (croppedTIFFIntermediate) {
            resultPanorama.selection.left = ROIRect.left;
            resultPanorama.selection.right = ROIRect.right + 1; // the right edge is actually the pixel NOT in the pano
            resultPanorama.selection.top = ROIRect.top;
        }
        else {
            resultPanorama.selection.left = 0;
            resultPanorama.selection.right = resultPanorama.width;
            resultPanorama.selection.top = 0;
        }

        resultPanorama.bitsPerPixel = image1.bitsPerPixel;
        resultPanorama.bytesPerLine = metadata.bytesPerLine;

        panoMetadataCopy(&resultPanorama.metadata, &metadata);

        panoMetadataFree(&metadata);

        //////End of set metadata


        ///  CREATE OUTPUT FILE

        // Copy the current output file name to he fullPathImages[loopCounter]
        memcpy(&fullPathImages[loopCounter], &panoFileName, sizeof(fullPath));

        // Create temporary file where output data wil be written
        if (panoFileMakeTemp(&fullPathImages[loopCounter]) == 0) {
            PrintError("Could not make Tempfile");
            goto mainError;
        }

        // Populate currentFullPath.name with output file name
        GetFullPath(&fullPathImages[loopCounter], currentFullPath.name);

        // Open up output file for writing...data will be written in TIFF format

        if ((tiffFile = panoTiffCreate(currentFullPath.name, 
                       &resultPanorama.metadata)) == 0) {
            PrintError("Could not open %s for writing", currentFullPath.name);
            goto mainError;
        }

        if (ptQuietFlag == 0) {
            if (Progress(_setProgress, "5") == 0) {
                panoTiffClose(tiffFile);
                remove(fullPathImages[loopCounter].name);
                return (-1);
            }
        }

        //The output image is generated a few lines at a time to make efficient use
        //of limited memory...compute a reasonable number of lines to process (must
        //be at least 1, but no more than output height)
        lines = 500000 / resultPanorama.bytesPerLine;

        if (lines == 0)
            lines = 1;

        //Don't process more lines than are available
        if (lines >
            (croppedTIFFIntermediate ? croppedHeight : resultPanorama.height))
            lines =
                (croppedTIFFIntermediate ? croppedHeight : resultPanorama.
                 height);

        if ((resultPanorama.data =
             (unsigned char **) mymalloc(lines *
                                         resultPanorama.bytesPerLine)) ==
            NULL) {
            PrintError("Not enough memory for output panorama buffer");
            exit(1);
        }
        //NB resultPanorama.selection.bottom is actually one pixel beyond last row with data.
        resultPanorama.selection.bottom =
            resultPanorama.selection.top + lines;

        //    printf("bits per pixel %d\n", resultPanorama.bitsPerPixel);
        //    printf("cropped %d\n", croppedTIFFIntermediate);

        if (resultPanorama.bitsPerPixel != image1.bitsPerPixel) {
            PrintError
                ("All source images must have the same number of bits per pixel.");
            exit(1);
        }

        //Copy all position related data (yaw, pitch, roll, etc) for input image to currentImagePtr
        CopyPosition(currentImagePtr, &(prefs->im));

        //image1.selection determines how much of the input image to be 
        //included during main pixel remapping logic
        image1.selection.top = prefs->im.selection.top;
        image1.selection.bottom = prefs->im.selection.bottom;
        image1.selection.left = prefs->im.selection.left;
        image1.selection.right = prefs->im.selection.right;

    /*
        printf("****** Image selection hfov %f, %d %d %d %d \n", image1.hfov, image1.selection.top,
               image1.selection.bottom,
               image1.selection.left,
               image1.selection.right);
    */

        CopyPosition(&resultPanorama, &(prefs->pano));

        //Set image data outside selection region to zeros

        Clear_Area_Outside_Selected_Region(currentImagePtr);

        //pano.width and height must be equal to the full canvas size (not the 
        //size of the cropped output image...if selected) in order for the pixel 
        //remapping logic to work correctly.
        prefs->pano.width = resultPanorama.width;
        prefs->pano.height = resultPanorama.height;

        //Iterate over the output image multiple lines at a time, remapping pixels
        //from the input image into the output image, and writing data to an
        //output TIFF file.  Finish iterating when we reach the bottom of the 
        //output image (or, in the case of a cropped file, the bottom of the 
        //output ROI).
        outputScanlineNumber = 0;
        while (resultPanorama.selection.top <
               (croppedTIFFIntermediate ? ROIRect.bottom +
                1 : resultPanorama.height)) {

            // Call the main pixel remapping routine...all the interpolation happens here

        /*
            printf("Prefs: %f\n", prefs->pano.hfov);
            printf("Prefs im: hvof %f, yaw %f pitch %f, roll %f\n", prefs->im.hfov, prefs->im.yaw, prefs->im.pitch, prefs->im.roll);
            printf("Prefs pano: hvof %f, vfov %f pitch %f, roll %f\n", prefs->pano.hfov, prefs->pano.yaw, prefs->pano.pitch, prefs->pano.roll);
            printf("Prefs Interpolator %d:\n", prefs->interpolator);
            printf("Prefs Gamma %d:\n", prefs->gamma);
            printf("Prefs FastT %d:/n,  prefs->fastStep);
        */

            MyMakePano(&transform, prefs, loopCounter);

            if (transform.success == 0) {       // Error 
                PrintError("Error converting image");
                goto mainError;
            }

            //Reverse byte order before writing out to TIFF file
            ARGtoRGBAImage(&resultPanorama);

            //Write calculated data rows to TIFF file one row (aka "scanline") at a time
            for (ebx = 0;
                 ebx <
                 resultPanorama.selection.bottom -
                 resultPanorama.selection.top; ebx++) {
                if (TIFFWriteScanline(tiffFile->tiff, 
                      *resultPanorama.data + (resultPanorama.bytesPerLine * ebx),
                      outputScanlineNumber, 1) != 1) {
                    PrintError("Unable to write to TIFF file\n");
                    return -1;
                }

                outputScanlineNumber++;
            }

            if (ptQuietFlag == 0) {

                //Update progress bar
                if (croppedTIFFIntermediate)
                    sprintf(tmpStr, "%d",
                            (int) ((resultPanorama.selection.bottom -
                                    ROIRect.top) * 100 / croppedHeight));
                else
                    sprintf(tmpStr, "%d",
                            (int) (resultPanorama.selection.bottom * 100 /
                                   resultPanorama.height));

                if (Progress(_setProgress, tmpStr) == 0) {
                    // Cancelled by the user
                    panoTiffClose(tiffFile);
                    remove(tempScriptFile.name);
                    remove(fullPathImages[loopCounter].name);
                    return (-1);
                }
            }

            //specify the next batch of rows to be processed 
            resultPanorama.selection.top = resultPanorama.selection.bottom;
            resultPanorama.selection.bottom =
                resultPanorama.selection.top + lines;

            //Be careful at boundary...end of image
            if (resultPanorama.selection.bottom >
                (croppedTIFFIntermediate ? ROIRect.bottom +
                 1 : resultPanorama.height))
                resultPanorama.selection.bottom =
                    (croppedTIFFIntermediate ? ROIRect.bottom +
                     1 : resultPanorama.height);
        }

        panoTiffClose(tiffFile);

#ifdef UNCROP_FISHEYES
        if (croppedTIFFIntermediate == 0) {
            // We can't process (yet) all files in cropped mode
            // To quite the roar from the masses let them think we
            // do. I wonder how long it will take for them to notice. Placebo effect?
            pano_cropping_parms croppingParms;
            bzero(&croppingParms, sizeof(croppingParms));

            if (panoTiffCrop(currentFullPath.name, currentFullPath.name, &croppingParms) == 0) {
                PrintError("Unable to write output file %s", currentFullPath.name);
                remove(tempScriptFile.name);
                return (-1);
            }
        }
#endif

        //////////////////////////////////////////////////////////////////////
        panoImageDispose(&image1);

        // The memory for td and ts was allocated in morpher.c with malloc 
        // (not myMalloc), so we need to use free (not myFree)
        if (prefs->td != NULL) {
            free((void **) prefs->td);
        }

        if (prefs->ts != NULL) {
            free((void **) prefs->ts);
        }
        free(prefs);

        panoImageDispose(&resultPanorama);
        
    }                           //End of main image processing loop
    
    if (!ptQuietFlag)
        Progress(_disposeProgress, "");

    // This is the end of the pixel remapping for all input images.
    // At this point we should have a collection of TIFF files containing
    // the warped input images.  For TIFF_m format this is all we need.  For
    // other formats, we may need to do extra work (feathering, flattening, etc.)

    //----------------------------------------------------------------------

    remove(tempScriptFile.name);

    panoImageDispose(&resultPanorama);
    panoImageDispose(&image1);

#if 0 
    // NO LONGER SUPPORTED IN THIS FUNCTION. IT SHOULD BE REMOVED IN THE FUTURE

    // These functions are to correct and/or brightness.  They are not required for 
    // panoramas that do not need any brightness adjustments.  Moreover, Dersch
    // was not fully satisfied with the quality of results obtained from
    // using these functions, and knew that they could be significantly
    // improved.  In general, I think it best to avoid using these features, 
    // and doing any color/brightness adjustments manually either before
    // or after stitching.  While these functions work OK for some images, some 
    // of the time, they can produce some obviously wrong results in some
    // circumstances...perhaps an area for future improvement, but probably not 
    // as important a feature (now that we have multi-resolution splining 
    // software like Enblend) as when Desrch first added these (MRDL).

    if (var00 != 0) {
        ColourBrightness(fullPathImages, fullPathImages, counterImageFiles,
                         var00 - 1, 1, 0);
    }

    if (var01 != 0) {           //
      //      fprintf(stderr, "This type of correction... 1\n");
        ColourBrightness(fullPathImages, fullPathImages, counterImageFiles,
                         var01 - 1, 2, 0);
    }                           // 

    if (colourCorrection != 0) {
      //      fprintf(stderr, "This type of correction... 2\n");
        ColourBrightness(fullPathImages, fullPathImages, counterImageFiles,
                         (colourCorrection / 4) - 1, 0, 0);
    }

    SetVRPanoOptionsDefaults(&defaultVRPanoOptions);

    /* Soo, at this point we have skipped the first word of the panorama:
       # n"QTVR w400 h300 c1"           additional viewer options in a quoted string together with format
       #              the following options are recognized:
       #                  w(width) and h(height) of viewer window (only QTVR on Macs)
       #                  c(codec: 0-JPEG, 1-Cinepak, 2-Sorenson) (only QTVR on Macs)
       #                  q(codec quality):
       #                     0-high,1-normal,2-low    QTVR on Macs
       #                     0-100(highest)           on other jpeg-formats (PAN, IVR, IVR_java, VRML)
       #                  g  progressive jpeg (0-no, 1-yes) (PAN, IVR, IVR_java, VRML)
       #                     Optimized JPEG (0-on(default), 2-disabled), (3-progressive with optimized disabled)
       #                  p  initial pan angle ( QTVR on Macs, VRML, IVR)
       #                  v  field of view (QTVR, VRML, IVR)
       #                  Many more options can be set by editing the viewer scripts
     */
    //int getVRPanoOptions( VRPanoOptions *v, char *line )

    getVRPanoOptions(&defaultVRPanoOptions, tempString);
#endif

    // We have to add "masks" to the images before finishing...

    if (ptQuietFlag == 0)
        Progress(_initProgress, "Writing Output Images");

    for (loopCounter = 0; loopCounter < counterImageFiles; loopCounter++) {
        
        if (ptQuietFlag == 0) {
            sprintf(tmpStr, "%d",
                    (100 * loopCounter) / counterImageFiles);
            if (Progress(_setProgress, tmpStr) == 0) {
                return (1);
            }
        }

        strcpy(outputFileName, panoFileName->name);
        sprintf(var40, "%04d", loopCounter);
        strcat(outputFileName, var40);
        panoReplaceExt(outputFileName, ".tif");
        // remove output file, if it exists. 
        if (panoSingleFileExists(outputFileName)) {
            remove(outputFileName);
        }
        rename(fullPathImages[loopCounter].name, outputFileName);

    }
    free(fullPathImages);


    if (ptQuietFlag == 0) {
        Progress(_setProgress, "100%");
        Progress(_disposeProgress, "");
    }

    free(regScript);
    return (0);

    // FUNCTION ENDS HERE

  mainError:
    free(regScript);

    return (-1);
}


#if 0

    // THIS_CODE_IS_NO_LONGER_SUPPORTED. IT SHOULD BE REMOVED IN THE FUTURE

    if (counterImageFiles > 1) {

      // There is no point in adding stitching masks for just one image 
      //printf("Creating seams******************\n");

        if (panoStitchReplaceMasks(fullPathImages, fullPathImages, counterImageFiles,
                                   feather) != 0) {
            PrintError("Could not create stitching masks");
            goto mainError;
        }
    }



    /************ OUTPUT FORMATS: Multiple TIFF ***************/
    // TIFF_m and TIFF_mask...just rename the intermediate files 
    // that we've already computed with numbers (e.g. img0000.tif, img0001.tif, etc.) 
    // and we are finished processing.
    if (strcmp(output_file_format, "TIFF_m") == 0
        || strcmp(output_file_format, "TIFF_mask") == 0) {


            if ((croppedTIFFIntermediate != 0 && croppedTIFFOutput != 0) ||
                (croppedTIFFIntermediate == 0 && croppedTIFFOutput == 0)) {
                // if intermediate and output formats are the same, then just rename and quit
                rename(fullPathImages[loopCounter].name, outputFileName);
            }
            else if (croppedTIFFIntermediate != 0 && croppedTIFFOutput == 0) {
                // if cropped intermediate, but we want uncropped output, then uncrop
                if (!panoUnCropTiff
                    (fullPathImages[loopCounter].name, outputFileName)) {
                    return (1);
                }
                remove(fullPathImages[loopCounter].name);
            }
            else {
                // only other option is to use uncropped files as intermediate, and want 
                // cropped as output.  This is (a) a waste of time and (b) not supported.
                // Show error, but be nice and rename existing images anyway
                PrintError
                    ("Cropped output files cannot be created from uncropped intermediate files\n\nWriting uncropped output: %s",
                     outputFileName);
                rename(fullPathImages[loopCounter].name, outputFileName);
            }


        }                       // end of for loop
        free(fullPathImages);

        if (ptQuietFlag == 0) {
            Progress(_setProgress, "100%");
            Progress(_disposeProgress, "");
        }
        return (0);
    }

    //printf("To start creating the output files\n");
    
  /************ OUTPUT FORMATS: Layered PSD ***************/
    // Layered PSD is less simple...we need to assemble the existing
    // intermediate files into a layered photoshop document
    if (strcmp(output_file_format, "PSD_nomask") == 0
        || strcmp(output_file_format, "PSD_mask") == 0
        ) {
        panoReplaceExt(panoFileName->name, ".psd");
        
        if (panoCreatePSD(fullPathImages, counterImageFiles, panoFileName, 0) != 0) {
            PrintError("Error creating PSD file");
            return (-1);
        }

        for (loopCounter = 0; loopCounter < counterImageFiles; loopCounter++) {
            remove(fullPathImages[loopCounter].name);
        }

        free(fullPathImages);
        return (0);
    }


  /************ OUTPUT FORMATS: Flattened files ***************/
    // All other formats require us to "flatten" the intermediate layers into
    // one final document...general approach is to flatten to a single TIFF file, 
    // and then convert this to the desired output file format (e.g. JPEG, PNG, etc.)
    if (counterImageFiles > 1) 
    {

    if (!panoFlattenTIFF
        (fullPathImages, counterImageFiles, &fullPathImages[0], TRUE)) 
        {
        PrintError("Error while flattening TIFF-image");
        goto mainError;
    }

    }
    panoReplaceExt(panoFileName->name, ".tif");
    rename(fullPathImages[0].name, panoFileName->name);

    free(fullPathImages);

    //Desired output format is TIFF...no further conversion needed
    if (strcmp(output_file_format, "TIFF") == 0
        || strcmp(output_file_format, "TIF") == 0)
        return (0);


    //Read back in again so we can convert to final desired format
    if (panoImageRead(&resultPanorama, panoFileName) == 0) {
        PrintError("Could not read result image %s", panoFileName->name);
        goto mainError;
    }

    remove(panoFileName->name);

    if (strcmp(output_file_format, "QTVR") == 0)
        return Create_QTVR(&resultPanorama, panoFileName);

    if (strcmp(output_file_format, "IVR_java") == 0) {
        if (panoProjection == 1)
            return Unknown03(&resultPanorama, panoFileName);
        else
            return Unknown02(&resultPanorama, panoFileName);
    }

    if (strcmp(output_file_format, "VRML") == 0)
        return Unknown05(&resultPanorama, panoFileName);

    if (strncmp(output_file_format, "IVR", 3) == 0) {   // compare first 3 characters of it // end at 804ae10
        if (panoProjection == 1)
            return Unknown01(&resultPanorama, panoFileName);
        else
            return Create_LP_ivr(&resultPanorama, panoFileName);
    }

    if (strcmp(output_file_format, "PAN") == 0) {       // 
        return Unknown04(&resultPanorama, panoFileName);
    }                           // 804ae10

    if (strcmp(output_file_format, "JPEG") == 0
        || strcmp(output_file_format, "JPG") == 0) {
        if (!ptQuietFlag) {
            char temp[100];
            
            sprintf(temp, "Creating JPEG (quality %d jpegProgressive %d)\n",
                    defaultVRPanoOptions.cquality,
                    defaultVRPanoOptions.progressive);
                    
            Progress(_initProgress, temp);
        }
        panoReplaceExt(panoFileName->name, ".jpg");
        return writeJPEG(&resultPanorama, panoFileName,
                         defaultVRPanoOptions.cquality,
                         defaultVRPanoOptions.progressive);
    }


    if (strcmp(output_file_format, "PSD") == 0) {
        panoReplaceExt(panoFileName->name, ".psd");
        return (writePSD(&resultPanorama, panoFileName));

    }

    if (strcmp(output_file_format, "PNG") == 0) {
        panoReplaceExt(panoFileName->name, ".PNG");
        return (writePNG(&resultPanorama, panoFileName));
    }

    PrintError("Panorama output format not supported: %s",
               output_file_format);

#endif



/*
 * Because this function can be called with a directory name with a period
 * inside it (e.g. "c:\dir\another.dir\filewithoutextension") then we need to 
 * make sure that the . happens after the last \ otherwise we'd truncate 
 * the directory name rather than replacing the extension
 */
void panoReplaceExt(char *filename, char *extension)
{
    char *dot_pos = strrchr(filename, '.');
    char *path_sep_win = strrchr(filename, '\\');
    char *path_sep_unix = strrchr(filename, '/');
    char *path_sep = (path_sep_unix == NULL ? path_sep_win : path_sep_unix );

    if (dot_pos != NULL && (path_sep == NULL || dot_pos>path_sep)) {
        strcpy(dot_pos, extension);
    }
    else {
        strcat(filename, extension);
    }
    return;
}





int panoFlattenTIFF(fullPath * fullPathImages, int counterImageFiles,
                    fullPath * outputFileName, int removeOriginals)
{

    pano_Tiff **tiffFileHandles;
    pano_Tiff *outputFile;

    unsigned char **imageDataBuffers;
    unsigned char *resultBuffer;


    fullPath tmpFullPath;
    char tmpFilename[512];


    pano_CropInfo *cropInfo;
    unsigned int linesPerPass;
    pano_ImageMetadata *outputMetadata;

    unsigned int i;
    unsigned int offsetBeforeThisPass = 0;
    int linesLeft = 0;
    unsigned int linesToRead;
    int rowInPass;
    int inputImageRowIndex;
    int outputImageRowIndex;
    unsigned char *pixelPtr;

    //Open up all intermediate TIFF files at once

    assert(fullPathImages != NULL);
    assert(counterImageFiles > 1);
    assert(outputFileName != NULL);

    tiffFileHandles = calloc(counterImageFiles, sizeof(pano_Tiff));

    if (tiffFileHandles == NULL) {
        PrintError("Not enough memory");
        return 0;
    }

    for (i = 0; (int) i < counterImageFiles; i++) {

        if (GetFullPath(&fullPathImages[i], tmpFilename) != 0) {
            PrintError("Could not get filename");
            return 0;
        }

        if ((tiffFileHandles[i] = panoTiffOpen(tmpFilename)) == NULL) {
            PrintError("Could not open TIFF-Layer %d", i);
            return 0;
        }

    }

//////////////////////////////////////////////////////////////////////


    //modify "tmpFullPath" to contain the name of a new, empty temp file
    if (panoFileMakeTemp(&tmpFullPath) == 0) {
        PrintError("Could not make Tempfile");
        return 0;
    }

    //copy the name of this new tmpFullPath into a string (tmpFilename)
    if (GetFullPath(&tmpFullPath, tmpFilename) != 0) {
        PrintError("Could not get filename");
        return 0;
    }

    // Because the 0th intermediate TIFF file might be a "cropped" file, we 
    // need to update the imageParameters so that the dimensions reflect the
    // the size of the full-sized output image, rather than one of the 
    // (potentially) cropped intermediate files

    if ((outputFile =
         panoTiffCreateUnCropped(tmpFilename,
                                 &tiffFileHandles[0]->metadata)) == 0) {
        PrintError("Could not create TIFF file");
        return 0;
    }

    // Calculate number of lines to read at a time so that we are reading 
    // approximately 500 KB at a time from each input file.  This could be 
    // memory intensive if we have an awful lot of images and not much memory, 
    // but probably not a big problem for 99.9% of cases on 99.9% of machines.
    linesPerPass = 500000 / outputFile->metadata.bytesPerLine;

    if (linesPerPass == 0)
        linesPerPass = 1;

    outputMetadata = &outputFile->metadata;

    // We dont need to read more lines that the size of the file
    if (outputMetadata->imageHeight < linesPerPass) {
        linesPerPass = outputMetadata->imageHeight;
        if (linesPerPass == 0) {
            PrintError
                ("Invalid image length in TIFF file. It might be corrupted");
            return -1;
        }
    }

    // Create as many image data buffers as we have input files.  Note that the 
    // input buffers are as wide as the final output image, which may be more
    // than we technically need if the input images are cropped...it makes the 
    // code simpler, however.
    imageDataBuffers = calloc(counterImageFiles, sizeof(unsigned char *));

    for (i = 0; (int) i < counterImageFiles; i++) {
        imageDataBuffers[i] =
            calloc(linesPerPass * outputMetadata->bytesPerLine, 1);
        if (imageDataBuffers[i] == NULL) {
            PrintError("Not enough memory to allocate input buffers");
            return -1;
        }
    }

    //we need one buffer to store output result
    resultBuffer = calloc(linesPerPass * outputMetadata->bytesPerLine, 1);

    if (resultBuffer == NULL) {
        PrintError("Not enough memory to allocate output buffer");
        return -1;
    }

    offsetBeforeThisPass = 0;

    if (ptQuietFlag == 0) {
        Progress(_initProgress, "Flattening Image");
    }

    //  printf("To do %d lines\n", outputMetadata->imageHeight);

    linesLeft = outputMetadata->imageHeight;

    // Main flattening loop...iterate over input files, read some data from each, 
    // combine into output buffer, write to file
    while (linesLeft > 0) {

        linesToRead = (linesLeft > (int)linesPerPass) ? linesPerPass : linesLeft;

        // iterate over each input file
        for (i = 0; (int) i < counterImageFiles; i++) {
            cropInfo = &(tiffFileHandles[i]->metadata.cropInfo);

            // Get a few lines of data from this input file one row at a time
            for (rowInPass = 0; rowInPass < (int) linesToRead; rowInPass++) {

                //figure out which row to read/write from input/output images
                outputImageRowIndex = offsetBeforeThisPass + rowInPass;
                inputImageRowIndex = outputImageRowIndex - cropInfo->yOffset;

                //point to first byte on this row of the input buffer
                pixelPtr =
                    imageDataBuffers[i] +
                    (outputMetadata->bytesPerLine * rowInPass);

                //clear out any old data, and fill with empty space (zeros)
                memset(pixelPtr, 0, outputMetadata->bytesPerLine);

                // Only try to read data if we are reading from a row that exists in the 
                // input image        
                if (inputImageRowIndex >= 0
                    && inputImageRowIndex < cropInfo->croppedHeight) {
                    if (TIFFReadScanline
                        (tiffFileHandles[i]->tiff,
                         pixelPtr +
                         (cropInfo->xOffset * outputMetadata->bytesPerPixel),
                         inputImageRowIndex, 0) != 1) {
                        PrintError("Error reading tiff file\n");
                        return 0;
                    }
                }
            }
        }

        //    printf("Passing offsetAfterThisPass [%d] of [%d] linesPerPass  %d \n",offsetAfterThisPass, outputMetadata->imageHeight, linesPerPass);

        if (ptQuietFlag == 0) {
            sprintf(tmpFilename, "%d",
                    (offsetBeforeThisPass +
                     linesToRead) * 100 / outputMetadata->imageHeight);
            if (Progress(_setProgress, tmpFilename) == 0)
                return 0;
        }

        // FlattenImageSection
        panoStitchBlendLayers(imageDataBuffers, counterImageFiles, resultBuffer,
                              linesToRead, outputMetadata->imageWidth,
                              outputMetadata->bitsPerPixel,
                              outputMetadata->bytesPerLine);

        for (i = 0; i < linesToRead; i++) {
            if (TIFFWriteScanline
                (outputFile->tiff,
                 resultBuffer + outputMetadata->bytesPerLine * i,
                 offsetBeforeThisPass + i, 0) != 1) {
                PrintError("Unable to write TIFF to file\n");
                return 0;
            }
        }

        offsetBeforeThisPass += linesToRead;
        linesLeft -= linesToRead;

    }

    if (!ptQuietFlag)
        Progress(_disposeProgress, "Done flattening.");

    //  printf("Lines read %d from %d\n", offsetBeforeThisPass,outputMetadata->imageHeight);

    for (i = 0; (int) i < counterImageFiles; i++) {
        free(imageDataBuffers[i]);
        panoTiffClose(tiffFileHandles[i]);
    }

    panoTiffClose(outputFile);

    if (removeOriginals) {
        for (i = 0; (int) i < counterImageFiles; i++) {
            remove(fullPathImages[i].name);
        }
    }

    rename(tmpFullPath.name, outputFileName->name);

    free(tiffFileHandles);

    free(imageDataBuffers);
    free(resultBuffer);

    return 1;

}




// the functionality of PTcrop and PTuncrop is essentially identical, except for
// the function that they call

int panoCroppingMain(int argc,char *argv[], int operation, char *version, char *usage, char *defaultPrefix)
{
    int opt;
    int ptForceProcessing = 0;
    int filesCount;
    int retVal;
    pano_cropping_parms croppingParms;
    char outputPrefix[MAX_PATH_LENGTH];
    int ptDeleteSources = 0;
    fullPath *ptrInputFiles = NULL;
    fullPath *ptrOutputFiles = NULL;
    int base;
    int i;

    // Set defaults
    strcpy(outputPrefix, defaultPrefix);
    bzero(&croppingParms, sizeof(croppingParms));
    
    printf("%s", version);

    //Need enough space for a message to be returned if something goes wrong

    while ((opt = getopt(argc, argv, "p:fqhx")) != -1) {

        // o overwrite
        // h       -> help
        // q       -> quiet?

        switch(opt) {  // fhoqs        f: 102 h:104  111 113 115  o:f:hsq
        case 'p':
            if (strlen(optarg) < MAX_PATH_LENGTH) {
                strcpy(outputPrefix, optarg);
            } else {
                PrintError("Illegal length for output prefix");
                return -1;
            }
            break;
        case 'f':
            ptForceProcessing = 1;
            break;
        case 'x':
            ptDeleteSources = 1;
            break;
        case 'q':
            ptQuietFlag = 1;
            break;
        case 'h':
            printf("%s",usage);
            exit(0);
        default:
            break;
        }
    }
    filesCount = argc - optind;

    if (filesCount < 1) {
        PrintError("No files specified in the command line");
        printf("%s",usage);
        exit(0);
    }
    // Allocate memory for filenames
    if ((ptrInputFiles = calloc(filesCount, sizeof(fullPath))) == NULL || 
        (ptrOutputFiles = calloc(filesCount, sizeof(fullPath))) == NULL)        {
        PrintError("Not enough memory");
        free(ptrInputFiles);
        free(ptrOutputFiles);
        return -1;
    }

    // GET input file names
    base = optind;
    for (; optind < argc; optind++) {
        char *currentParm;

        currentParm = argv[optind];

        if (StringtoFullPath(&ptrInputFiles[optind-base], currentParm) !=0) { // success
            PrintError("Syntax error: Not a valid pathname");
            return(-1);
        }
    }
        // Generate output file names
    if (panoFileOutputNamesCreate(ptrOutputFiles, filesCount, outputPrefix) == 0) {
        return -1;
    }

    if (!ptForceProcessing) {
        char *temp;
        if ((temp = panoFileExists(ptrOutputFiles, filesCount)) != NULL) {
            PrintError("Output filename exists %s. Use -f to overwrite", temp);
            return -1;
        }
    }
    if (! ptQuietFlag) 
        printf("Cropping %d files\n", filesCount);

    for (i=0; i< filesCount; i++) {

        if (!ptQuietFlag) {
            PrintError("Processing %d reading %s creating %s", i, ptrInputFiles[i].name, ptrOutputFiles[i].name);
        }
        croppingParms.forceProcessing = ptForceProcessing;
        switch (operation) {
        case PANO_CROPPING_CROP:
            retVal = panoTiffCrop(ptrInputFiles[i].name, ptrOutputFiles[i].name, &croppingParms);
            break;
        case PANO_CROPPING_UNCROP:
            retVal = panoTiffUnCrop(ptrInputFiles[i].name, ptrOutputFiles[i].name, &croppingParms);
            break;
        default:
            PrintError("Illegal operation in panoCroppingMain. Programming error");
            exit(0);
        }


        if (! retVal ) {
            PrintError("Error cropping file %s", ptrInputFiles[i].name);
            return -1;
        }
    }
    if (ptDeleteSources) {
        panoFileDeleteMultiple(ptrInputFiles, filesCount);
    }
    if (ptrInputFiles != NULL)
        free(ptrInputFiles);
    if (ptrOutputFiles != NULL)
        free(ptrOutputFiles);

    return 0;
}



void panoPrintImage(char *msg, Image *im)
{
    printf("-------------%s\n", msg);
    if (im != NULL) {
        printf(">>>Image format %d\n", (int)im->format);
        printf(">>>Roll %f\n", im->roll);
        printf(">>>Pitch %f\n", im->pitch);
        printf(">>>Yaw %f\n", im->yaw);

        printf(">>>im->cP.shear %d\n", im->cP.shear);
        printf(">>>im->cP.tilt %d\n", im->cP.tilt);
        printf(">>>im->cP.tilt_x %f\n", im->cP.tilt_x);
        printf(">>>im->cP.tilt_y %f\n", im->cP.tilt_y);
        printf(">>>im->cP.tilt_z %f\n", im->cP.tilt_z);
        printf(">>>im->cP.tilt_scale %f\n", im->cP.tilt_scale);

        printf(">>>im->cP.translation %d\n", im->cP.trans);
        printf(">>>im->cP.trans_x %f\n", im->cP.trans_x);
        printf(">>>im->cP.trans_y %f\n", im->cP.trans_y);
        printf(">>>im->cP.trans_z %f\n", im->cP.trans_z);
        printf(">>>im->cP.trans_yaw %n", im->cP.trans_yaw);
        printf(">>>im->cP.trans_pitch %f\n", im->cP.trans_pitch);

        printf(">>>im->cP.test %d\n", im->cP.test);
        printf(">>>im->cP.test parm1 %f\n", im->cP.test_p0);
        printf(">>>im->cP.test parm2 %f\n", im->cP.test_p1);
        printf(">>>im->cP.test parm3 %f\n", im->cP.test_p2);
        printf(">>>im->cP.test parm4 %f\n", im->cP.test_p3);
    }
    printf("\n\n");

}

