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

// This file contains functions to handle multilayer files
// The 8bit versions are PS-compatible

#include "filter.h"

#if _MSC_VER > 1000
#pragma warning(disable: 4100) // disable unreferenced formal parameter warning
#endif

void	DisposeMultiLayerImage( MultiLayerImage *mim ){
	int i;
	
	if( mim->Layer != NULL ){
		for( i=0; i<mim->numLayers; i++){
			if( mim->Layer[i].data != NULL ){
				myfree((void**)mim->Layer[i].data);
			}
		}		
		free( mim->Layer );
	}
}

// obsolete dummies
double OverlapRMS( MultiLayerImage *mim PT_UNUSED){ return 0.0; }


