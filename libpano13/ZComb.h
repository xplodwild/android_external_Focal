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

PANO13_IMPEX void ZCombSetDisabled(void);
PANO13_IMPEX void ZCombSetEnabled(void);
PANO13_IMPEX void ZCombSetMaskType(int mt);
PANO13_IMPEX void ZCombSetFocusWindowHalfwidth(int fwh);
PANO13_IMPEX void ZCombSetSmoothingWindowHalfwidth(int swh);
PANO13_IMPEX void ZCombSetMaskFromFocusData(Image *im);
PANO13_IMPEX void ZCombSetGreenTo255(Image *im);
PANO13_IMPEX int  ZCombSeeImage(Image *im, char *filename);
