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

void 	pan	(TrformStr *TrPtr, panControls *pc )
{
	int	 		destwidth, destheight;
	aPrefs		aP;
	double		hfov;
	Image		*im, buf;


	if( readPrefs( (char*)&aP, _adjust ) != 0 )
	{
		PrintError("Could not read Preferences");
		TrPtr->success = 0;
		return;
	}

	switch( TrPtr->tool )
	{
		case _apply:			
			// Maybe the viewing window has been resized (Hopefully not more!)
			aP.im.width			= TrPtr->src->width;
			aP.im.height		= TrPtr->src->height;
			
			// pano must be the saved buffer image
			if( *aP.sBuf.destName == 0  || LoadBufImage( &aP.pano, aP.sBuf.destName, 0) != 0 )
			{
				PrintError("Could not load Buffer");
				TrPtr->success = 0;
				return;
			}
			
			im					= TrPtr->dest;
			TrPtr->dest 		= &aP.pano;
			TrPtr->dest->data 	= (unsigned char**) mymalloc( (size_t)TrPtr->dest->dataSize );
			if( TrPtr->dest->data == NULL )
			{
				PrintError( "Not enough memory to create Panorama");
				TrPtr->success = 0;
				return;
			}
			TrPtr->mode				|= _honor_valid;

			CopyPosition( TrPtr->src, &(aP.im) );
			addAlpha( TrPtr->src ); // Add alpha channel to indicate valid data
		
			MakePano( TrPtr,  &aP );

			// Stitch images; Proceed only if panoramic image valid
				
			if( TrPtr->success )
			{
				// Now load the whole bufferimage

				if(  LoadBufImage( &buf, aP.sBuf.destName, 1 ) != 0  )
				{
					PrintError( "Not enough Memory to merge Images" );
				}
				else 
				{
					if( merge( TrPtr->dest , &buf, aP.sBuf.feather, TrPtr->mode & _show_progress, _dest ) != 0 )
					{
						PrintError( "Error merging images" );
					}
					else
					{
						if( SaveBufImage( TrPtr->dest, aP.sBuf.destName ) != 0 )
							PrintError( "Could not save Buffer Image.");
					}
					myfree( (void**)buf.data );
				}

			} // Tr.success 
			
			TrPtr->success = 0;
			myfree( (void**)TrPtr->dest->data );
			TrPtr->dest = im;
			break;
		case _getPano:
			
			// Load buffer into dest
			// destSupplied is not allowed here!
			
			if( *aP.sBuf.destName == 0  || LoadBufImage( TrPtr->dest, aP.sBuf.destName, 1) != 0 )
			{
				PrintError("Could not load Buffer");
				TrPtr->success = 0;
			}
			else
				TrPtr->success = 1;
			break;
		case _increment:
			if( SetPanPrefs( pc ) )
				writePrefs( (char*)pc, _panleft );
			TrPtr->success = 0; // don't destroy source!
			break;
		default:
			switch( TrPtr->tool )
			{
				case _panright:
					aP.im.yaw += pc->panAngle;
					NORM_ANGLE( aP.im.yaw );
					break;
				case _panleft:
					aP.im.yaw -= pc->panAngle;
					NORM_ANGLE( aP.im.yaw );
					break;
				case _panup:
					aP.im.pitch += pc->panAngle;
					NORM_ANGLE( aP.im.pitch );
					break;
				case _pandown:
					aP.im.pitch -= pc->panAngle;
					NORM_ANGLE( aP.im.pitch );
					break;
				case _zoomin:
					hfov = aP.im.hfov / ((100.0 + pc->zoomFactor)/100.0);
					if( aP.im.format != _rectilinear || hfov < 180.0 )
						aP.im.hfov = hfov;
					break;
				case _zoomout:
					hfov = aP.im.hfov * ((100.0 + pc->zoomFactor)/100.0);
					if( aP.im.format != _rectilinear || hfov < 180.0 )
						aP.im.hfov = hfov;
					break;
				default: break;
			}
			// Maybe the viewing window has been resized (Hopefully not more!)
			aP.im.width			= TrPtr->src->width;
			aP.im.height		= TrPtr->src->height;


			destheight 				= aP.im.height;
			destwidth 				= aP.im.width;
			
			if( SetDestImage( TrPtr, destwidth, destheight) != 0)
			{
				PrintError("Could not allocate %ld bytes",TrPtr->dest->dataSize );
				TrPtr->success = 0;
				return;
			}
			TrPtr->mode				|= _honor_valid;

			if( LoadBufImage( &aP.pano, aP.sBuf.destName, 1) != 0 )
			{
				PrintError("Could not load Buffer" );
				TrPtr->success = 0;
				return;
			}
						
			im = TrPtr->src;
			TrPtr->src = &aP.pano;

			if( aP.pano.hfov == 360.0 )
				TrPtr->mode				|= _wrapX;
			
			ExtractStill( TrPtr , &aP );

			myfree( (void**)TrPtr->src->data );
			TrPtr->src = im;
			
			if(TrPtr->success)
				writePrefs(  (char*)&aP, _adjust );

			if( TrPtr->success == 0 && ! (TrPtr->mode & _destSupplied))
				myfree( (void**)TrPtr->dest->data );
			break;
		
	}
}



