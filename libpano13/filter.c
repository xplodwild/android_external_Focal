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


// Program specific includes

#include "filter.h"


// Standard C includes

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <math.h>
#include <tiff.h>
#include <assert.h>





//   Main entry for all Panorama Tools. Dispatches to individual tools using
//   the selector entry. 


// These globals are only used and valid during dialog set-up! 


TrformStr               *gTrPtr;
sPrefs                  *gsPrPtr;



void dispatch (TrformStr *TrPtr, sPrefs *spref)
{
        panoPrefs       prefs, *prPtr;
        char version[10];
        

        if( TrPtr->src->bitsPerPixel != 32 && TrPtr->src->bitsPerPixel != 24 &&
                TrPtr->src->bitsPerPixel != 64 && TrPtr->src->bitsPerPixel != 48 &&
                TrPtr->src->bitsPerPixel != 128 && TrPtr->src->bitsPerPixel != 96)      // Only support 3/4 byte/short pixels
        {
                PrintError( "Please convert image to 24/32/48/64/96/128 bit pixelsize.");
                PrintError( "Pixelsize is now  %d", (int)TrPtr->src->bitsPerPixel );            
                TrPtr->success = 0;
                return;
        }
        
        TrPtr->dest->bitsPerPixel = TrPtr->src->bitsPerPixel;

        // Check version of preferences file
        // only if we are not using _usedata
        
        if ((TrPtr->mode & 7) != _usedata)
        {
                if (spref == NULL)
                {
                        PrintError("spref cannot be NULL");
                }
                else
                {
			if( readPrefs( version, _version ) != 0 || strcmp( version, PREF_VERSION ) != 0 )
                        {
				writePrefs( PREF_VERSION, _version );
                                SetSizeDefaults( spref);
                                writePrefs( (char*)spref, _sizep );
                
                                SetPrefDefaults( &prefs, _perspective );
                                writePrefs( (char*)&prefs.pP, _perspective );
                                SetPrefDefaults( &prefs, _correct );
                                writePrefs( (char*)&prefs.cP, _correct );
                                SetPrefDefaults( &prefs, _remap );
                                writePrefs( (char*)&prefs.rP, _remap );
                                SetPrefDefaults( &prefs, _adjust );
                                writePrefs( (char*)&prefs.aP, _adjust );
                                SetPrefDefaults( &prefs, _panleft );
                                writePrefs( (char*)&prefs.pc, _panleft );
                
                        }
                }
        }

        // Read and/or set preferences; Do Xform
        
        gTrPtr = TrPtr;
        gsPrPtr = spref;


        switch( TrPtr->mode & 7 )
        {
                case _interactive:              // Display dialog, set prefs, Do Xform
                                        if( readPrefs( (char*)spref, _sizep ) != 0 ) 
                                                        SetSizeDefaults( spref);
                                        prPtr = &prefs;
                                        if( readPrefs( (char*)prPtr, TrPtr->tool ) != 0 )
                                                SetPrefDefaults( prPtr,  TrPtr->tool);
                                        if( !SetPrefs(  prPtr ))
                                        {
                                                TrPtr->success = 0;
                                        }
                                        else
                                        {
                                                TrPtr->interpolator = spref->interpolator;
                                                TrPtr->gamma        = spref->gamma;
                                                TrPtr->fastStep     = spref->fastStep;
                                                        
                                                writePrefs( (char*)prPtr, TrPtr->tool );
                                                writePrefs( (char*)spref, _sizep );
                                                DoTransForm( TrPtr, prPtr );
                                        }
                                        break;
                case _setprefs:                 // Display dialog, set prefs
                                        if( readPrefs( (char*)spref, _sizep ) != 0 ) 
                                                        SetSizeDefaults( spref);
                                        prPtr = &prefs;
                                        if( readPrefs( (char*)prPtr, TrPtr->tool ) != 0 )
                                                SetPrefDefaults( prPtr,  TrPtr->tool);
                                        if( SetPrefs( prPtr ) )
                                        {
                                                writePrefs( (char*)spref, _sizep );
                                                writePrefs( (char*)prPtr, TrPtr->tool );
                                                TrPtr->success = 1;
                                        }
                                        else
                                                TrPtr->success = 0;
                                        break;
                case _useprefs:                 // Read prefs, do Xform
                                        if( readPrefs( (char*)spref, _sizep ) != 0 ) 
                                                        SetSizeDefaults( spref);
                                        prPtr = &prefs;
                                        if( readPrefs( (char*)prPtr, TrPtr->tool ) != 0 )
                                                SetPrefDefaults( prPtr,  TrPtr->tool);
                                        DoTransForm( TrPtr, prPtr );
                                        break;
                case _usedata:                  // ignore prefs, do Xform
                                        prPtr = (panoPrefs *) TrPtr->data;
                                        DoTransForm( TrPtr, prPtr );
                                        break;
                default:        TrPtr->success = 0;
                                        break;
        }
        
        return;
}
                                        
        


void DoTransForm( TrformStr *TrPtr, panoPrefs *prPtr )
{

        // Dispatch to selected tool
        
        
        switch( TrPtr->tool )
        {
                case _perspective:      
                        perspective     (TrPtr, &prPtr->pP);
                        break;
                case _remap:            
                        remap           (TrPtr, &prPtr->rP);
                        break;
                case _correct:          
                        correct         (TrPtr, &prPtr->cP);  
                        break;
                case _adjust:           
                        adjust          (TrPtr, &prPtr->aP); 
                        break;
//              case _interpolate:              
//                      interp      (TrPtr, &prPtr->iP);
//                      break;
#if 0
                case _panright:
                case _panleft:
                case _panup:
                case _pandown:
                case _zoomin:
                case _zoomout:
                case _apply:
                case _getPano:
                case _increment:
                        pan(TrPtr,  &prPtr->pc);
                        break;
#endif
        }
        Progress( _disposeProgress, "" ); 
        return;
        
}


void SetPrefDefaults(panoPrefs *prPtr,  int selector)
{
        // Dispatch to selected tool
        
        switch( selector )
        {
                case _perspective:      
                        SetPerspectiveDefaults( &prPtr->pP); 
                        break;
                case _remap:            
                        SetRemapDefaults( &prPtr->rP); 
                        break;
                case _correct:
                        SetCorrectDefaults( &prPtr->cP);  
                        break;
                case _adjust:           
                        SetAdjustDefaults( &prPtr->aP); 
                        break;
//              case _interpolate:              
//                      SetInterpolateDefaults( &prPtr->iP );
//                      break;
                case _panright:
                case _panleft:
                case _panup:
                case _pandown:
                case _zoomin:
                case _zoomout:
                case _apply:
                case _getPano:
                case _increment:
                        SetPanDefaults( &prPtr->pc );
                        break;
        }
        return;
}

void    SetPanDefaults( panControls *pc)
{
        pc->panAngle    = 15.0;
        pc->zoomFactor  = 30.0;
}


int SetPrefs(  panoPrefs *prPtr )
{
        // Dispatch to selected tool
        
        switch( gTrPtr->tool )
        {
                case _perspective:      
                        return SetPerspectivePrefs      ( &prPtr->pP );
                        break;
                case _remap:            
                        return SetRemapPrefs            ( &prPtr->rP );
                        break;
                case _correct:          
                        return SetCorrectPrefs          (  &prPtr->cP ); 
                        break;
                case _adjust:           
                        return SetAdjustPrefs           (  &prPtr->aP );
                        break;
//              case _interpolate:              
//                      return SetInterpPrefs           (  &prPtr->iP );
//                      break;
                case _panright:
                case _panleft:
                case _panup:
                case _pandown:
                case _zoomin:
                case _zoomout:
                case _apply:
                case _getPano:
                case _increment:
                        return TRUE;
                        break;
        }
        return FALSE;
}



//   Filter function; src and dest should be equal sized


void filter( TrformStr *TrPtr, flfn func, flfn16 func16, void* params, int color)
{
        register int            x, y;                   // Loop through destination image
        register int            i,  col;                // Auxilliary loop variables
        int                             skip = 0;               // Update progress counter
        unsigned char           *dest, *src;    // Source and destination image data
                                                                                // Default message to be displayed by progress reporter
        char*                           progressMessage = "Something is wrong here";
        char                percent[8];         // Number displayed by Progress reporter
        int                                     valid;                  // Is this pixel valid? (i.e. inside source image)
        long                            coeff;                  // pixel coefficient in destination image

        int                             xs, ys;                 // Source screen coordinates 

        // Variables used to convert screen coordinates to cartesian coordinates
        
        int                             w2 = (int)(TrPtr->dest->width / 2.0 - 0.5);  // Steve's L
        int                             h2 = (int)(TrPtr->dest->height / 2.0 - 0.5);  

        // Selection rectangle
        PTRect                  destRect;

        // Jim W July 11 2004 for 16bit
        int                                     BytesPerPixel = 0;
        int                                     FirstColorByte = 0;
        int                                     SamplesPerPixel = 0;
        int                                     BytesPerSample = 0;
        
        switch( TrPtr->src->bitsPerPixel ){
                case 128:       FirstColorByte = 4; BytesPerPixel = 16; SamplesPerPixel = 4; BytesPerSample = 4; break;
                case 96:        FirstColorByte = 0; BytesPerPixel = 12; SamplesPerPixel = 3; BytesPerSample = 4; break;
                case 64:        FirstColorByte = 2; BytesPerPixel = 8; SamplesPerPixel = 4; BytesPerSample = 2; break;
                case 48:        FirstColorByte = 0; BytesPerPixel = 6; SamplesPerPixel = 3; BytesPerSample = 2; break;
                case 32:        FirstColorByte = 1; BytesPerPixel = 4; SamplesPerPixel = 4; BytesPerSample = 1; break;
                case 24:        FirstColorByte = 0; BytesPerPixel = 3; SamplesPerPixel = 3; BytesPerSample = 1; break;
                case  8:        FirstColorByte = 0; BytesPerPixel = 1; SamplesPerPixel = 1; BytesPerSample = 1; break;
                default:        PrintError("Unsupported Pixel Size: %d", TrPtr->src->bitsPerPixel);
                                        TrPtr->success = 0;
                                        return;
        }
        // Jim W

        if( TrPtr->dest->selection.bottom == 0 && TrPtr->dest->selection.right == 0 ){
                destRect.left   = 0;
                destRect.right  = TrPtr->dest->width;
                destRect.top    = 0;
                destRect.bottom = TrPtr->dest->height;
        }else{
                memcpy( &destRect, &TrPtr->dest->selection, sizeof(PTRect) );
        }

        dest = *TrPtr->dest->data;
        src  = *TrPtr->src->data; // is locked


        if(TrPtr->mode & _show_progress){
                switch(color){
                        case 0: progressMessage = "Image Conversion";   break;
                        case 1: switch( TrPtr->src->dataformat)
                                        {
                                                case _RGB:      progressMessage = "Red Channel"  ; break;
                                                case _Lab:      progressMessage = "Lightness"    ; break;
                                        } break;
                        case 2: switch( TrPtr->src->dataformat)
                                        {
                                                case _RGB:      progressMessage = "Green Channel"; break;
                                                case _Lab:      progressMessage = "Color A"      ; break;
                                        } break; 
                        case 3: switch( TrPtr->src->dataformat)
                                        {
                                                case _RGB:      progressMessage = "Blue Channel"; break;
                                                case _Lab:      progressMessage = "Color B"     ; break;
                                        } break; 
                }
                Progress( _initProgress, progressMessage );
        }



        for(y=destRect.top; y<destRect.bottom; y++){
                // Update Progress report and check for cancel every 2%
                skip++;
                if( skip == (int)ceil(TrPtr->dest->height/50.0) )
                {
                        if(TrPtr->mode & _show_progress)
                        {       
                                sprintf( percent, "%d", (int) ((y * 100)/ TrPtr->dest->height));
                                if( ! Progress( _setProgress, percent ) )
                                {
                                        //myfree( (void**)TrPtr->dest->data );
                                        TrPtr->success = 0;
                                        return;
                                }
                        }
                        else
                        {
                                
                                if( ! Progress( _idleProgress, 0) )
                                {
                                        //myfree( (void**)TrPtr->dest->data );
                                        TrPtr->success = 0;
                                        return;
                                }
                        }
                        skip = 0;
                        
                }
                for(x=destRect.left; x<destRect.right; x++)
                {
                        xs = x; ys = y;

                        // Is the pixel valid, i.e. from within source image?

                        if( (xs >= TrPtr->src->width)   || (ys >= TrPtr->src->height) )
                                valid = FALSE;
                        else
                                valid = TRUE;

                        // if alpha channel marks valid portions, set valid 
                        if(1 == BytesPerSample)//8bit
                        {
                                if( (TrPtr->mode & _honor_valid) && 
                                        (SamplesPerPixel == 4)           &&
                                        (src[ ys * TrPtr->src->bytesPerLine + BytesPerPixel * xs] == 0))
                                        valid = FALSE;          
                        }
                        else //16bit
                        {
                                if( (TrPtr->mode & _honor_valid) && 
                                        (SamplesPerPixel == 4)           &&
                                        (*(unsigned short*)&src[ ys * TrPtr->src->bytesPerLine + BytesPerPixel * xs] == 0))
                                        valid = FALSE;          
                        }


                        // Check for and handle edge locations

                        if( xs < 0)                     xs = 0; 
                        if( xs >= TrPtr->src->width )   xs = TrPtr->src->width -1;
                        if( ys < 0)                     ys = 0;
                        if( ys >= TrPtr->src->height )  ys = TrPtr->src->height -1;


                        // Calculate pixel coefficient in dest image just once

                        coeff = (y-destRect.top) * TrPtr->dest->bytesPerLine  + BytesPerPixel * (x-destRect.left);              

                        if( valid )
                        {
                                if( color == 0 ) // Convert all color channels equally 
                                {
                                        for( col = FirstColorByte; col < BytesPerPixel; col += BytesPerSample)
                                        {
                        //Kekus 16 bit 2003/Nov/18
                            if( BytesPerSample == 2 )
                                                                *(unsigned short*)&dest[ coeff + col] = (unsigned short) func16( *(unsigned short*)&src[ ys * TrPtr->src->bytesPerLine + BytesPerPixel * xs + col], x-w2, y-h2, params );
                            else
                                                                dest[ coeff + col] = func( src[ ys * TrPtr->src->bytesPerLine + BytesPerPixel * xs + col ], x-w2, y-h2, params );
                        //Kekus.     
                                        }
                                        if( SamplesPerPixel == 4 )
                                        {
                                                if( BytesPerSample == 2 ) //16bit
                                                        *(unsigned short*)&dest[ coeff ] = *(unsigned short*)&src[ ys * TrPtr->src->bytesPerLine + BytesPerPixel * xs + col]; // Set alpha channel
                                                else
                                                        dest[ coeff ] = src[ ys * TrPtr->src->bytesPerLine + BytesPerPixel * xs + col ]; // Set alpha channel
                                        }
                                }
                                else // Convert just one channel
                                {
                                        col = (FirstColorByte ? color : color-1); 
                                        
                    //Kekus: 2003/Nov/18 
                        if( BytesPerSample == 2 )
                            *(unsigned short*)&dest[ coeff + col*2] = (unsigned short) func16( *(unsigned short*)&src[ ys * TrPtr->src->bytesPerLine + BytesPerPixel * xs + col*2 ], x-w2, y-h2, params );
                        else
                                                        dest[ coeff + col] = func( src[ ys * TrPtr->src->bytesPerLine + BytesPerPixel * xs + col ], x-w2, y-h2, params );
                    //Kekus.     
                                        if( SamplesPerPixel == 4 )
                                        {
                                                if( BytesPerSample == 2 ) //16bit
                                                        *(unsigned short*)&dest[ coeff ] = *(unsigned short*)&src[ ys * TrPtr->src->bytesPerLine + BytesPerPixel * xs ]; // Set alpha channel
                                                else
                                                        dest[ coeff ] = src[ ys * TrPtr->src->bytesPerLine + BytesPerPixel * xs]; // Set alpha channel
                                        }
                                }
                        }
                        else
                        {
                                for(i = 0; i < BytesPerPixel; i++)
                                        dest[ coeff + i] = 0;
                        }

                }
        }

        if(TrPtr->mode & _show_progress)
        {
                Progress( _disposeProgress, percent );
        }       

        TrPtr->success = 1;
}



// Given source image and dest dimensions, set dest and
// allocate memory for dest->data

int SetDestImage( TrformStr *TrPtr, int width, int height) 
{
        int result = 0;

        if( TrPtr->mode & _destSupplied )
                return 0;
                
        memcpy( TrPtr->dest, TrPtr->src, sizeof( Image ));
        
        TrPtr->dest->width                      = width;
        TrPtr->dest->height                     = height;

        // bytesPerLine depending on image format

        TrPtr->dest->bytesPerLine       =       TrPtr->dest->width * (TrPtr->dest->bitsPerPixel / 8) ; 

        TrPtr->dest->dataSize           = TrPtr->dest->height * TrPtr->dest->bytesPerLine;
        TrPtr->dest->data                       = (unsigned char**) mymalloc ((size_t)TrPtr->dest->dataSize);
        
        if( TrPtr->dest->data           == NULL )
                result = -1;
        
        return result;
}


// Copy image data from src to dest with framing/cropping if sizes differ
// src and dest may differ in bytesPerPixel (3 and/or 4)

void CopyImageData( Image *dest, Image *src )
{
        register unsigned char  *in, *out;
        register int                    x,y, dx, dy, id, is, i;
        int                                             bpp_s, bpp_d;
        
        in              = *(src->data);
        out     = *(dest->data);
        dx              = (src->width  - dest->width)  / 2;
        dy              = (src->height - dest->height) / 2;
        bpp_s   = src->bitsPerPixel  / 8;
        bpp_d   = dest->bitsPerPixel / 8;
        
        for( y = 0; y < dest->height; y++)
        {
                for( x = 0; x < dest->width; x++)
                {
                        is = (y + dy) * src->bytesPerLine  + bpp_s * (x + dx);
                        id = y        * dest->bytesPerLine + bpp_d * x;

                        if( y + dy < 0 || y + dy >= src->height ||
                                x + dx < 0 || x + dx >= src->width )    // outside src; set dest = 0
                        {
                                i = bpp_d;
                                while( i-- > 0 ) out[ id++ ] = 0;
                        }
                        else                                                                            // inside src; set dest = src
                        {
                                switch( bpp_d )
                                {
                                        case 8: switch( bpp_s )
                                                        {
                                                                case 8: memcpy( out + id, in + is, 8 ); 
                                                                                break;  
                                                                case 6: out[id++] = 255U; out[id++] = 255U;
                                                                                memcpy( out + id, in + is, 6 ); 
                                                                                break;
                                                                case 4: out[id++] = in[ is++ ]; out[id++] = 0;
                                                                                out[id++] = in[ is++ ]; out[id++] = 0;
                                                                                out[id++] = in[ is++ ]; out[id++] = 0;
                                                                                out[id++] = in[ is++ ]; out[id++] = 0;
                                                                                break;
                                                                case 3: out[id++] = 255U; out[id++] = 255U;
                                                                                out[id++] = in[ is++ ]; out[id++] = 0;
                                                                                out[id++] = in[ is++ ]; out[id++] = 0;
                                                                                out[id++] = in[ is++ ]; out[id++] = 0;
                                                                                break;
                                                        }
                                                        break;
                                        case 6:  switch( bpp_s )
                                                        {
                                                                case 8: is += 2;
                                                                                memcpy( out + id, in + is, 6 ); 
                                                                                break;  
                                                                case 6: memcpy( out + id, in + is, 6 ); 
                                                                                break;
                                                                case 4: is++;
                                                                                out[id++] = in[ is++ ]; out[id++] = 0;
                                                                                out[id++] = in[ is++ ]; out[id++] = 0;
                                                                                out[id++] = in[ is++ ]; out[id++] = 0;
                                                                                break;
                                                                case 3: out[id++] = in[ is++ ]; out[id++] = 0;
                                                                                out[id++] = in[ is++ ]; out[id++] = 0;
                                                                                out[id++] = in[ is++ ]; out[id++] = 0;
                                                                                break;
                                                        }
                                                        break;

                                        case 4:  switch( bpp_s )
                                                        {
                                                                case 8: out[id++] = in[ is++ ]; is++;
                                                                                out[id++] = in[ is++ ]; is++;
                                                                                out[id++] = in[ is++ ]; is++;
                                                                                out[id++] = in[ is++ ]; is++;
                                                                                break;  
                                                                case 6: out[id++] = 255U;
                                                                                out[id++] = in[ is++ ]; is++;
                                                                                out[id++] = in[ is++ ]; is++;
                                                                                out[id++] = in[ is++ ]; is++;
                                                                                break;
                                                                case 4: memcpy( out + id, in + is, 4 ); 
                                                                                break;
                                                                case 3: out[id++] = 255U;
                                                                                memcpy( out + id, in + is, 3 ); 
                                                                                break;
                                                        }
                                                        break;

                                        case 3:  switch( bpp_s )
                                                        {
                                                                case 8: is+=2;
                                                                                out[id++] = in[ is++ ]; is++;
                                                                                out[id++] = in[ is++ ]; is++;
                                                                                out[id++] = in[ is++ ]; is++; 
                                                                                break;  
                                                                case 6: out[id++] = in[ is++ ]; is++;
                                                                                out[id++] = in[ is++ ]; is++;
                                                                                out[id++] = in[ is++ ]; is++; 
                                                                                break;
                                                                case 4: is++;
                                                                                memcpy( out + id, in + is, 3 ); 
                                                                                break;
                                                                case 3: memcpy( out + id, in + is, 3 ); 
                                                                                break;
                                                        }
                                                        break;

                                }
                        }
                }
        }
}

// expand image from 3 to 4 bytes per pixel. No pad bytes allowed.
// Memory must be allocated
void ThreeToFourBPP( Image *im ){
        register int x,y,c1,c2;

        if( im->bitsPerPixel == 32 || im->bitsPerPixel == 64 || im->bitsPerPixel == 128) // Nothing to do
                return;

        if( im->bitsPerPixel == 24 ){   // Convert to 4byte / pixel
                for( y = im->height-1; y>=0; y--){
                        for( x= im->width-1; x>=0; x--){
                                c1 = (y * im->width + x) * 4;
                                c2 = y * im->bytesPerLine + x * 3;
                                (*(im->data))[c1++] = UCHAR_MAX;
                                (*(im->data))[c1++] = (*(im->data))[c2++];
                                (*(im->data))[c1++] = (*(im->data))[c2++];
                                (*(im->data))[c1++] = (*(im->data))[c2++];
                        }
                }
                im->bitsPerPixel = 32;
                im->bytesPerLine = im->width * 4;
        }else if( im->bitsPerPixel == 48 ){ // Convert to 8byte / pixel
                for( y = im->height-1; y>=0; y--){
                        for( x= im->width-1; x>=0; x--){
                                c1 = (y * im->width + x) * 4;
                                c2 = y * im->bytesPerLine/2 + x * 3;
                                ((uint16_t*)(*(im->data)))[c1++] = USHRT_MAX;
                                ((uint16_t*)(*(im->data)))[c1++] = ((uint16_t*)(*(im->data)))[c2++];
                                ((uint16_t*)(*(im->data)))[c1++] = ((uint16_t*)(*(im->data)))[c2++];
                                ((uint16_t*)(*(im->data)))[c1++] = ((uint16_t*)(*(im->data)))[c2++];
                        }
                }
                im->bitsPerPixel = 64;
                im->bytesPerLine = im->width * 8;
        }
        else if( im->bitsPerPixel == 96 ){ // Convert to 16byte / pixel
                for( y = im->height-1; y>=0; y--){
                        for( x= im->width-1; x>=0; x--){
                                c1 = (y * im->width + x) * 4;
                                c2 = y * im->bytesPerLine/4 + x * 3;
                                ((float*)(*(im->data)))[c1++] = 1.0;
                                ((float*)(*(im->data)))[c1++] = ((float*)(*(im->data)))[c2++];
                                ((float*)(*(im->data)))[c1++] = ((float*)(*(im->data)))[c2++];
                                ((float*)(*(im->data)))[c1++] = ((float*)(*(im->data)))[c2++];
                        }
                }
                im->bitsPerPixel = 128;
                im->bytesPerLine = im->width * 16;
        }
        im->dataSize = im->height * im->bytesPerLine;
}

// eliminate alpha channel.
// pad bytes allowed
void    FourToThreeBPP          ( Image *im )
{
        register int x,y,c1,c2;
        
        if( im->bitsPerPixel == 24 || im->bitsPerPixel == 48 || im->bitsPerPixel == 96) // Nothing to do
                return;

        
        if( im->bitsPerPixel == 32 ) // Convert to 3byte / pixel
        {
                register unsigned char *data = *(im->data);
                for( y = 0; y < im->height; y++)
                {
                        for( x=0; x < im->width; x++)
                        {
                                c1 =  y * im->bytesPerLine + x * 4;
                                c2 = (y * im->width + x) * 3;
                                c1++;
                                data [c2++] = data [c1++];
                                data [c2++] = data [c1++];
                                data [c2++] = data [c1++];
                        }
                }
                im->bitsPerPixel = 24;
                im->bytesPerLine = im->width * 3;
        }
        else if( im->bitsPerPixel == 64 )// Convert to 6byte / pixel
        {
                register uint16_t *data = (uint16_t*)*(im->data);
                for( y = 0; y < im->height; y++)
                {
                        for( x=0; x < im->width; x++)
                        {
                                c1 =  y * im->bytesPerLine/2 + x * 4;
                                c2 = (y * im->width + x) * 3;
                                c1++;
                                data [c2++] = data [c1++];
                                data [c2++] = data [c1++];
                                data [c2++] = data [c1++];
                        }
                }
                im->bitsPerPixel = 48;
                im->bytesPerLine = im->width * 6;
        }
        else if( im->bitsPerPixel == 128 )// Convert to 12byte / pixel
        {
                register float *data = (float*)*(im->data);
                for( y = 0; y < im->height; y++)
                {
                        for( x=0; x < im->width; x++)
                        {
                                c1 =  y * im->bytesPerLine/4 + x * 4;
                                c2 = (y * im->width + x) * 3;
                                c1++;
                                data [c2++] = data [c1++];
                                data [c2++] = data [c1++];
                                data [c2++] = data [c1++];
                        }
                }
                im->bitsPerPixel = 96;
                im->bytesPerLine = im->width * 12;
        }
        
        im->dataSize = im->height * im->bytesPerLine;
}


void OneToTwoByte( Image *im )
{
        register int x,y,c1,c2,i;
        int bpp;
        
        if( im->bitsPerPixel > 32 ) return;

        bpp = im->bitsPerPixel / 8;

        for( y = im->height-1; y>=0; y--)
        {
                for( x= im->width-1; x>=0; x--)
                {
                        c1 = ( y * im->width + x) * bpp * 2;
                        c2 = y * im->bytesPerLine + x * bpp;
                        
                        for(i=0; i<bpp; i++)
                        {
                                *((uint16_t*)(*im->data + c1)) = ((uint16_t)(*(im->data))[c2++]) << 8; c1 += 2;
                        }
                }
        }
        im->bitsPerPixel        *= 2;
        im->bytesPerLine        = im->width * im->bitsPerPixel/8;
        im->dataSize            = im->height * im->bytesPerLine;
}

void TwoToOneByte( Image *im ){
        register int x,y,c1,c2,i;
        int bpp_old, bpp_new;
        
        if( im->bitsPerPixel < 48 ) return;
        
        bpp_old = im->bitsPerPixel / 8;
        bpp_new = bpp_old / 2;

        for( y = 0; y < im->height; y++){
                for( x=0; x < im->width; x++){
                        c1 = (y * im->width + x) * bpp_new;
                        c2 = y * im->bytesPerLine + x * bpp_old;
                        
                        for(i=0; i<bpp_new; i++){
                                (*(im->data))[c1++] = *((uint16_t*)(*im->data + c2)) >> 8; c2 += 2;
                        }
                }
        }
        im->bitsPerPixel        /= 2;
        im->bytesPerLine        = im->width * im->bitsPerPixel/8;
        im->dataSize            = im->height * im->bytesPerLine;
}




static void panoSetMetadataDefaults(pano_ImageMetadata *m)
{

    bzero(m, sizeof(*m));

    // These are "meaningful defaults 

    m->xPixelsPerResolution = PANO_DEFAULT_PIXELS_PER_RESOLUTION;
    m->yPixelsPerResolution = PANO_DEFAULT_PIXELS_PER_RESOLUTION;
    m->resolutionUnits = PANO_DEFAULT_TIFF_RESOLUTION_UNITS;


    m->rowsPerStrip =1; // THis will speed up processing of TIFFs as only one line
                        // at a time needs to be read

    m->compression.type = PANO_DEFAULT_TIFF_COMPRESSION; 

}


void SetImageDefaults(Image *im){
        im->data                = NULL;
        im->bytesPerLine        = 0;
        im->width               = 0;
        im->height              = 0;
        im->dataSize            = 0;
        im->bitsPerPixel        = 0;
        im->format              = 0;
	im->formatParamCount    = 0;
	bzero(im->formatParam, sizeof(im->formatParam));
	im->precomputedCount    = 0;
	bzero(im->precomputedValue, sizeof(im->precomputedValue));
        im->dataformat          = _RGB;
        im->hfov                = 0.0;
        im->yaw                 = 0.0;
        im->pitch               = 0.0;
        im->roll                = 0.0;
        SetCorrectDefaults( &(im->cP) );
        *(im->name)             = 0;
        im->selection.top       = 0;
        im->selection.bottom    = 0;
        im->selection.left      = 0;
        im->selection.right     = 0;
        im->cropInformation.cropped_height  = 0;
        im->cropInformation.cropped_width   = 0;
        im->cropInformation.full_height     = 0;
        im->cropInformation.full_width      = 0;
        im->cropInformation.x_offset        = 0;
        im->cropInformation.y_offset        = 0;
        panoSetMetadataDefaults(&im->metadata);
}

// Copy all position  related data
void CopyPosition( Image *to, Image *from ){
        to->format              = from->format;
        to->hfov                = from->hfov;
        to->yaw                 = from->yaw;
        to->pitch               = from->pitch;
        to->roll                = from->roll;
        memcpy(&(to->cP), &(from->cP), sizeof( cPrefs ) );
}

#ifndef abs
#define abs(a) ( (a) >= 0 ? (a) : -(a) )
#endif
#define EPSILON 1.0e-8

// Compare position data of two image
// return 0 if equal
// return +1 if only yaw differs
// return +2 if more differs
int PositionCmp( Image *im1, Image *im2 )
{
        if( abs(im1->format - im2->format       ) < EPSILON     &&
                abs(im1->hfov   - im2->hfov             ) < EPSILON             &&
                abs(im1->pitch  - im2->pitch    ) < EPSILON             &&
                abs(im1->roll   - im2->roll             ) < EPSILON             &&
                EqualCPrefs( &im1->cP, &im2->cP ) )
        {
                if( im1->yaw    == im2->yaw     )
                        return 0;
                else
                        return 1;
        }
        else
        {
                return 2;
        }
}

// Compare optimizable cprefs-parameters
int EqualCPrefs( cPrefs *c1, cPrefs *c2 )
{
        if( abs(c1->radial_params[0][0] - c2->radial_params[0][0]       ) < EPSILON  &&
                abs(c1->radial_params[0][1] - c2->radial_params[0][1]   ) < EPSILON  &&
                abs(c1->radial_params[0][2] - c2->radial_params[0][2]   ) < EPSILON  &&
                abs(c1->radial_params[0][3] - c2->radial_params[0][3]   ) < EPSILON  &&
                abs(c1->vertical_params[0]      - c2->vertical_params[0]        ) < EPSILON      &&
                abs(c1->horizontal_params[0]- c2->horizontal_params[0]  ) < EPSILON )
                return TRUE;
        else
                return FALSE;
}

        


// Do these images have equal pixel sizes

int HaveEqualSize( Image *im1, Image *im2 )
{

        if( (im1->bytesPerLine != im2->bytesPerLine) ||
                (im1->width        != im2->width)                ||
                (im1->height       != im2->height)       ||
                (im1->dataSize     != im2->dataSize)     ||
                (im1->bitsPerPixel != im2->bitsPerPixel) )
                return FALSE;
        else
                return TRUE;
}



void SetSizeDefaults( sPrefs *pref)
{
    pref->magic             = 70;                   // File validity check; must be 70
    pref->displayPart       = TRUE;                 // Display cropped/framed image ?
    pref->saveFile          = FALSE;                // Save to tempfile?
    pref->launchApp         = FALSE;                // Open sFile ?
    pref->interpolator      = _spline36;            //
    makePathForResult( &(pref->sFile) );
    makePathToHost ( &(pref->lApp) );
    pref->gamma             = 1.0;
    pref->noAlpha           = FALSE;                // Check only for Photoshop LE
    pref->optCreatePano     = TRUE;
    pref->fastStep          = FAST_TRANSFORM_STEP_NONE; // the value will be changed in parser.c
}





void    SetVRPanoOptionsDefaults( VRPanoOptions *v)
{
        v->width        =       400;
        v->height       =       300;
        v->pan          =       0.0;
        v->tilt         =       0.0;
        v->fov          =       45.0;
        v->codec        =       0; 
        v->cquality     =       80;
        v->progressive = FALSE;
}

                        

// Crop Image to selection rectangle    
int CropImage(Image *im, PTRect *r){

        uint32_t x,y,i;
        unsigned char *src, *dst, **data = NULL;
        uint32_t width = r->right - r->left;
        uint32_t height = r->bottom - r->top;
        int bytesPerPixel = im->bitsPerPixel / 8 ;
        int bytesPerLine = width * im->bitsPerPixel / 8 ;
        size_t dataSize = bytesPerLine * height;

        // Some checks first
        if( r->left < 0 || r->left >im->width ||
            r->right < 0|| r->right > im->width ||
            r->left >= r->right||
            r->top < 0 || r->top > im->height||
            r->bottom < 0 || r->bottom > im->height ||
            r->top >= r->bottom )
                return -1;


        data = (unsigned char**) mymalloc( dataSize );
        if( data == NULL ) return -1;
        
        for(y=0; y<height; y++){
                for(x=0, 
                    src=*im->data + (y+r->top)*im->bytesPerLine +r->left*bytesPerPixel, 
                    dst = *data + y*bytesPerLine; 
                    x<width; x++ ){
                        i = 0;
                        while( i++ < bytesPerPixel ) *dst++ = *src++;
                    }
        }
        myfree((void**)im->data);
        
        im->data = data;
        im->width = width;
        im->height = height;
        im->bytesPerLine = bytesPerLine;
        im->dataSize = dataSize;
        
        return 0;
}
                
                
// Are we inside the ROI
int panoROIRowInside(pano_CropInfo * cropInfo, int row)
{
    // We are in the ROI if the row is bigger than the yoffset
    // and the row is less or equal to the offset + height
    assert(cropInfo != NULL);
    assert(row >= 0);

    return
        row >= cropInfo->yOffset &&
        row < cropInfo->yOffset + cropInfo->croppedHeight;

}

        



void panoMetadataFree(pano_ImageMetadata * metadata)
{
    // Free any parts of metadata that are dynamically allocated

    if (metadata->iccProfile.size != 0) {
        assert(metadata->iccProfile.data != NULL);
        //      printf("+++++++++++++++++++++++++++++++++++++Freeing ICC profile %x\n",
        //             (int)(metadata->iccProfile.data));

        free(metadata->iccProfile.data);
        metadata->iccProfile.data = NULL;
        metadata->iccProfile.size = 0;
    }

    if (metadata->copyright!=NULL)  {
        free(metadata->copyright);
	metadata->copyright = NULL;
    }
    

    if (metadata->artist!=NULL) {
        free(metadata->artist);
	metadata->artist = NULL;
    }

    if (metadata->datetime!=NULL)  {
	free(metadata->datetime);
	metadata->datetime = NULL;
    }

    if (metadata->imageDescription!=NULL)  {
        free(metadata->imageDescription);
        metadata->imageDescription = NULL;
    }
    
}

// Allocate a block of memory, copy it and return it
int panoAllocAndCopy(char **pTo, char *from, int size)
{
    char *temp;
    // Make it easy for the caller

    if (size == 0 ||
        from == NULL) 
    {
        *pTo = NULL;
        return TRUE;
    }

    temp = calloc(size, 1);
    if (temp == NULL) 
    {
        PrintError("Not enough memory");
        return FALSE;
    }   
    memcpy(temp, from, size);

    *pTo = temp;
    return TRUE;
}

// Allocate a block of memory, copy it and return it
int panoAllocAndCopyString(char **pTo, char *from)
{
    if (from == NULL) 
    {
        *pTo = NULL;
        return TRUE;
    }

    return panoAllocAndCopy(pTo, from, strlen(from)+1);
}


int panoMetadataCopy(pano_ImageMetadata * to, pano_ImageMetadata * from)
{
    /* 
       Copy the metadata, allocate memory as needed
     */
    int result;
    char *temp;

    assert(from != NULL);
    assert(to != NULL);

    // clear the destination
    bzero(to, sizeof(*to));
    // most of the data can be copied this way
    memcpy(to, from, sizeof(*to));

    // Allocate memory for dynamic areas

    //these fields can't be mem copied 
    to->iccProfile.data = NULL;
    to->copyright = NULL;
    to->datetime = NULL;
    to->imageDescription = NULL;
    to->artist = NULL;

    //    result = panoAllocAndCopy(&(to->iccProfile.data),
    result = panoAllocAndCopy(&temp,
                              from->iccProfile.data,
                              (int)from->iccProfile.size);

    to->iccProfile.data  = temp;

    //panoDumpMetadata(to, "----------------->Copy");

#define pano_COPY_STRING(a) (panoAllocAndCopyString(&(to->a), from->a))

    result = result && 
        pano_COPY_STRING(copyright) &&
        pano_COPY_STRING(datetime) &&
        pano_COPY_STRING(imageDescription) &&
        pano_COPY_STRING(artist);

#undef pano_COPY_STRING

    //panoDumpMetadata(to, "----------------->Copy after");

    //printf("End of copy result %d\n", result);

    return result;
}



void panoMetadataSetCompression(pano_ImageMetadata * metadata, char *compressionName)
{
    //Packbits compression was used by original PTStitcher and is retained
    //as the default...the option to use the more efficient LZW compression
    //is also provided
    if (strstr(compressionName, "c:LZW") != NULL)
    {
        metadata->compression.type = COMPRESSION_LZW;
        metadata->compression.predictor = 2;
    }
    else if (strstr(compressionName, "c:NONE") != NULL) 
    {
        metadata->compression.type = COMPRESSION_NONE;
    }
    else if (strstr(compressionName, "c:DEFLATE") != NULL)
    {
        metadata->compression.type = COMPRESSION_DEFLATE;
    }
    else 
    {                  // Default is PACKBITS
        // TODO
        // dmg: I am not very happy with this type of compression
        // DEFLATE is better
        metadata->compression.type = COMPRESSION_PACKBITS;
    }
}

// We need sometimes to reuse metadata but change its size.
// This function does that
void panoMetadataResetSize(pano_ImageMetadata * metadata, 
                           int width, int height)
{
    metadata->imageWidth = width;
    metadata->imageHeight = height;
    metadata->bytesPerLine = metadata->imageWidth * metadata->bytesPerPixel;

    metadata->isCropped = FALSE;
    
}

void panoMetadataSetAsCropped(pano_ImageMetadata * metadata, 
                              int croppedWidth, 
                              int croppedHeight,
                              int roiLeft, 
                              int roiTop)
{
    // Set crop structure

    metadata->cropInfo.fullWidth = metadata->imageWidth;
    metadata->cropInfo.fullHeight = metadata->imageHeight;

    metadata->cropInfo.xOffset = roiLeft;
    metadata->cropInfo.yOffset = roiTop;

    metadata->cropInfo.croppedWidth = croppedWidth;
    metadata->cropInfo.croppedHeight = croppedHeight;

    // Set main metadata fields

    metadata->imageWidth = croppedWidth;
    metadata->imageHeight = croppedHeight;
    metadata->bytesPerLine = metadata->imageWidth * metadata->bytesPerPixel;

    // And of course the most important
    metadata->isCropped = TRUE;


}

#ifdef panoDumpMetadata
#undef panoDumpMetadata
#endif

void panoDumpMetadata(pano_ImageMetadata * metadata, char *message)
{
    printf("**Metadata***%s\n", message);
    printf("  Size %dx%d ",     metadata->imageWidth,     metadata->imageHeight);
    printf("  is cropped %d\n", metadata->isCropped);
    if (metadata->isCropped) 
    {
        printf("  Cropped size %dx%d offset %d,%d Full size %dx%d\n",
               (int)metadata->cropInfo.croppedWidth,
               (int)metadata->cropInfo.croppedHeight,
               (int)metadata->cropInfo.xOffset,
               (int)metadata->cropInfo.yOffset,
               (int)metadata->cropInfo.fullWidth,
               (int)metadata->cropInfo.fullHeight);
    }
    printf("  REsolution %f, %f units %d ",metadata->xPixelsPerResolution,
           metadata->yPixelsPerResolution, metadata->resolutionUnits);
    printf("  Samplesperpixel %d, bitsPerSample %d ",
           metadata->samplesPerPixel, metadata->bitsPerSample);

    printf("  bytesPerLine %d ", metadata->bytesPerLine);

    printf("  rows per strip %d ", metadata->rowsPerStrip);

    printf("  compression %d %d ", metadata->compression.type,
           metadata->compression.predictor);

    printf("  bytesPerPixel %d bitsPerPixel %d\n", metadata->bytesPerPixel,
           (int)metadata->bitsPerPixel);

    if (metadata->copyright != NULL)
        printf("Copyright [%s]\n", metadata->copyright);

    if (metadata->artist != NULL)
        printf("Artist [%s]\n", metadata->artist);

    if (metadata->datetime != NULL)
        printf("datetime [%s]\n", metadata->datetime);

    if (metadata->imageDescription != NULL)
        printf("Artist [%s]\n", metadata->imageDescription);

    printf("**EndMetadata***%s\n", message);
}

/* ENDIAN aware file i/o funtions.  Used for reading and writing photoshop files */
Boolean panoWriteUCHAR(file_spec fnum, uint8_t theChar )
{ 
    size_t count = 1;

    mywrite( fnum, count, &theChar );
    return count == 1;
}

Boolean panoWriteSHORT(file_spec fnum, uint16_t theShort )
{
    size_t count = 2;
    char data[2], *d;
    d = data;
    
    assert(sizeof(uint16_t) == 2);
    SHORTNUMBER( theShort, d );

    mywrite( fnum, count, data );
    return count == 2;
}

Boolean panoWriteINT32(file_spec fnum, uint32_t theLong )
{
    size_t count = 4;
    char data[4], *d;
    d = data;
    
    assert(sizeof(uint32_t) == 4);
    
    LONGNUMBER( theLong, d );

    mywrite( fnum, count, data );
    return count == 4;
}

Boolean panoWriteINT64(file_spec fnum, int64_t theLongLong )
{
    size_t count = 8;
    char data[8], *d;
    d = data;
    
    assert(sizeof(int64_t) == 8);
    
    LONGLONGNUMBER( theLongLong, d );

    mywrite( fnum, count, data );
    return count == 8;
}

Boolean panoWriteINT32or64(file_spec fnum, int64_t theLongLong, Boolean bBig )
{
    if(bBig)
        return panoWriteINT64(fnum, theLongLong);
    else
        return panoWriteINT32(fnum, (uint32_t)theLongLong);
}

Boolean panoReadUCHAR(file_spec fnum, uint8_t *pChar )
{
    size_t count = 1;

    myread( fnum, count, pChar );
    return (count== 1);
}

Boolean panoReadSHORT(file_spec fnum, uint16_t *pShort )
{
    size_t count = 2;
    char data[2];
    char *d;

    myread( fnum, count, data);
    if (count != 2) {
        return FALSE;
    }

    d = data;
    NUMBERSHORT( (*pShort), d );
    return TRUE;
}

Boolean panoReadINT32(file_spec fnum, uint32_t *pLong )
{
    size_t count = 4;
    char data[4], *d;

    myread( fnum, count, data);
    if (count != 4) {
        return FALSE;
    }
    d = data;
    NUMBERLONG( (*pLong), d );
    return TRUE;
}

Boolean panoReadINT64(file_spec fnum, int64_t  *pLongLong )
{
    size_t count = 8;
    char data[8], *d;

    myread( fnum, count, data);
    if (count != 8) {
        return FALSE;
    }
    d = data;
    NUMBERLONGLONG( (*pLongLong), d );
    return TRUE;
}

Boolean panoReadINT32or64(file_spec fnum, int64_t  *pLongLong, Boolean bBig )
{
    if(bBig)
        return panoReadINT64( fnum, pLongLong );
    else
    {
        uint32_t Long;
        Boolean bRtn = panoReadINT32( fnum, &Long );
        *pLongLong = (int64_t)Long;
        return bRtn;
    }
}
