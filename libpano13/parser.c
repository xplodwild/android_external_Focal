/* Panoama_Tools   -   Generate, Edit and Convert Panoramic Images
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

/*
    Modifications by Max Lyons (maxlyons@erols.com):

    March 16, 2002.  Added LINE_LENGTH to increase the amount of memory
    allocated when reading optimizer scripts.  Needed to reduce constraint
    on number of parameters than can be read by optimizer


/ *------------------------------------------------------------*/

/*

    Modifications by Fulvio Senore, June.2004
    Added support for a new letter "f" to enable fast pisels transform in "ReadModeDescription()"
    Added code between

    // FS+

    and

    // FS-

/ *------------------------------------------------------------*/



#include <assert.h>
#include "filter.h"
#include <locale.h>

#include "ZComb.h"
#include "adjust.h"


/* defined in adjust.c */
int AddEdgePoints( AlignInfo *gl );
void setFcnPanoHuberSigma(double sigma);

static int      ReadControlPoint    ( controlPoint * cptr, char *line);
static int      ReadImageDescription( Image *imPtr, stBuf *sPtr, char *line );
static int      ReadPanoramaDescription( Image *imPtr, stBuf *sPtr, char *line );
static int      ReadModeDescription ( sPrefs *sP, char *line );
static int      ReadCoordinates(    CoordInfo   *cp, char *line );

static int panoExternalToInternalInputProjection(int32_t input);

#define MY_SSCANF( str, format, ptr )       if( sscanf( str, format, ptr ) != 1 )   \
                                            {                                       \
                                                PrintError(                         \
                                                "Syntax error in script: Line %d\nCould not assign variable [%s]",\
                                                lineNum, str);         \
                                                return -1;                          \
                                            }                                       \

#define READ_VAR(format, ptr )      nextWord( buf, &li );           \
                                    MY_SSCANF( buf, format, ptr );
                                    
                                
                                


#define READ_OPT_VAR(var)       nextWord( buf, &li );           \
                                MY_SSCANF( buf, "%d", &k);      \
                                if( k<0 || k>= numIm )          \
                                {                               \
                                    PrintError("Syntax error in script: Line %d\n\nIllegal image number: %ld", lineNum, k);\
                                    return -1;                  \
                                }                               \
                                if( gl->opt[k].var )            \
                                {                               \
                                    PrintError("Conflict in script: Line %d. Multiple Instances of Variable %s Image number: %d (%d)", lineNum, #var, k, gl->opt[k].var); \
                                    return -1;                  \
                                }                               \
                                gl->opt[k].var   = 1;           \
                                n++;                            \
                                            
//Increased so more params can be parsed/optimized (MRDL - March 2002)
#define LINE_LENGTH         65536

                                            
/* 
void panoLocaleSave(void)
{
    char *p;
    p=setlocale(LC_ALL, NULL);
    oldLocale=strdup(p);
    setlocale(LC_ALL, "C");
}
*/

#define panoLocaleSave    char *oldLocale;oldLocale=strdup(setlocale(LC_ALL, NULL));setlocale(LC_ALL, "C")

#define panoLocaleRestore (oldLocale != NULL? (setlocale(LC_ALL,oldLocale),free(oldLocale)):0)


// Optimizer Script parser; fill global info structure

char *panoParseVariable(char *buf, char *li, int lineNum, int *indirectVar, double *var)
{
    if (*(li+1) == '=') {
        li++; // point to next character
        nextWord( buf, &li );
        if( sscanf( buf, "%d", indirectVar ) != 1 ) {                                       
            PrintError("Syntax error in script: Line %d\nCould not assign variable %s", li-1,
                       lineNum);
            return NULL;
        }
        (*indirectVar)+=2; //its offset should be increased by 2... arghh
    } else {
        nextWord( buf, &li );

        if( sscanf( buf, " %lf", var ) != 1 ) {                                       
            PrintError("Syntax error in script: Line %d\nCould not assign variable %s", li-1,
                       lineNum);
            return NULL;
        }
    }
    return li;
}



int ParseScript( char* script, AlignInfo *gl )
{
    Image               *im;
    optVars             *opt;
    CoordInfo           *ci;
    
    // Variables used by parser
    
    char                *li, line[LINE_LENGTH], *ch ,*lineStart, buf[LINE_LENGTH];
    int                 lineNum = 0;
    int                 i,k;


    int                 n=0; // Number of parameters to optimize
    int                 numIm,numPts,nt;

    panoLocaleSave;

    gl->im  = NULL;
    gl->opt = NULL;
    gl->cpt = NULL;
    gl->t   = NULL;
    gl->cim = NULL;

    // Determine number of images and control points


    gl->numIm   = numLines( script, 'i' );
    gl->numPts  = numLines( script, 'c' );
    gl->nt      = numLines( script, 't' );



    // Allocate Space for Pointers to images, preferences and control points
    
    gl->im      = (Image*)      malloc( gl->numIm   * sizeof(Image) );
    gl->opt     = (optVars*)        malloc( gl->numIm   * sizeof(optVars) );
    gl->cpt     = (controlPoint*)   malloc( gl->numPts  * sizeof( controlPoint ));          
    gl->t       = (triangle*)       malloc( gl->nt      * sizeof( triangle ));  
    gl->cim     = (CoordInfo*)      malloc( gl->numIm   * sizeof(CoordInfo) );
    
    if( gl->im == NULL || gl->opt == NULL || gl->cpt == NULL || gl->t == NULL || gl->cim == NULL ) {
        PrintError("Not enough memory");
        goto fail;
    }
    
    // Rik's mask-from-focus hacking
    ZCombSetDisabled();
    // end mask-from-focus Rik's hacking
    SetImageDefaults(&(gl->pano));  
    SetStitchDefaults(&(gl->st)); strcpy( gl->st.srcName, "buf" ); // Default: Use buffer 'buf' for stitching
    // printf("Number of images %d\n",gl-> numIm);

    for(i=0; i<gl->numIm; i++) {
        SetImageDefaults( &(gl->im[i]) );
        SetOptDefaults  ( &(gl->opt[i]));
        SetCoordDefaults( &(gl->cim[i]), i);
    }
    
    
    numIm =0; numPts = 0; nt = 0; // reused as indices
    
    // Parse script 
    
    ch = script;

    
    while( *ch != 0 ) {
        lineNum++;
        
        while(*ch == '\n')
            ch++;
        lineStart = ch;
        
        nextLine( line, &ch );
        
        // parse line; use only if first character is i,p,v,c,m
        
        switch( line[0] ) {
        case 'i':       // Image description
            
            im  = &(gl->im[numIm]);     // This image is being set 
            opt = &(gl->opt[numIm]);


            ci  = &(gl->cim[numIm]);
            
            li = &(line[1]);

            while( *li != 0) {

                switch(*li)
                    {
                    case 'w':   READ_VAR( FMT_INT32, &(im->width) ); break;
                    case 'h':   READ_VAR( FMT_INT32, &(im->height)); break;
                    case 'v':   if( *(li+1) == '=' ){
                            li++;
                            READ_VAR( "%d", &(opt->hfov));
                            opt->hfov += 2;
                        }else{
                            READ_VAR(  "%lf", &(im->hfov));
                        }
                        break;
                    case 'a':   if( *(li+1) == '=' ){
                            li++;
                            READ_VAR( "%d", &(opt->a));
                            opt->a += 2;
                        }else{
                            READ_VAR( "%lf", &(im->cP.radial_params[0][3]));
                        }
                        im->cP.radial   = TRUE;
                        break;
                    case 'b':   if( *(li+1) == '=' ){
                            li++;
                            READ_VAR( "%d", &(opt->b));
                            opt->b += 2;
                        }else{
                            READ_VAR( "%lf", &(im->cP.radial_params[0][2]));
                        }
                        im->cP.radial   = TRUE;
                        break;
                    case 'c':   if( *(li+1) == '=' ){
                            li++;
                            READ_VAR(  "%d", &(opt->c));
                            opt->c += 2;
                        }else{
                            READ_VAR( "%lf", &(im->cP.radial_params[0][1]));
                        }
                        im->cP.radial   = TRUE;
                        break;
                    case 'f':
                        READ_VAR( "%d", &k );
                        switch (k)
                            {
                            case IMAGE_FORMAT_RECTILINEAR:                im->format = _rectilinear;    break;
                            case IMAGE_FORMAT_PANORAMA:                   im->format = _panorama;
                                im->cP.correction_mode |= correction_mode_vertical;
                                break;
                            case IMAGE_FORMAT_FISHEYE_EQUIDISTANCECIRC:   im->format = _fisheye_circ;   break;
                            case IMAGE_FORMAT_FISHEYE_EQUIDISTANCEFF:     im->format = _fisheye_ff;     break;
                            case IMAGE_FORMAT_EQUIRECTANGULAR:            im->format = _equirectangular;
                                im->cP.correction_mode |= correction_mode_vertical;
                                break;
                            case IMAGE_FORMAT_MIRROR:                     im->format = _mirror; break;
                            case IMAGE_FORMAT_FISHEYE_ORTHOGRAPHIC:       im->format = _orthographic; break;
                            case IMAGE_FORMAT_FISHEYE_STEREOGRAPHIC:      im->format = _stereographic; break;
                            case IMAGE_FORMAT_FISHEYE_EQUISOLID:          im->format = _equisolid; break;
                            case IMAGE_FORMAT_FISHEYE_THOBY:              im->format = _thoby; break;
                              break;
                            default:  PrintError("Syntax error in script.  Projection not known: Line %d", lineNum);
                                goto fail;
                                break;
                            }
                        break;
                    case 'o':   li++;
                        im->cP.correction_mode |=  correction_mode_morph;
                        break;
                    case 'y':   if( *(li+1) == '=' ){
                            li++;
                            READ_VAR( "%d", &(opt->yaw));
                            opt->yaw += 2;
                        }else{
                            READ_VAR( "%lf", &(im->yaw));
                        }
                        break;
                    case 'p':   if( *(li+1) == '=' ){
                            li++;
                            READ_VAR( "%d", &(opt->pitch));
                            opt->pitch += 2;
                        }else{
                            READ_VAR("%lf", &(im->pitch));
                        }
                        break;
                    case 'r':   if( *(li+1) == '=' ){
                            li++;
                            READ_VAR( "%d", &(opt->roll));
                            opt->roll += 2;
                        }else{
                            READ_VAR("%lf", &(im->roll));
                        }
                        break;
                    case 'd' :  if( *(li+1) == '=' ){
                            li++;
                            READ_VAR( "%d", &(opt->d));
                            opt->d += 2;
                        }else{
                            READ_VAR( "%lf", &(im->cP.horizontal_params[0]));
                        }
                        im->cP.horizontal= TRUE;
                        break;  
                    case 'e':   if( *(li+1) == '=' ){
                            li++;
                            READ_VAR( "%d", &(opt->e));
                            opt->e += 2;
                        }else{
                            READ_VAR("%lf", &(im->cP.vertical_params[0]));
                        }
                        im->cP.vertical = TRUE;
                        break;
                    case 'g':   if( *(li+1) == '=' ){
                            li++;
                            READ_VAR( "%d", &(opt->shear_x));
                            opt->shear_x += 2;
                        }else{
                            READ_VAR("%lf", &(im->cP.shear_x));
                        }
                        im->cP.shear    = TRUE;
                        break;
                    case 't':   if( *(li+1) == '=' ){
                            li++;
                            READ_VAR( "%d", &(opt->shear_y));
                            opt->shear_y += 2;
                        }else{
                            READ_VAR("%lf", &(im->cP.shear_y));
                        }
                        im->cP.shear    = TRUE;
                        break;
                    case 'T':
                        li++;
                        switch (*li) {
                        case 'i': // Tilt
                            li++;

                            switch (*li) {
                            case 'X':  
                                li = panoParseVariable(buf, li, lineNum, &(opt->tiltXopt), &(im->cP.tilt_x));

                                break;
                            case 'Y': 
                                li = panoParseVariable(buf, li, lineNum, &(opt->tiltYopt), &(im->cP.tilt_y));
                                break;
                            case 'Z': 
                                li = panoParseVariable(buf, li, lineNum, &(opt->tiltZopt), &(im->cP.tilt_z));
                                break;
                            case 'S': 
                                li = panoParseVariable(buf, li, lineNum, &(opt->tiltScaleOpt), &(im->cP.tilt_scale));
                                if (opt->tiltScaleOpt > 1) {
                                    // it is an actual value to optimize, not a reference... check it
                                    if (im->cP.tilt_scale == 0) {
                                        PrintError("TiS parameter can't be zero. Error in script: Line %d", lineNum);
                                        return -1;
                                    }
                                }
                                break;
                            default:
                                PrintError("Unkonwn parameter Ti%c in script: Line %d", *li, lineNum);
                                return -1;
                            }

                            if (li == NULL) return -1;
                            im->cP.tilt    = TRUE;

                            break;
                        case 'r': // Translation
                            li++;
                            switch (*li) {
                            case 'X':  
                                li = panoParseVariable(buf, li, lineNum, &(opt->transXopt), &(im->cP.trans_x));
                                break;
                            case 'Y': 
                                li = panoParseVariable(buf, li, lineNum, &(opt->transYopt), &(im->cP.trans_y));
                                break;
                            case 'Z': 
                                li = panoParseVariable(buf, li, lineNum, &(opt->transZopt), &(im->cP.trans_z));
                                break;
                            default:
                                PrintError("Unknown translation parameter Tr%c in script: Line %d", *li, lineNum);
                                return -1;
                            }
                            if (li == NULL) return -1;
                            // Make sure that we only apply trans when these parameters are not zero
                            // Otherwise images are not rendered beyond 180 degrees FOV
                            if (im->cP.trans_x != 0.0 || 
                                im->cP.trans_y != 0.0 || 
                                im->cP.trans_z != 0.0) {
                                im->cP.trans    = TRUE;
                            }
                            break;
                        case 'p': // Translation remap plane
                            li++;
                            switch (*li) {
                            case 'y':  
                                li = panoParseVariable(buf, li, lineNum, &(opt->transYawOpt), &(im->cP.trans_yaw));
                                break;
                            case 'p': 
                                li = panoParseVariable(buf, li, lineNum, &(opt->transPitchOpt), &(im->cP.trans_pitch));
                                break;
                            default:
                                PrintError("Unknown translation parameter Tp%c in script: Line %d", *li, lineNum);
                                return -1;
                            }
                            if (li == NULL) return -1;
                            break;
                        case 'e': // test parameters
                            li++;
                            switch (*li) {
                            case '0':  
                                li = panoParseVariable(buf, li, lineNum, &(opt->testP0opt), &(im->cP.test_p0));
                                break;
                            case '1': 
                                li = panoParseVariable(buf, li, lineNum, &(opt->testP1opt), &(im->cP.test_p1));
                                break;
                            case '2': 
                                li = panoParseVariable(buf, li, lineNum, &(opt->testP2opt), &(im->cP.test_p2));
                                break;
                            case '3': 
                                li = panoParseVariable(buf, li, lineNum, &(opt->testP3opt), &(im->cP.test_p3));
                                break;
                            default:
                                PrintError("Unknown Test parameter Te%c in script: Line %d", *li, lineNum);
                                return -1;
                            }
                            if (li == NULL) return -1;
                            im->cP.test    = TRUE;
                            break;
                        default:
                            PrintError("Unkonwn parameter T%c in script: Line %d", *li, lineNum);
                            return -1;
                        }
                        break;
                    case 'n':           // Set filename
                        nextWord( buf, &li );
                        sprintf( im->name, "%s", buf );
                        break;
                    case 'm':  // Frame
                        li++;
                        switch( *li )
                            {
                            case 'x':
                                READ_VAR("%d", &(im->cP.fwidth) );
                                im->cP.cutFrame = TRUE;
                                break;
                            case 'y':
                                READ_VAR("%d", &(im->cP.fheight) );
                                im->cP.cutFrame = TRUE;
                                break;
                            default: 
                                li--;
                                READ_VAR("%d", &(im->cP.frame) );
                                im->cP.cutFrame = TRUE;                                         
                                break;
                            }
                        break;  
                    case 'X':   READ_VAR( "%lf", &ci->x[0] );
                        break;
                    case 'Y':   READ_VAR( "%lf", &ci->x[1] );
                        break;
                    case 'Z':   READ_VAR( "%lf", &ci->x[2] );
                        break;
                    case 'S':   nextWord( buf, &li );       
                        sscanf( buf, FMT_INT32","FMT_INT32","FMT_INT32","FMT_INT32, &im->selection.left, &im->selection.right, &im->selection.top, &im->selection.bottom );
                        break;
                    case 'C':   nextWord( buf, &li );       
                        sscanf( buf, FMT_INT32","FMT_INT32","FMT_INT32","FMT_INT32, &im->selection.left, &im->selection.right, &im->selection.top, &im->selection.bottom );
                        im->cP.cutFrame = TRUE;
                        break;
                    case 'V':
                    case 'K':
                        // Ignore V variables in i
                        nextWord( buf, &li );       
                    break;
                    default: 
                        li++;
                        break;
                    }

            }

            numIm++;

            break;  
        case 't':       // Triangle
            li = &(line[1]);
            i = 0;
            while( *li != 0)
                {
                    switch(*li)
                        {
                        case ' ' : 
                        case '\t': li++; break;
                        case 'i' : READ_VAR( "%d", &(gl->t[nt].nIm));   break;
                        default  : if(i<3)
                            {
                                li--;
                                READ_VAR( "%d", &(gl->t[nt].vert[i]) ); 
                                i++;
                            }
                        else
                            li++;
                            break;
                        }
                }
            nt++;
            break;
        case 'c':       // Control Points
                    
            gl->cpt[numPts].type = 0;   // default : optimize r
            if(  ReadControlPoint( &(gl->cpt[numPts]), &(line[1]) ) != 0 )
                {
                    PrintError("Syntax error in script in control point 'c': Line %d", lineNum);
                    goto fail;
                }
            numPts++;
            break;

        case 'm':       // Mode description
            if( ReadModeDescription( &gl->sP, &(line[1]) ) != 0 )
                {
                    PrintError( "Syntax error in script in mode description 'm': line %d" , lineNum);
                    goto fail;
                }
            break;
                                

        case 'v':       // Variables to optimize
            li = &(line[1]);

            while( *li != 0)
                {
                    switch(*li)
                        {
                        case 'y':   READ_OPT_VAR(yaw);
                            break;
                        case 'p':   READ_OPT_VAR(pitch);
                            break;
                        case 'r':   READ_OPT_VAR(roll);
                            break;
                        case 'v':   READ_OPT_VAR(hfov);
                            break;
                        case 'a':   READ_OPT_VAR(a);
                            break;
                        case 'b':   READ_OPT_VAR(b);
                            break;
                        case 'c':   READ_OPT_VAR(c);
                            break;
                        case 'd':   READ_OPT_VAR(d);
                            break;
                        case 'e':   READ_OPT_VAR(e);
                            break;
                        case 'g':   READ_OPT_VAR(shear_x);
                            break;
                        case 't':   READ_OPT_VAR(shear_y);
                            break;
                        case 'T':   
                            li++;
                            switch (*li) {
                            case 'i':

                                li++;
                                switch (*li) {
                                case 'X':
                                    READ_OPT_VAR(tiltXopt);
                                    break;
                                case 'Y':
                                    READ_OPT_VAR(tiltYopt);
                                    break;
                                case 'Z':
                                    READ_OPT_VAR(tiltZopt);
                                    break;
                                case 'S':
                                    READ_OPT_VAR(tiltScaleOpt);
                                    break;
                                default:
                                    PrintError("Unknown variable name variable to optimize Ti%c in script: Line %d", *li, lineNum);
                                    return -1;
                                }
                                break;
                            case 'r':
                                li++;
                                switch (*li) {
                                case 'X':
                                    READ_OPT_VAR(transXopt);
                                    break;
                                case 'Y':
                                    READ_OPT_VAR(transYopt);
                                    break;
                                case 'Z':
                                    READ_OPT_VAR(transZopt);
                                    break;
                                default:
                                    PrintError("Unknown variable name to optimize Tr%c in script: Line %d", *li, lineNum);
                                    return -1;
                                }
                                break;
                            case 'p':
                                li++;
                                switch (*li) {
                                case 'y':
                                    READ_OPT_VAR(transYawOpt);
                                    break;
                                case 'p':
                                    READ_OPT_VAR(transPitchOpt);
                                    break;
                                default:
                                    PrintError("Unknown variable name to optimize Tp%c in script: Line %d", *li, lineNum);
                                    return -1;
                                }
                                break;
                            case 'e':
                                li++;
                                switch (*li) {
                                case '0':
                                    READ_OPT_VAR(testP0opt);
                                    break;
                                case '1':
                                    READ_OPT_VAR(testP1opt);
                                    break;
                                case '2':
                                    READ_OPT_VAR(testP2opt);
                                    break;
                                case '3':
                                    READ_OPT_VAR(testP3opt);
                                    break;
                                default:
                                    PrintError("Unknown variable name to optimize Te%c in script: Line %d", *li, lineNum);
                                    return -1;
                                }
                                break;
                            default:
                                PrintError("Unkonwn parameter T%c in script: Line %d", *li, lineNum);
                                return -1;
                            }                                

                            break;
                        case 'X':   READ_VAR( "%d", &k );
                            if( k>=0 && k<gl->numIm )
                                gl->cim[k].set[0] = FALSE;
                            break;
                        case 'Y':   READ_VAR( "%d", &k );
                            if( k>=0 && k<gl->numIm )
                                gl->cim[k].set[1] = FALSE;
                            break;
                        case 'Z':   READ_VAR( "%d", &k );
                            if( k>=0 && k<gl->numIm )
                                gl->cim[k].set[2] = FALSE;
                            break;
                        default:
                            li++;
                            break;
                        }
                        
                }
            break;
        case 'p':       // panorama 
            gl->pano.format     = 2; // _equirectangular by default
            gl->pano.hfov       = 360.0;
            if( ReadPanoramaDescription( &(gl->pano), &(gl->st), &(line[1]) ) != 0 )
                {
                    PrintError( "Syntax error in panorama description p line: %d (%s)" , lineNum,line);
                    goto fail;
                }
            switch (gl->pano.format) {
            case PANO_FORMAT_RECTILINEAR:
                gl->pano.format = _rectilinear;
                break;
            case PANO_FORMAT_PANORAMA:
                gl->pano.format = _panorama;
                break;
            case PANO_FORMAT_EQUIRECTANGULAR:
                gl->pano.format = _equirectangular;
                break;
            case PANO_FORMAT_FISHEYE_FF:
                gl->pano.format = _fisheye_ff;
                break;
            case PANO_FORMAT_STEREOGRAPHIC:
                gl->pano.format = _stereographic;
                break;
            case PANO_FORMAT_MERCATOR:
                gl->pano.format = _mercator;
                break;
            case PANO_FORMAT_TRANS_MERCATOR:
                gl->pano.format = _trans_mercator;
                break;
            case PANO_FORMAT_SINUSOIDAL:
                gl->pano.format = _sinusoidal;
                break;
            case PANO_FORMAT_LAMBERT_EQUAL_AREA_CONIC:
                gl->pano.format = _lambert;
                break;
            case PANO_FORMAT_LAMBERT_AZIMUTHAL:
                gl->pano.format = _lambertazimuthal;
                break;
            case PANO_FORMAT_HAMMER:
                gl->pano.format = _hammer;
                break;
            case PANO_FORMAT_ALBERS_EQUAL_AREA_CONIC:
                gl->pano.format = _albersequalareaconic;
                break;
            case PANO_FORMAT_MILLER_CYLINDRICAL:
                gl->pano.format = _millercylindrical;
                break;
            case PANO_FORMAT_PANINI:
                gl->pano.format = _panini;
                break;
            case PANO_FORMAT_EQUI_PANINI:
                gl->pano.format = _equipanini;
                break;
            case PANO_FORMAT_PANINI_GENERAL:
                gl->pano.format = _panini_general;
                break;
            case PANO_FORMAT_ARCHITECTURAL:
                gl->pano.format = _architectural;
                break;
            case PANO_FORMAT_ORTHOGRAPHIC:
                gl->pano.format = _orthographic;
                break;
            case PANO_FORMAT_THOBY:
                gl->pano.format = _thoby;
                break;
            case PANO_FORMAT_EQUISOLID:
                gl->pano.format = _equisolid;
                break;
            case PANO_FORMAT_BIPLANE:
                gl->pano.format = _biplane;
                break;
            case PANO_FORMAT_TRIPLANE:
                gl->pano.format = _triplane;
                break;
            default:
                PrintError( "Unknown panorama projection: %d", gl->pano.format );
                goto fail;
            }
            if( (gl->pano.format == _rectilinear || gl->pano.format == _trans_mercator) && gl->pano.hfov >= 180.0 )
                {
                    PrintError( "Destination image must have HFOV < 180" );
                    goto fail;
                }
            break;

            // Rik's mask-from-focus hacking
        case 'z':   
            // I was tempted to remove this code, but I am not sure how it will affect PToptimizer, until then.. it will remain
            //PrintError( "z option is no longer supported by PTmender and it is ignored. Use PTmasker instead\n" );
            //ZCombSetEnabled();
            li = &(line[1]);
            while( *li != 0)
                {
                    switch(*li)
                        {
                        case 'm':   {   int mtype;
                                READ_VAR( "%d", &mtype);
                                ZCombSetMaskType(mtype);
                        }
                            break;
                        case 'f':   {   int fwHalfwidth;
                                READ_VAR( "%d", &fwHalfwidth);
                                ZCombSetFocusWindowHalfwidth(fwHalfwidth);
                        }
                            break;
                        case 's':   {   int swHalfwidth;
                                READ_VAR( "%d", &swHalfwidth);
                                ZCombSetSmoothingWindowHalfwidth(swHalfwidth);
                        }
                            break;
                        default:
                            li++;
                            break;
                        }
                }
            break;
            // end Rik's mask-from-focus hacking


        case '*':   // End of script-data
            *lineStart = 0; *ch = 0;
            break;
        default: break;
        }
    }

    // Set up Panorama description
    
    if( gl->pano.width == 0 && gl->im[0].hfov != 0.0)  // Set default for panorama width based on first image
        {
            gl->pano.width = ( gl->pano.hfov / gl->im[0].hfov ) * gl->im[0].width;
            gl->pano.width /= 10; gl->pano.width *= 10; // Round to multiple of 10
        }

    if( gl->pano.height == 0 )
        gl->pano.height = gl->pano.width/2;
        

    //  Set up global information structure
    gl->numParam    = n;
    gl->data        = NULL;

    // Set initial values for linked variables
    for(i=0; i<gl->numIm; i++){
        k = gl->opt[i].yaw - 2;
        if( k >= 0 ) gl->im[i].yaw = gl->im[ k ].yaw;

        k = gl->opt[i].pitch - 2;
        if( k >= 0 ) gl->im[i].pitch = gl->im[ k ].pitch;

        k = gl->opt[i].roll - 2;
        if( k >= 0 ) gl->im[i].roll = gl->im[ k ].roll;

        k = gl->opt[i].hfov - 2;
        if( k >= 0 ) gl->im[i].hfov = gl->im[ k ].hfov;

        k = gl->opt[i].a - 2;
        if( k >= 0 ) gl->im[i].cP.radial_params[0][3] = gl->im[ k ].cP.radial_params[0][3];

        k = gl->opt[i].b - 2;
        if( k >= 0 ) gl->im[i].cP.radial_params[0][2] = gl->im[ k ].cP.radial_params[0][2];

        k = gl->opt[i].c - 2;
        if( k >= 0 ) gl->im[i].cP.radial_params[0][1] = gl->im[ k ].cP.radial_params[0][1];

        k = gl->opt[i].d - 2;
        if( k >= 0 ) gl->im[i].cP.horizontal_params[0] = gl->im[ k ].cP.horizontal_params[0];

        k = gl->opt[i].e - 2;
        if( k >= 0 ) gl->im[i].cP.vertical_params[0] = gl->im[ k ].cP.vertical_params[0];

        k = gl->opt[i].shear_x - 2;
        if( k >= 0 ) gl->im[i].cP.shear_x = gl->im[ k ].cP.shear_x;

        k = gl->opt[i].shear_y - 2;
        if( k >= 0 ) gl->im[i].cP.shear_y = gl->im[ k ].cP.shear_y;

        // tilt variables----------------------------------------------------------------
        k = gl->opt[i].tiltXopt - 2;
        if( k >= 0 ) gl->im[i].cP.tilt_x = gl->im[ k ].cP.tilt_x;

        k = gl->opt[i].tiltYopt - 2;
        if( k >= 0 ) gl->im[i].cP.tilt_y = gl->im[ k ].cP.tilt_y;

        k = gl->opt[i].tiltZopt - 2;
        if( k >= 0 ) gl->im[i].cP.tilt_z = gl->im[ k ].cP.tilt_z;

        k = gl->opt[i].tiltScaleOpt - 2;
        if( k >= 0 ) gl->im[i].cP.tilt_scale = gl->im[ k ].cP.tilt_scale;

        // translation variables----------------------------------------------------------------
        k = gl->opt[i].transXopt - 2;
        if( k >= 0 ) gl->im[i].cP.trans_x = gl->im[ k ].cP.trans_x;

        k = gl->opt[i].transYopt - 2;
        if( k >= 0 ) gl->im[i].cP.trans_y = gl->im[ k ].cP.trans_y;

        k = gl->opt[i].transZopt - 2;
        if( k >= 0 ) gl->im[i].cP.trans_z = gl->im[ k ].cP.trans_z;

        k = gl->opt[i].transYawOpt - 2;
        if( k >= 0 ) gl->im[i].cP.trans_yaw = gl->im[ k ].cP.trans_yaw;

        k = gl->opt[i].transPitchOpt - 2;
        if( k >= 0 ) gl->im[i].cP.trans_pitch = gl->im[ k ].cP.trans_pitch;

        // test variables ----------------------------------------------------------------------

        k = gl->opt[i].testP0opt - 2;
        if( k >= 0 ) gl->im[i].cP.test_p0 = gl->im[ k ].cP.test_p0;
        k = gl->opt[i].testP1opt - 2;
        if( k >= 0 ) gl->im[i].cP.test_p1 = gl->im[ k ].cP.test_p1;
        k = gl->opt[i].testP2opt - 2;
        if( k >= 0 ) gl->im[i].cP.test_p2 = gl->im[ k ].cP.test_p2;
        k = gl->opt[i].testP3opt - 2;
        if( k >= 0 ) gl->im[i].cP.test_p3 = gl->im[ k ].cP.test_p3;


        //----------------------------------------------------------------------

        gl->im[i].cP.radial_params[0][0] = 1.0 - ( gl->im[i].cP.radial_params[0][3]
                                                   + gl->im[i].cP.radial_params[0][2]
                                                   + gl->im[i].cP.radial_params[0][1] ) ;
                                                        
        SetEquColor( &(gl->im[i].cP) );
        
    }


    panoLocaleRestore;
    return 0;

 fail:
    panoLocaleRestore;
    return -1;
}


// Report Results 

void WriteResults( char* script, fullPath *sfile,  AlignInfo *g, double ds( int i), int launch)
{
    char        *res, **hres, *line;
    int         optHfov;         // Has hfov being optimized?
    int         opta,optb,optc;  // same for a,b,c - the polynomial corrections
    int         format;
    int i;

    panoLocaleSave;

    hres = (char**) mymalloc( strlen(script) + g->numIm * 600 + g->numPts * 200 + 10000 ); // Do we ever need more?
    if( hres == NULL )
    {
        PrintError("Not enough memory to create resultfile");
        goto fail;
    }
    line = res = *hres;
    line += sprintf( line, "%s", script );
    line += sprintf( line, "\n*\n\n" );
    line += sprintf( line, "# ====================================================================\n");
    line += sprintf( line, "# Output  generated by Panorama Tools\n\n" );
    if( g->data != NULL )
        line += sprintf( line, "%s", (char*) g->data );

    line += sprintf( line, "\n# Panorama description\n" );
    switch( g->pano.format )
    {
        case _rectilinear:            format = PANO_FORMAT_RECTILINEAR; break;
        case _panorama:               format = PANO_FORMAT_PANORAMA; break;
        case _equirectangular:        format = PANO_FORMAT_EQUIRECTANGULAR; break;
        case _fisheye_ff:             format = PANO_FORMAT_FISHEYE_FF; break;
        case _stereographic:          format = PANO_FORMAT_STEREOGRAPHIC; break;
        case _mercator:               format = PANO_FORMAT_MERCATOR; break;
        case _trans_mercator:         format = PANO_FORMAT_TRANS_MERCATOR; break;
        case _sinusoidal:             format = PANO_FORMAT_SINUSOIDAL; break;
        case _lambert:                format = PANO_FORMAT_LAMBERT_EQUAL_AREA_CONIC; break;
        case _lambertazimuthal:       format = PANO_FORMAT_LAMBERT_AZIMUTHAL; break;
        case _hammer:                 format = PANO_FORMAT_HAMMER; break;
        case _albersequalareaconic:   format = PANO_FORMAT_ALBERS_EQUAL_AREA_CONIC; break;
        case _millercylindrical:      format = PANO_FORMAT_MILLER_CYLINDRICAL; break;
        case _panini:                 format = PANO_FORMAT_PANINI; break;
        case _equipanini:             format = PANO_FORMAT_EQUI_PANINI; break;
        case _architectural:          format = PANO_FORMAT_ARCHITECTURAL; break;
        case _orthographic:           format = PANO_FORMAT_ORTHOGRAPHIC; break;
        case _equisolid:              format = PANO_FORMAT_EQUISOLID; break;
        case _biplane:                format = PANO_FORMAT_BIPLANE; break;
	case _triplane:               format = PANO_FORMAT_TRIPLANE; break;
        case _panini_general:     format = PANO_FORMAT_PANINI_GENERAL; break;
        case _thoby:                   format = PANO_FORMAT_THOBY; break;
        default:                      format = -1; break;
    }

    line += sprintf( line, "# p f%d w"FMT_INT32" h"FMT_INT32" v%g n\"%s\"\n\n", format, g->pano.width, g->pano.height, g->pano.hfov, g->pano.name );

    line += sprintf( line, "# Parameters for Each Input Image:\n" );
    line += sprintf( line, "# (*) - optimized         (p) - preset \n\n");

    for( i=0; i<g->numIm; i++ )
    {
        switch( g->im[i].format )
        {
            case _rectilinear:        format = IMAGE_FORMAT_RECTILINEAR; break;
            case _panorama:           format = IMAGE_FORMAT_PANORAMA; break;
            case _fisheye_circ:       format = IMAGE_FORMAT_FISHEYE_EQUIDISTANCECIRC; break;
            case _fisheye_ff:         format = IMAGE_FORMAT_FISHEYE_EQUIDISTANCEFF; break;
            case _equirectangular:    format = IMAGE_FORMAT_EQUIRECTANGULAR; break;
            case _mirror:             format = IMAGE_FORMAT_MIRROR; break;
            case _orthographic:       format = IMAGE_FORMAT_FISHEYE_ORTHOGRAPHIC; break;
            case _stereographic:      format = IMAGE_FORMAT_FISHEYE_STEREOGRAPHIC; break;
            case _equisolid:          format = IMAGE_FORMAT_FISHEYE_EQUISOLID; break;
            case _thoby:              format = IMAGE_FORMAT_FISHEYE_THOBY; break;
            default:                  format = -1; break;
        }

        if( g->opt[i].hfov == 1 ||
            ( g->opt[i].hfov > 1 &&  g->opt[g->opt[i].hfov-2].hfov == 1 ))
            optHfov = 1;
        else
            optHfov = 0;

        if( g->opt[i].a == 1 ||
            ( g->opt[i].a > 1 &&  g->opt[g->opt[i].a-2].a == 1 ))
            opta = 1;
        else
            opta = 0;
                    
        if( g->opt[i].b == 1 ||
            ( g->opt[i].b > 1 &&  g->opt[g->opt[i].b-2].b == 1 ))
            optb = 1;
        else
            optb = 0;
                    
        if( g->opt[i].c == 1 ||
            ( g->opt[i].c > 1 &&  g->opt[g->opt[i].c-2].c == 1 ))
            optc = 1;
        else
            optc = 0;

        line += sprintf( line, "# Image No %d:\n", i );
        line += sprintf( line, "# Yaw:  %g deg (%c) Pitch:  %g deg (%c) \n"
                         "# Roll:   %g deg (%c) HFov:   %g deg (%c)\n"
                         "# Polynomial Coefficients: a   %f (%c); b   %f (%c); c   %f (%c)\n"
                         "# Horizontal Shift: %f (%c)   Vertical Shift:  %f (%c)\n"
                         "# TiltX: %f (%c)   TiltY:  %f (%c)\n"
                         "# TiltZ: %f (%c)   TiltScale:  %f (%c)\n"
                         "# TransX: %f (%c)   TransY:  %f (%c)\n"
                         "# TransZ: %f (%c)   \n"
                         "# Test P0: %f (%c)   Test P1:  %f (%c)\n"
                         "# Test P2: %f (%c)   Test P3:  %f (%c)\n",
                         g->im[i].yaw, ( g->opt[i].yaw ? '*' : 'p' ),
                         g->im[i].pitch,  ( g->opt[i].pitch  ? '*' : 'p' ),
                         g->im[i].roll, ( g->opt[i].roll ? '*' : 'p' ),
                         g->im[i].hfov,  (              optHfov  ? '*' : 'p' ),
                         g->im[i].cP.radial_params[0][3], (opta ? '*':'p'),
                         g->im[i].cP.radial_params[0][2], (optb ? '*':'p'),
                         g->im[i].cP.radial_params[0][1], (optc ? '*':'p'),
                         g->im[i].cP.horizontal_params[0], (g->opt[i].d ? '*':'p'),
                         g->im[i].cP.vertical_params[0], (g->opt[i].e ? '*':'p'),
                         // Tilt
                         g->im[i].cP.tilt_x, (g->opt[i].tiltXopt ? '*':'p'),
                         g->im[i].cP.tilt_y, (g->opt[i].tiltYopt ? '*':'p'),
                         g->im[i].cP.tilt_z, (g->opt[i].tiltZopt ? '*':'p'),
                         g->im[i].cP.tilt_scale, (g->opt[i].tiltScaleOpt ? '*':'p'),
                         // Trans
                         g->im[i].cP.trans_x, (g->opt[i].transXopt ? '*':'p'),
                         g->im[i].cP.trans_y, (g->opt[i].transYopt ? '*':'p'),
                         g->im[i].cP.trans_z, (g->opt[i].transZopt ? '*':'p'),
                         g->im[i].cP.trans_yaw,   (g->opt[i].transYawOpt   ? '*':'p'),
                         g->im[i].cP.trans_pitch, (g->opt[i].transPitchOpt ? '*':'p'),
                         // test parameters
                         g->im[i].cP.test_p0, (g->opt[i].testP0opt ? '*':'p'),
                         g->im[i].cP.test_p1, (g->opt[i].testP1opt ? '*':'p'),
                         g->im[i].cP.test_p2, (g->opt[i].testP2opt ? '*':'p'),
                         g->im[i].cP.test_p3, (g->opt[i].testP3opt ? '*':'p')

                    );
        if( opta || optb || optc )
        {
            line += sprintf(line,"# 4th polynomial coefficient: %g\n", 1.0 - ( g->im[i].cP.radial_params[0][3]
                                                                 + g->im[i].cP.radial_params[0][2]
                                                                 + g->im[i].cP.radial_params[0][1] ) );
        }
        line += sprintf(line, "# Command for Panorama Creation: \n" );
        line += sprintf( line, "o f%d r%g p%g y%g v%g ", format, g->im[i].roll,g->im[i].pitch,g->im[i].yaw,g->im[i].hfov);
        if( g->im[i].cP.radial )
        {
            line += sprintf( line, "a%f b%f c%f ", g->im[i].cP.radial_params[0][3], g->im[i].cP.radial_params[0][2], g->im[i].cP.radial_params[0][1]);
        }
        if( g->im[i].cP.shear )
        {
            line += sprintf( line, "g%f t%f ", g->im[i].cP.shear_x, g->im[i].cP.shear_y);
        }
        if( g->im[i].cP.tilt ) {
            line += sprintf( line, "TiX%f TiY%f TiZ%f TiS%f ", g->im[i].cP.tilt_x, g->im[i].cP.tilt_y, g->im[i].cP.tilt_z, g->im[i].cP.tilt_scale);
        }
        if( g->im[i].cP.trans ) {
            line += sprintf( line, "TrX%f TrY%f TrZ%f Tpy%f Tpp%f", g->im[i].cP.trans_x, g->im[i].cP.trans_y, g->im[i].cP.trans_z, g->im[i].cP.trans_yaw, g->im[i].cP.trans_pitch);
        }
        if( g->im[i].cP.test ) {
            line += sprintf( line, "Te0%f Te1%f Te2%f Te3%f ", g->im[i].cP.test_p0, g->im[i].cP.test_p1, g->im[i].cP.test_p2, g->im[i].cP.test_p3 );
        }
        if( g->im[i].cP.cutFrame &&
         !( g->im[i].selection.bottom != 0 || g->im[i].selection.right != 0 )) // g->im[i].format != _fisheye_circ && g->im[i].cP.cutFrame )
        {
            if( g->im[i].cP.frame != 0 )
            {
                line += sprintf( line, "m%d ",g->im[i].cP.frame );
            }
            else
            {
                line += sprintf( line, "mx%d my%d ",g->im[i].cP.fwidth, g->im[i].cP.fheight );
            }
        }

//      line += sprintf( line, "d%f e%f ", g->prefs[i]->c_prefs.horizontal_params[0], g->prefs[i]->c_prefs.vertical_params[0]);
#if 0
        if( g->prefs[i]->colCorrect )
        {
            line += sprintf( line, "t%f,%f,%f,%f,%f,%f ", g->prefs[i]->ColCoeff[0][0],
                                                        g->prefs[i]->ColCoeff[0][1],
                                                        g->prefs[i]->ColCoeff[1][0],
                                                        g->prefs[i]->ColCoeff[1][1],
                                                        g->prefs[i]->ColCoeff[2][0],
                                                        g->prefs[i]->ColCoeff[2][1]);
        }
#endif
        if( g->im[i].cP.horizontal )
        {
            line += sprintf( line, "d%f ",g->im[i].cP.horizontal_params[0]); 
        }
        if( g->im[i].cP.vertical )
        {
            line += sprintf( line, "e%f ",g->im[i].cP.vertical_params[0]);
        }
        if( g->im[i].cP.correction_mode & correction_mode_morph )
        {
            line += sprintf( line, "o " );
        }
        if( g->im[i].selection.bottom != 0 || g->im[i].selection.right != 0 ){
            if( g->im[i].cP.cutFrame ){
                line += sprintf( line, " C"FMT_INT32","FMT_INT32","FMT_INT32","FMT_INT32" ",g->im[i].selection.left, g->im[i].selection.right,
                               g->im[i].selection.top, g->im[i].selection.bottom );
            }else{
                line += sprintf( line, " S"FMT_INT32","FMT_INT32","FMT_INT32","FMT_INT32" ",g->im[i].selection.left, g->im[i].selection.right,
                               g->im[i].selection.top, g->im[i].selection.bottom );
            }
        }

        line += sprintf(line, "u%d ", g->st.feather);
#if 0
        // Print filename
        if( g->im[i].name != NULL && strlen( g->im[i].name != 0 ) ){
            line += sprintf(line, " n\"%s\" ", g->im[i].name);
        }
    
#endif
        // Add command for stitcher, depending on g->st
        // If '-' option has been set (destName != 0), do nothing
        // else add stitch commands
        if( !*(g->st.destName) )
        {
            if( i== 0 )
            {
                line += sprintf(line, "-%s ", g->st.srcName);
            }
            else if( i == g->numIm-1 )
            {
                line += sprintf(line, "+%s ", g->st.srcName);
            }
            else
            {
                line += sprintf(line, "+%s -%s ", g->st.srcName, g->st.srcName);
            }
            // Print Feather commands
        }
        line += sprintf(line, "\n\n");
    } // numIm

    if( g->numPts > 0 ) // Display Controlpoint distances
    {
        line += sprintf(line, "\n");
        line += sprintf( line, "# ==========================================================================\n");
        line += sprintf( line, "# Control Points: Distance between desired and fitted Position (in \"Pixels\")\n\n");

        for( i=0; i<g->numPts; i++ )
        {
            line += sprintf( line, "# Control Point No %d:  %g\n", i , sqrt ( ds(i)) );
        }
        // Print optimum positions for points in panorama
        for( i=0; i<g->numPts; i++ )
        {
            double x[2],y[2],xd,yd,d,D[2];

            EvaluateControlPointErrorAndComponents(i,&d,D);
            // write only normal control points, not horizontals, verticals, or lines
            if (g->cpt[i].type == 0)
            {
                GetControlPointCoordinates(i, x, y, g );
                // Write only points inside image
                if( x[0] >= 0.0 && x[0] < g->pano.width && x[1] >= 0.0 && x[1] < g->pano.width &&
                    y[0] >= 0.0 && y[0] < g->pano.height && y[1] >= 0.0 && y[1] < g->pano.height)
                {
                    xd = (x[0]+x[1]) / 2.0;
                    yd = (y[0]+y[1]) / 2.0;
                    line += sprintf( line, "C i%d c%d  x%g y%g X%g Y%g  D%g Dx%g Dy%g\n", g->cpt[i].num[0], i, x[0], y[0], xd, yd, d, D[0], D[1] );
                    line += sprintf( line, "C i%d c%d  x%g y%g X%g Y%g  D%g Dx%g Dy%g\n", g->cpt[i].num[1], i, x[1], y[1], xd, yd, d, D[0], D[1] );
                }
            }
        }
    }
    if( WriteScript( res, sfile, launch ) != 0 )
    {
        PrintError("Could not write results to scriptfile");
    }
    if( hres ) myfree( (void**)hres );

    panoLocaleRestore;
    return;

 fail:
    panoLocaleRestore;

}



// Reads 'adjust' parameters from script 'sfile'
// return 0 on success/ -1 if failed

int readAdjust( aPrefs *p,  fullPath* sfile, int insert, sPrefs *sP )
{
    char*               script;
    // Variables used by parser
    
    char                line[LINE_LENGTH], *ch;
    int                 lineNum = 0;
    int                 seto;
    int                 seti;
    
    
    panoLocaleSave;
    
    
    // Set prefs and sBuf to defaults
    
    SetAdjustDefaults( p );
    

    
    
    script = LoadScript( sfile );
    if( script == NULL )
	goto fail;
    
    
    // Parse script 
    
    ch = script;
    seto = FALSE;
    seti = FALSE;
    
    while( *ch != 0 )   {
	lineNum++;
    
	while(*ch == '\n')
	    ch++;
    
	// read a line of text into line[];
    
	nextLine( line, &ch );
	// parse line; use only if first character is p,o,m
    
	switch( line[0] ) {
	case 'i':
	    // The original i line was optional and it did not contain any information except
	    // for the filename.
	    // Hugin .pto files use the 'i' line instead of 'o' line.

	    // 'o' has priority over 'i'
	    // if 'o' has been read then skip
	    if (!seto  && !seti) {
		if( ReadImageDescription( &(p->im), &(p->sBuf), &(line[1]) ) != 0 ) {
		    PrintError("Syntax error in i-line %d (%s)", lineNum, line);
		    goto fail;
		}
		seti = TRUE;
        
	    }
	    break;
	case 'o':   // Image description
	    if( !seto ) { // Read only _one_ image
		// 'o' has priority over 'i' lines
		if( ReadImageDescription( &(p->im), &(p->sBuf), &(line[1]) ) != 0 ) {
		    PrintError( "Syntax error parsing o-line %d (%s)" , lineNum, line);
		    goto fail;
		}
		seto = TRUE;
	    }
	    break;
	case 'm':       // Mode description
	    if( ReadModeDescription( sP, &(line[1]) ) != 0 )
		{
		    PrintError( "Syntax error in m-line %d (%s)" , lineNum, line);
		    goto fail;
		}
	    break;
	case 'p':       // panorama 
	    p->pano.format  = 2; // _equirectangular by default
	    p->pano.hfov    = 360.0;
	    if( ReadPanoramaDescription( &(p->pano), &(p->sBuf), &(line[1]) ) != 0 )
		{
		    PrintError( "Syntax error in line %d" , lineNum);
		    goto fail;
		}
      switch (p->pano.format) {
        case PANO_FORMAT_RECTILINEAR:
          p->pano.format = _rectilinear;
          break;
        case PANO_FORMAT_PANORAMA:
          p->pano.format = _panorama;
          break;
        case PANO_FORMAT_EQUIRECTANGULAR:
          p->pano.format = _equirectangular;
          break;
        case PANO_FORMAT_FISHEYE_FF:
          p->pano.format = _fisheye_ff;
          break;
        case PANO_FORMAT_STEREOGRAPHIC:
          p->pano.format = _stereographic;
          break;
        case PANO_FORMAT_MERCATOR:
          p->pano.format = _mercator;
          break;
        case PANO_FORMAT_TRANS_MERCATOR:
          p->pano.format = _trans_mercator;
          break;
        case PANO_FORMAT_SINUSOIDAL:
          p->pano.format = _sinusoidal;
          break;
        case PANO_FORMAT_LAMBERT_EQUAL_AREA_CONIC:
          p->pano.format = _lambert;
          break;
        case PANO_FORMAT_LAMBERT_AZIMUTHAL:
          p->pano.format = _lambertazimuthal;
          break;
        case PANO_FORMAT_HAMMER:
          p->pano.format = _hammer;
          break;
        case PANO_FORMAT_ALBERS_EQUAL_AREA_CONIC:
          p->pano.format = _albersequalareaconic;
          break;
        case PANO_FORMAT_MILLER_CYLINDRICAL:
          p->pano.format = _millercylindrical;
          break;
        case PANO_FORMAT_PANINI:
          p->pano.format = _panini;
          break;
        case PANO_FORMAT_PANINI_GENERAL:
          p->pano.format = _panini_general;
          break;
        case PANO_FORMAT_EQUI_PANINI:
          p->pano.format = _equipanini;
          break;
        case PANO_FORMAT_ARCHITECTURAL:
          p->pano.format = _architectural;
          break;
        case PANO_FORMAT_ORTHOGRAPHIC:
          p->pano.format = _orthographic;
          break;
        case PANO_FORMAT_EQUISOLID:
          p->pano.format = _equisolid;
          break;
        case PANO_FORMAT_THOBY:
          p->pano.format = _thoby;
          break;
		case PANO_FORMAT_BIPLANE:
          p->pano.format = _biplane;
          break;
		case PANO_FORMAT_TRIPLANE:
	      p->pano.format = _triplane;
          break;
        default:
          PrintError( "Unknown panorama projection: %d", p->pano.format );
          goto fail;
        }
	    if( (p->pano.format == _rectilinear || p->pano.format == _trans_mercator) && p->pano.hfov >= 180.0 ) {
		PrintError( "Destination image must have HFOV < 180" );
		goto fail;
	    }
	    break;
        
	default: 
	    //           PrintError("Unknown line %s", line);
	    // silently ignore any unknown lines
	    break;
	}
    }
    
    if( ! (seto || seti) ) {
    PrintError( "Syntax error in scriptfile (readAdjust). It contains no 'o' line");
    goto fail;
    }
    
    // Create and Write changed scriptfile if inserting
    
    if( insert ) {
    char charToFind;
    // dmg: this is the part that I hate the most about the parser:
    // it rewrites the file over and over again, eating one line at at a time

    if (seti)
        charToFind = 'i';

    // 'o' has priority over 'i'
    if (seto)
        charToFind = 'o';

    seto = FALSE; ch = script;
    
    
    while( *ch != 0 && !seto )
        {
        while(*ch == '\n')
            ch++;
        if( *ch == charToFind )
            seto = TRUE;
        else
            {
            while(*ch != '\n' && *ch != 0)
                ch++;
            }
        }
    if( *ch == charToFind )  *ch = '!';
    
    // If this is the last image to convert: recover script file
    
    seto = FALSE; ch = script;
    
    while( *ch != 0 && !seto ) {
        while(*ch == '\n')
        ch++;
        if( *ch == charToFind )
        seto = TRUE;
        else
        {
            while(*ch != '\n' && *ch != 0)
            ch++;
        }
    }
    if( seto == FALSE ) { // No more images to convert
        ch = script;
        
        while( *ch != 0  ) {
        while(*ch == '\n')
            ch++;
        if( *ch == '!' )
            *ch = charToFind;
        else
            {
            while(*ch != '\n' && *ch != 0)
                ch++;
            }
        }
    }
    
    
    if( WriteScript( script, sfile, 0 ) != 0 )      {
        PrintError("Could not write scriptfile");
        goto fail;
    }
    }
    
    free( script);
    panoLocaleRestore;
    return 0;
    
 fail:
    if (script != NULL)
        free(script);
    panoLocaleRestore;

    return -1;



}


void readControlPoints(char* script, controlPoint *cp )
{
    struct controlPoint defCn;

    // Variables used by parser
    
    char                line[LINE_LENGTH], *ch ,*lineStart;
    int                 lineNum = 0;
    int                 i;
    int                 numPts;


    panoLocaleSave;

    defCn.num[0]    =   defCn.num[1] = -1;
    defCn.type      =   0;
    defCn.x[0]  =   defCn.x[1] =    defCn.y[0]  =   defCn.y[1] = 0;

    for(i=0; i< NUMPTS; i++)
        memcpy( &(cp[i]), &defCn, sizeof( struct controlPoint ) );
    
    numPts = 0; // reused as indices


    // Parse script 
    
    ch = script;
    
    while( *ch != 0 )
    {
        lineNum++;

        while(*ch == '\n')
            ch++;

        lineStart = ch;

        // read a line of text into line[];

        nextLine( line, &ch );

        // parse line; use only if first character is c
    
        switch( line[0] )
        {
            case 'c':       // Control Points
                defCn.num[0]    =   defCn.num[1] = -1;
                defCn.type      =   0;
                defCn.x[0]  =   defCn.x[1] =    defCn.y[0]  =   defCn.y[1] = 0;
                
                if( ReadControlPoint( &defCn, &(line[1]) ) != 0 )
                {
                    PrintError("Error in line %d", lineNum);
                    return;
                }
                if( defCn.num[1] == -1 )    // We found a partial controlpoint
                {
                    *lineStart = 0;         // script ends here
                    memcpy( &(cp[numPts]), &defCn, sizeof( struct controlPoint ) );
                    numPts++;
                }
                break;
            case '*':   // End of script-data
                *lineStart = 0; *ch = 0;
                break;
            default: 
                break;
        }
    }
    panoLocaleRestore;
}


// Fill 'word' with word starting at (*ch). Advance *ch

void nextWord( register char* word, char** ch )
{
    register char *c;
    c = *ch;
    
    c++;
    if( *c == '\"' )
    {
        c++;
        while( *c != '\"' && *c != 0 )
            *word++ = *c++;
        c++; // to eat last character
    }
    else
    {
        while( !isspace(*c) && *c != 0 )
        {
            *word++ = *c++;
        }
    }
    *word = 0;
    *ch  = c;
}

// Fill 'line' with line starting at (*ch). Advance *ch

void nextLine( register char* line, char** ch )
{
    register char *c;
    register int i;
    
    c = *ch;

    while(*c == '\n')
        c++;

    // read a line of text into line[];
        
    i=0;
    
    //Increased by Max Lyons (January 12, 2003) to increase size of optimizer
    //lines that can be read (previously hard-coded to 255 characters).
    while( *c != 0 && *c != '\n' && i++<LINE_LENGTH)
        *line++ = *c++;
    *line = 0;
    *ch = c;
}

// Number of lines in script starting with character 'first' 

int numLines( char* script, char first )
{
    register char *ch;
    int result = 0;
    
    ch = script;
    
    while( *ch != 0 )
    {
        // Advance to linestart
        while(*ch == '\n')
            ch++;
        if( *ch == first )
            result++;

        while(*ch != '\n' && *ch != 0)
            ch++;
    }
    return result;
}

#undef  MY_SSCANF
#define MY_SSCANF( str, format, ptr )       if( sscanf( str, format, ptr ) != 1 )   \
    {                               \
        PrintError("Syntax error in script: Could read value for variable");\
        return -1;                          \
    }

#undef READ_VAR
#define READ_VAR(format, ptr )      nextWord( buf, &ch );           \
                                    MY_SSCANF( buf, format, ptr );


// Parse a line describing a single Controlpoint

static int ReadControlPoint( controlPoint * cptr, char *line)
{
    controlPoint        cp;
    char *ch    = line;
    int setn, setN, setx, setX, sety, setY; 
    char buf[LINE_LENGTH];
    
    
    setn = setN = setx= setX = sety = setY = FALSE;
    
    memcpy(  &cp, cptr, sizeof(controlPoint) );
    
    while( *ch != 0)
    {
        switch(*ch)
        {
            case 't':   READ_VAR("%d", &(cp.type));     
                        break;
            case 'n':   READ_VAR("%d", &(cp.num[0]));
                        setn = TRUE;
                        break;
            case 'N':   READ_VAR("%d", &(cp.num[1]));
                        setN = TRUE;
                        break;
            case 'x':   READ_VAR("%lf", &(cp.x[0]));
                        setx = TRUE;
                        break;
            case 'X':   READ_VAR("%lf", &(cp.x[1]));
                        setX = TRUE;
                        break;
            case 'y':   READ_VAR("%lf", &(cp.y[0]));
                        sety = TRUE;
                        break;
            case 'Y':   READ_VAR("%lf", &(cp.y[1]));
                        setY = TRUE;
                        break;
            case 'i':   READ_VAR("%d", &(cp.num[0]));
                        cp.num[1] = cp.num[0];
                        setn = TRUE;
                        setN = TRUE;
            default:    ch++;
                        break;
        }
    }
    
    // Check values
    
    if( setn    == FALSE || setN    == FALSE || setx == FALSE || setX == FALSE ||
        sety    == FALSE || setY    == FALSE )
    {
            PrintError("Missing Control Point Parameter");
            return -1;
    }
    else if( cp.type < 0 )
    {
            PrintError("Control Point Type must be positive");
            return -1;
    }
// Joost: cp coordinates can be possible, no problem!  
//  else if( cp.x[0] < 0 || cp.y[0] < 0 || cp.x[1] < 0 || cp.y[1] < 0)
//  {
//          PrintError("Pixel Coordinates must be positive");
//          return -1;
//  }
    else // looks ok so far
    {
        memcpy( cptr, &cp, sizeof(controlPoint) );
        return 0;
    }
}


// Parse a line describing a single image

static int ReadImageDescription( Image *imPtr, stBuf *sPtr, char *line )
{
    // This function parses the i- and -o lines

    Image im;
    stBuf sBuf;
    char *ch = line;
    char buf[LINE_LENGTH];
    int  i;
    int    cropping = 0;
    int tempInt;
    char typeParm;
    int32_t tempInt32;

    memcpy( &im,    imPtr,   sizeof(Image) );
    memcpy( &sBuf,  sPtr,    sizeof(stBuf ));
    
    //    panoPrintImage("Before read image", imPtr);
   //  printf("************************************* Before Cut Frame %d \n", im.cP.cutFrame);
    while( *ch != 0) {
        switch(*ch) {
        case 'f':   READ_VAR( FMT_INT32, &tempInt32 );
            tempInt = panoExternalToInternalInputProjection(tempInt32);
            if (tempInt < 0) {
                PrintError("Syntax error in script.  Projection not known: %ud", tempInt32);
                return -1;
            }
            im.format = tempInt;
            if( im.format == _panorama || im.format == _equirectangular )
                im.cP.correction_mode |= correction_mode_vertical;
            break;
        case 'v':   READ_VAR( "%lf", &im.hfov );
            break;
        case 'y':   
            READ_VAR( "%lf", &im.yaw);
            break;
        case 'p':   READ_VAR( "%lf", &im.pitch);
            break;
        case 'r':   READ_VAR( "%lf", &im.roll);
            break;
            
            
        case 'a':   READ_VAR( "%lf", &(im.cP.radial_params[0][3]));
            im.cP.radial    = TRUE;
            break;
        case 'b':   READ_VAR("%lf", &(im.cP.radial_params[0][2]));
            im.cP.radial    = TRUE;
            break;
        case 'c':   READ_VAR("%lf", &(im.cP.radial_params[0][1]));
            im.cP.radial    = TRUE;
            break;
        case 'd':   READ_VAR("%lf", &(im.cP.horizontal_params[0]));
            im.cP.horizontal    = TRUE;
            break;
        case 'e':   READ_VAR("%lf", &(im.cP.vertical_params[0]));
            im.cP.vertical = TRUE;
            break;
        case 'g':   READ_VAR("%lf", &(im.cP.shear_x));
            im.cP.shear     = TRUE;
            break;
        case 't':   READ_VAR("%lf", &(im.cP.shear_y));
            im.cP.shear = TRUE;
            break;
        case 'T':   
            ch++;
            switch (*ch) {
            case 'i':
                ch++;
                switch (*ch) {
                case 'X':
                    READ_VAR("%lf",&(im.cP.tilt_x));
                    break;
                case 'Y':
                    READ_VAR("%lf",&(im.cP.tilt_y));
                    break;
                case 'Z':
                    READ_VAR("%lf",&(im.cP.tilt_z));
                    break;
                case 'S':
                    READ_VAR("%lf",&(im.cP.tilt_scale));
                    break;
                default:
                    PrintError("Unknown variable name Ti%c in script", *ch);
                    return -1;
                }
                im.cP.tilt    = TRUE;
                break;
            case 'r':
                ch++;
                switch (*ch) {
                case 'X':
                    READ_VAR("%lf",&(im.cP.trans_x));
                    break;
                case 'Y':
                    READ_VAR("%lf",&(im.cP.trans_y));
                    break;
                case 'Z':
                    READ_VAR("%lf",&(im.cP.trans_z));
                    break;
                default:
                    PrintError("Unknown variable name Tr%c in script", *ch);
                    return -1;
                }
                if (im.cP.trans_x != 0.0 ||
                    im.cP.trans_y != 0.0 || 
                    im.cP.trans_z != 0.0) {
                    im.cP.trans    = TRUE;
                }
                break;
            case 'p':
                ch++;
                switch (*ch) {
                case 'y':
                    READ_VAR("%lf",&(im.cP.trans_yaw));
                    break;
                case 'p':
                    READ_VAR("%lf",&(im.cP.trans_pitch));
                    break;
                default:
                    PrintError("Unknown variable name Tp%c in script", *ch);
                    return -1;
                }
                break;
            case 'e':
                ch++;
                switch (*ch) {
                case '0':
                    READ_VAR("%lf",&(im.cP.test_p0));
                    break;
                case '1':
                    READ_VAR("%lf",&(im.cP.test_p1));
                    break;
                case '2':
                    READ_VAR("%lf",&(im.cP.test_p2));
                    break;
                case '3':
                    READ_VAR("%lf",&(im.cP.test_p3));
                    break;
                default:
                    PrintError("Unknown variable name Te%c in script", *ch);
                    return -1;
                }
                im.cP.test    = TRUE;
                break;
            default:
                PrintError("Unkonwn parameter T%c in script", *ch);
                return -1;
            }
            break;
        case '+':   nextWord( buf, &ch );
            PrintError("Obsolete + parameter is ignored in image description");
            sprintf( sBuf.srcName, "%s", buf);
            break;
        case '-':   nextWord( buf, &ch );
            PrintError("Obsolete - parameter is ignored in image description");
            sprintf( sBuf.destName, "%s", buf );
            break;
            
            
        case 'S':  
            if (cropping) {
                PrintError("Contradictory cropping specified. S cropping ignored\n");
                // Eat next token
                nextWord( buf, &ch );       
                break;
            }
            cropping = 1;
            
            nextWord( buf, &ch );       
            sscanf( buf, FMT_INT32","FMT_INT32","FMT_INT32","FMT_INT32, &im.selection.left, &im.selection.right, &im.selection.top, &im.selection.bottom );
            break;
        case 'C':  
            if (cropping) {
                PrintError("Contradictory cropping specified. C cropping ignored\n");
                // Eat next token
                nextWord( buf, &ch );       
                break;
            }
            cropping = 1;
            
            nextWord( buf, &ch );       
            sscanf( buf, FMT_INT32","FMT_INT32","FMT_INT32","FMT_INT32, &im.selection.left, &im.selection.right, &im.selection.top, &im.selection.bottom );
            im.cP.cutFrame = TRUE;
            break;
            
        case 'm':  // Frame
            //THiS NEEDS A GOOD FIX...
            typeParm = *(ch+1);             
            // first consume the parameter
            if (typeParm =='x' || typeParm == 'y') {
                // Consume next character, then read parm
                ch++;
                READ_VAR( "%d", &tempInt);
            }
            else {
                READ_VAR( "%d", &tempInt);
            }
            
            if (tempInt == 0) {
                // value of zero, just ignore
                break;
            } 
            
            // Sometimes this is specified to force a zero. In this case
            // issue a warning and ignore
            if (cropping) {
                PrintError("Contradictory cropping specified. M cropping ignored\n");
                break;
            }
            // Eat next token to avoid error
            cropping = 1;
            im.cP.cutFrame = TRUE;
            
            switch( typeParm ) {
            case 'x':
                im.cP.fwidth = tempInt;
                //READ_VAR( "%d", &im.cP.fwidth );
                break;
            case 'y':
                im.cP.fheight = tempInt;
                //READ_VAR( "%d", &im.cP.fheight );
                //im.cP.cutFrame = TRUE;
                break;
            default:
                im.cP.frame = tempInt;
                READ_VAR( "%d", &(im.cP.frame) );
                // im.cP.cutFrame = TRUE;                                           
                break;
            }
            
            break;
        case 's':   READ_VAR( "%d", &sBuf.seam );
            PrintError("Obsolete s parameter ignored in image description");
            break;
            
        case 'o':   
            ch++;
            im.cP.correction_mode |= correction_mode_morph;
            break;
        case 'u':
            READ_VAR( "%d", &i );
            PrintError("Feathering is ignored. Use PTmasker");
            break;  
        case 'w':   READ_VAR( FMT_INT32, &im.width );
            break;
        case 'h':   READ_VAR( FMT_INT32, &im.height );
            break;
        case 'n':  // Name string (used for input image name)
            nextWord( buf, &ch );
            strcpy( im.name, buf );
            break;  
        case 'K':
        case 'V':
            // Used by Hugin. Silently ignore until next space. This way we can accept .pto files for processing
            nextWord( buf, &ch );       
        break;
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            // skip characters and tabs
            ch++;
        break;
        default:
            printf("REturning...........\n");
            PrintError("Illegal token in adjust line [%c]  rest of line [%s]", *ch, ch);
            return -1;
        }
    }
    
    //  printf("************************************* A Cut Frame %d \n", im.cP.cutFrame);
    // Set 4th polynomial parameter
    
    im.cP.radial_params[0][0] = 1.0 - ( im.cP.radial_params[0][3] + im.cP.radial_params[0][2]
                        + im.cP.radial_params[0][1] ) ;
    
    SetEquColor( &im.cP );
    SetCorrectionRadius( &im.cP );  
    // Do checks
    
    // appears ok
    
    memcpy( imPtr,  &im, sizeof(Image) );
    memcpy( sPtr,   &sBuf, sizeof(stBuf ) );

    //    panoPrintImage("After read image", imPtr);

    return 0;
}

static int ReadPanoramaDescription( Image *imPtr, stBuf *sPtr, char *line )
{
    //  This function parses the p- line

    Image im;
    stBuf sBuf;
    char *ch = line;
    char buf[LINE_LENGTH];
    char *b;
    int  i;
    int    cropping = 0;
    double tempDbl;
    
    memcpy( &im,    imPtr,   sizeof(Image) );
    memcpy( &sBuf,  sPtr,    sizeof(stBuf ));
    
    //  printf("************************************* Before Cut Frame %d \n", im.cP.cutFrame);
    while( *ch != 0) {
        switch(*ch) {
        case 'w':   READ_VAR( FMT_INT32, &im.width );
        break;
        case 'h':   READ_VAR( FMT_INT32, &im.height );
        break;
        case 'f':   READ_VAR( FMT_INT32, &im.format );
        if( im.format == _panorama || im.format == _equirectangular )
            im.cP.correction_mode |= correction_mode_vertical;
        break;
        case 'P':
        nextWord(buf, &ch);
        b = strtok(buf, " \"");
        if (b != NULL) {
            while (b != NULL) {
            if (sscanf(b, "%lf", &tempDbl) == 1) {
                if (++im.formatParamCount > PANO_PROJECTION_MAX_PARMS) {
                PrintError("Illegal number of projection parameters. Maximum is %d", PANO_PROJECTION_MAX_PARMS);
                return -1;
                }
                im.formatParam[im.formatParamCount - 1] = tempDbl;
                b = strtok(NULL, " \"");
            } else {
                PrintError("Illegal value in P parameter %s", b);
                return -1;
            }
            }
        } 
        break;
        case 'v':   READ_VAR( "%lf", &im.hfov );
        break;
        
        case 'n':  // Name string (used for panorama format)
        nextWord( buf, &ch );
        strcpy( im.name, buf );
        break;  
        
        case 'u':  //Feather
        READ_VAR( "%d", &i );
        PrintError("Feathering is ignored. Use PTmasker");
        break;  
        case 'k':  // Colour correction
        READ_VAR( "%d", &i );
        PrintError("Colour correction ignored (k). Use PTblender");
        break;  
        case 'd':  // Colour correction
        READ_VAR( "%d", &i );
        PrintError("Colour correction ignored (d). Use PTblender");
        break;  
        case 'b':  // Colour correction
        READ_VAR( "%d", &i );
        PrintError("Colour correction ignored parameter ignored (b). Use PTblender");
        break;  
        case ' ':
        case '\t':
        case '\n':
        case '\r':
        // skip characters and tabs
        ch++;
        break;
        default:
        PrintError("Illegal token in 'p'-line [%d] [%c] [%s]", *ch, *ch, ch);
        ch++;
        break;
        }
    }
    
    // I am not sure this is needed, but I am not sure it is not :)
    // code inherited from ReadImageDescription
    im.cP.radial_params[0][0] = 1.0 - ( im.cP.radial_params[0][3] + im.cP.radial_params[0][2]
                        + im.cP.radial_params[0][1] ) ;
    
    SetEquColor( &im.cP );
    SetCorrectionRadius( &im.cP );  
    
    memcpy( imPtr,  &im,   sizeof(Image) );
    memcpy( sPtr,   &sBuf, sizeof(stBuf ) );
    return 0;
}




// Parse a line describing modes

static int ReadModeDescription( sPrefs *sP, char *line )
{
    sPrefs theSprefs;
    char *ch = line;
    char buf[LINE_LENGTH];
    double sigma = 0;
    int n;

    panoLocaleSave;

    memcpy( &theSprefs,     sP,  sizeof(sPrefs) );

    // set some default values
    setFcnPanoHuberSigma(0);
    
    while( *ch != 0)
    {
        switch(*ch)
        {
            case 'g':   READ_VAR( "%lf", &theSprefs.gamma );
                        if( theSprefs.gamma <= 0.0 )
                            goto fail;
                        break;
            case 'i':   READ_VAR( "%d", &theSprefs.interpolator );
                        if( theSprefs.interpolator < 0 ||  theSprefs.interpolator > 23)
                            theSprefs.interpolator = 0;
                        break;
            case 'p':   READ_VAR( "%d", &theSprefs.optCreatePano );
                        if(theSprefs.optCreatePano != 0)
                            theSprefs.optCreatePano = TRUE;
                        break;
            case 'f':   READ_VAR( "%d", &n );
                        if( n == 0 ) 
                          theSprefs.fastStep = FAST_TRANSFORM_STEP_NORMAL;    
                        else if( n == 1 ) 
                          theSprefs.fastStep = FAST_TRANSFORM_STEP_MORPH;
                        else
                          theSprefs.fastStep = FAST_TRANSFORM_STEP_NONE;
                        break;
            case 'm':   READ_VAR( "%lf", &sigma);
                        setFcnPanoHuberSigma(sigma);
                        break;
            default:    ch++;
                        break;
        }
    }
    
    // appears ok
    
    memcpy( sP,  &theSprefs,    sizeof(sPrefs) );
    panoLocaleRestore;
    return 0;

 fail:
    panoLocaleRestore;
    return -1;

}

// Parse a string desscribing VRPanoOptions

int getVRPanoOptions( VRPanoOptions *v, char *line )
{
    char            *ch = line;
    char            buf[LINE_LENGTH];
    VRPanoOptions   VRopt;

    panoLocaleSave;

    memcpy( &VRopt, v, sizeof( VRPanoOptions ) );
        
    while( *ch != 0)
    {
        switch(*ch)
        {
            case 'w':   READ_VAR("%d", &VRopt.width); 
                        break;
            case 'h':   READ_VAR("%d", &VRopt.height);
                        break;
            case 'p':   READ_VAR("%lf", &VRopt.pan);
                        break;
            case 't':   READ_VAR("%lf", &VRopt.tilt);
                        break;
            case 'v':   READ_VAR("%lf", &VRopt.fov);
                        break;
            case 'c':   READ_VAR("%d", &VRopt.codec);
                        break;
            case 'q':   READ_VAR("%d", &VRopt.cquality);
                        break;
            case 'g':   READ_VAR("%d", &VRopt.progressive);
                        break;
            default:    ch++;
                        break;
        }
    }
    memcpy( v, &VRopt, sizeof( VRPanoOptions ) );
    panoLocaleRestore;

    return 0;
}



// Read coordinates of positions

int readPositions( char* script, transformCoord *tP )
{
    // Variables used by parser
    
    char                line[LINE_LENGTH], *ch ;
    int                 lineNum = 0;
    int                 nr=0,np=0;


    panoLocaleSave;

    // Determine number of images and control points


    tP->nump    = numLines( script, 'P' );
    tP->numr    = numLines( script, 'R' );

    // Allocate Space for Pointers to images, preferences and control points
    
    tP->r       = (CoordInfo*)      malloc( tP->numr    * sizeof(CoordInfo) );
    tP->p       = (CoordInfo*)      malloc( tP->nump    * sizeof(CoordInfo) );

    if( tP->r == NULL || tP->p == NULL )
    {
        PrintError("Not enough memory");
        goto fail;
    }


    // Parse script 
    
    ch = script;

    while( *ch != 0 )
    {
        lineNum++;

        while(*ch == '\n')
            ch++;

        // read a line of text into line[];

        nextLine( line, &ch );
        // parse line; use only if first character is p,o,m
    
        switch( line[0] )
        {
        case 'P':   // Coordinates as is
                    if( ReadCoordinates( &tP->p[np++], &(line[1]) ) != 0 )
                    {
                        PrintError( "Syntax error in line %d" , lineNum);
                        goto fail;
                    }
                    break;
        case 'R':   // Coordinate values requested
                    if( ReadCoordinates( &tP->r[nr++], &(line[1]) ) != 0 )
                    {
                        PrintError( "Syntax error in line %d" , lineNum);
                        goto fail;
                    }
                    break;
         default: break;
        }
    }
    panoLocaleRestore;

    return 0;

 fail:
    panoLocaleRestore;

    return -1;
    

}


static int ReadCoordinates(     CoordInfo   *cp, char *line )
{
    CoordInfo   ci;
    char *ch = line;
    char buf[LINE_LENGTH];
    
    ci.num = ci.set[0] = ci.set[1] = ci.set[2] = 0;
    ci.x[0] = ci.x[1] = ci.x[2] = 1.0;

    
    while( *ch != 0)
    {
        switch(*ch)
        {
            case 'c':   READ_VAR( "%d", &ci.num );
                        break;
            case 'i':   READ_VAR( "%d", &ci.num );
                        ci.num -= 2;
                        break;
            case 'X':   READ_VAR( "%lf", &ci.x[0] );
                        ci.set[0] = TRUE;
                        break;
            case 'Y':   READ_VAR( "%lf", &ci.x[1] );
                        ci.set[1] = TRUE;
                        break;
            case 'Z':   READ_VAR( "%lf", &ci.x[2] );
                        ci.set[2] = TRUE;
                        break;
            default:    ch++;
                        break;
        }
    }
    
    // appears ok
    
    memcpy( cp,  &ci,   sizeof(CoordInfo) );
    return 0;
}

int ReadMorphPoints( char *script, AlignInfo *gl, int nIm )
{
    // Variables used by parser
    
    char                line[LINE_LENGTH], *ch ;
    int                 lineNum = 0;
    int                 np = 0;
    controlPoint        cp;
    void                *tmp;


    panoLocaleSave;
    
    // Determine number of morph control points


    gl->numPts  = numLines( script, 'C' );
    if( gl->numPts == 0 )
        goto success;

    // Allocate Space for Pointers to images, preferences and control points
    
    gl->cpt     = (controlPoint*)   malloc( gl->numPts  * sizeof(controlPoint) );

    if( gl->cpt == NULL )
    {
        PrintError("Not enough memory");
        goto fail;
    }


    // Parse script 
    
    ch = script;

    while( *ch != 0 )
    {
        lineNum++;

        while(*ch == '\n')
            ch++;

        // read a line of text into line[];

        nextLine( line, &ch );
        // parse line; use only if first character is p,o,m
    
        switch( line[0] )
        {
        case 'C':   // Coordinates as is
                    cp.type = 0;
                    if(  ReadControlPoint( &cp, &(line[1]) ) != 0 )
                    {
                        PrintError( "Syntax error in line %d" , lineNum);
                        goto fail;
                    }
                    if( cp.num[0] == nIm )
                    {
                        cp.num[0] = 0;
                        cp.num[1] = 1;
                        memcpy( &gl->cpt[np], &cp, sizeof( controlPoint ));
                        np++;
                    }
         default: break;
        }
    }
    

    tmp =  realloc( gl->cpt, np * sizeof( controlPoint ) );
    if( tmp == NULL )   goto fail;
    gl->numPts=np; gl->cpt = (controlPoint*)tmp; 

 success:
    panoLocaleRestore;

    return np;
 fail:
    panoLocaleRestore;
    return -1;

}

void SetCoordDefaults( CoordInfo *c, int num )
{
        c->num  = num;
        c->x[0] = (double) num;
        c->x[1] = c->x[2] = 0.0;
        c->set[0] = c->set[1] = c->set[2] = TRUE;
}


aPrefs* readAdjustLine( fullPath *theScript ){  
    sPrefs sP;
    aPrefs *aP = (aPrefs*)malloc(sizeof(aPrefs));
    if(aP==NULL) return NULL;
    SetAdjustDefaults( aP );
    SetSizeDefaults( &sP );

    if( readAdjust( aP, theScript, 1, &sP ) != 0 ){
        PrintError("Error processing script file" );
        return NULL;
    }

    // Use modevalues read from script
    aP->interpolator  = sP.interpolator;
    aP->gamma         = sP.gamma;
    aP->fastStep      = sP.fastStep;
                
    // Parse script again, now reading triangles if morphing requested
    if( aP->im.cP.correction_mode & correction_mode_morph ){
        char*               script;
        AlignInfo           ainf;
        int                 nIm, nPts; // Number of image being processed
        Image               im[2];
                    
        script = LoadScript( theScript) ;
        if( script != NULL ){   
            nIm = numLines( script, '!' ) - 1;
                        
            if( nIm < 0)
            nIm = numLines( script, 'o' ) - 1;
                    
            // Set ainf
            ainf.nt     = 0;
            ainf.t      = NULL;
            ainf.numIm  = 2;
            ainf.im     = im;
            memcpy( &ainf.pano, &aP->pano, sizeof( Image ));
            memcpy( &ainf.im[0], &aP->pano, sizeof( Image ));
            memcpy( &ainf.im[1], &aP->pano, sizeof( Image ));
                        
            nPts = ReadMorphPoints( script, &ainf, nIm );
            if(nPts > 0){
                AddEdgePoints( &ainf );
                TriangulatePoints( &ainf, 1 );
                aP->nt = ainf.nt;
                if(aP->nt > 0){
                    SortControlPoints( &ainf, 1 );
                    SetSourceTriangles( &ainf, 1, &aP->td  );
                    SetDestTriangles( &ainf, 1, &aP->ts  );
                }
            }
            if(ainf.numPts > 0) free(ainf.cpt);
            free( script );
        }
    }

    //panoDumpAdjustData(aP, "readAdjustLine", 0);


    return aP;
}   


char *panoParserFindOLine(char *script, int index)
{
    char *ptr;
    int count = 0;


    ptr = script;
    while (ptr != NULL) {
    if (*ptr == 'o') {
        if (count == index) {
        // we have found it
        int length;
        char *temp;
        char *result;
        // how big is it?
        temp = strchr(ptr, '\n');
        if (temp == NULL)
            length = strlen(ptr);
        else
            length = temp -ptr;
        //allocate it

        result = calloc(length + 1, 1);
        if (result == NULL) {
            PrintError("Not enought memory");
            return NULL;
        } else {
            strncpy(result, ptr, length);
        }
        return result;
        } else {
        count++;
        }
    }
    // find next beginning of line
    ptr = strchr(ptr, '\n');
    ptr++;

    }
    return NULL;
}


int panoExternalToInternalInputProjection(int input)
{
    // Internal and external projection do not match, unfortunately.
    // So this code does the remapping

    switch (input) {
    case IMAGE_FORMAT_RECTILINEAR:                return(_rectilinear);
    case IMAGE_FORMAT_PANORAMA:                   return(_panorama);
    case IMAGE_FORMAT_FISHEYE_EQUIDISTANCECIRC:   return(_fisheye_circ);
    case IMAGE_FORMAT_FISHEYE_EQUIDISTANCEFF:     return(_fisheye_ff);
    case IMAGE_FORMAT_EQUIRECTANGULAR:            return(_equirectangular);
    case IMAGE_FORMAT_MIRROR:                     return(_mirror);
    case IMAGE_FORMAT_FISHEYE_ORTHOGRAPHIC:       return(_orthographic);
    case IMAGE_FORMAT_FISHEYE_STEREOGRAPHIC:      return(_stereographic);
    case IMAGE_FORMAT_FISHEYE_EQUISOLID:          return(_equisolid);
    case IMAGE_FORMAT_FISHEYE_THOBY:              return(_thoby);
    default:     return -1;
  }
}
