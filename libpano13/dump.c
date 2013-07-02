/*
 *  dump.c
 * 
 *  Routines to dump data structures for debugging purposes
 *
 *  Copyright  Daniel M. German
 *  
 *  June 2010
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

#include <stdio.h>
#include <assert.h>
#include "dump.h"

#define MAX_INDENT 20

// Macro to generalize printing the fields. Notice the heavey use of stringification
// var can be preceeded with a cast, such as (int)var and it applies to the field
// i.e. (int)var->field due to the higher precedence of the -> operator
#define PRINT_GENERIC(field, format, var) fprintf(stderr, "%s\t" #field " " format "\n", ind, var->field)

static void panoDumpSetIndent(char *ind, int size)
{
    assert(size < MAX_INDENT && size >= 0);
    memset(ind, '\t', size);
    ind[size] = 0;
}

void panoDumpPTRect(PTRect *rect, char *label, int indent)
{
    char ind[MAX_INDENT+1];
    panoDumpSetIndent(ind, indent);

    if (label != NULL) {
        fprintf(stderr, "%s%s\n", ind, label);
    }

    fprintf(stderr, "%sRectangle\n", ind);

#define PRINT_INT(a) PRINT_GENERIC(a, "%d", (int)rect)
    PRINT_INT(top);
    PRINT_INT(bottom);
    PRINT_INT(left);
    PRINT_INT(right);
#undef PRINT_INT
}

void panoDumpCropInfo(CropInfo *crop, char *label, int indent)
{
    char ind[MAX_INDENT+1];
    panoDumpSetIndent(ind, indent);

#define PRINT_INT(a) PRINT_GENERIC(a, "%d", (int)crop)

    fprintf(stderr, "%sCrop Info\n", ind);
    PRINT_INT(full_width);
    PRINT_INT(full_height);
    PRINT_INT(cropped_width);
    PRINT_INT(cropped_height);
    PRINT_INT(x_offset);
    PRINT_INT(y_offset);

#undef PRINT_INT
}


void panoDumpCorrectPrefs(cPrefs *cP, char *label, int indent)
{
    char ind[MAX_INDENT+1];
    int i,j;

    panoDumpSetIndent(ind, indent);



#define PRINT_INT(a) PRINT_GENERIC(a,"%d", (int)cP)
#define PRINT_F(a)   PRINT_GENERIC(a,"%f", cP)

    if (label != NULL) {
        fprintf(stderr, "%s%s\n", ind, label);
    }

    fprintf(stderr, "%sCorrect Preferences\n", ind);

    if (cP->radial) {
        for (i=0;i<3;i++)  {
            for (j=0;j<5;j++)  {
                fprintf(stderr, "%s\tradial_params[%d][%d]\t%f\n", ind, i, j, cP->radial_params[i][j]);
            }
        }
    }
    if (cP->vertical) {
        for (i=0;i<3;i++)  {
            fprintf(stderr, "%s\tvertical_params[%d]\t%f\n", ind, i, cP->vertical_params[i]);
        }
    }
    if (cP->horizontal) {
        for (i=0;i<3;i++)  {
            fprintf(stderr, "%s\thorizontal_params[%d]\t%f\n", ind, i, cP->horizontal_params[i]);
        }
    }

    if (cP->shear) {
        PRINT_F(shear_x);
        PRINT_F(shear_y);
    }
    
    if (cP->tilt) {
        PRINT_F(tilt_x);
        PRINT_F(tilt_y);
        PRINT_F(tilt_z);
        PRINT_F(tilt_scale);
    }

    if (cP->trans) {
        PRINT_F(trans_x);
        PRINT_F(trans_y);
        PRINT_F(trans_z);
        PRINT_F(trans_yaw);
        PRINT_F(trans_pitch);
    }

    if (cP->test) {
        PRINT_F(test_p0);
        PRINT_F(test_p1);
        PRINT_F(test_p2);
        PRINT_F(test_p3);
    }
    /* I don't think these files are read as parameters... we'll see
    int resize;                 
    int32_t width;             
    int32_t height;            
    int luminance;              
    double lum_params[3];       
    int correction_mode;        
    int cutFrame;               
    int fwidth;
    int fheight;
    int frame;
    int fourier;                
    int fourier_mode;           
    fullPath psf;               
    int fourier_nf;             
    fullPath nff;               
    double filterfactor;        
    double fourier_frame;       
    */
#undef PRINT_INT
#undef PRINT_F

}



void panoDumpImage(Image *im, char *label, int indent)
{
    char ind[MAX_INDENT+1];
    int i;

    if (im == NULL) return;
    // prepare indent

    panoDumpSetIndent(ind, indent);

    if (label != NULL) {
        fprintf(stderr, "%s%s\n", ind, label);
    }
    
#define PRINT_INT(a) PRINT_GENERIC(a,"%d", (int)im)
#define PRINT_F(a) PRINT_GENERIC(a,"%f", im)
#define PRINT_S(a)   PRINT_GENERIC(a,"\"%s\"", im)

    fprintf(stderr, "%sImage Data\n", ind);
    PRINT_INT(width);
    PRINT_INT(height);
    PRINT_INT(bytesPerLine);
    PRINT_INT(bitsPerPixel);
    PRINT_INT(dataSize);
    PRINT_INT(dataformat);
    PRINT_INT(format);
    PRINT_INT(formatParamCount);

    for (i=0;i< im->formatParamCount; i++) {
        fprintf(stderr, "%s\t\tformat Param[%d] %f\n", ind, i, im->formatParam[i]);
    }
    PRINT_F(hfov);
    PRINT_F(yaw);
    PRINT_F(roll);
    PRINT_F(pitch);

    PRINT_S(name);
    
    panoDumpCorrectPrefs(&im->cP, NULL, indent+1);
    panoDumpPTRect(&im->selection, NULL,indent+1);
    panoDumpCropInfo(&im->cropInformation, NULL, indent+1);

#undef PRINT_INT
#undef PRINT_F
#undef PRINT_S

}


void panoDumpAdjustData(aPrefs* aP, char *label, int indent)
{

    char ind[MAX_INDENT+1];

    assert (aP != NULL);

    panoDumpSetIndent(ind, indent);

    if (label != NULL) {
        fprintf(stderr, "%s%s\n", ind, label);
    }

    fprintf(stderr, "%s\tAdjust Data\n", ind);

#define PRINT_INT(a) PRINT_GENERIC(a,"%d", (int)aP)
#define PRINT_F(a) PRINT_GENERIC(a,"%f", aP)
#define PRINT_S(a)   PRINT_GENERIC(a,"\"%s\"", aP)


    PRINT_INT(mode);
    PRINT_S(scriptFile.name);
    PRINT_INT(nt);
    PRINT_INT(interpolator);
    PRINT_F(gamma);
    PRINT_INT(fastStep);

#undef PRINT_INT
#undef PRINT_F
#undef PRINT_S


    panoDumpImage(&aP->im, "Input Image", indent + 1);
    panoDumpImage(&aP->pano, "Panorama",  indent + 1);

}
