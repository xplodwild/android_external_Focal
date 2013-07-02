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
#ifndef PTVERSIONINFO
#define PTVERSIONINFO

#if defined(_IA64_) || defined(_AMD64_) || defined(_M_IA64) || defined(_M_AMD64) || defined(WIN64) || defined(_WIN64)
  #define PTVERSIONBIT "64 bit"
#else
  #define PTVERSIONBIT "32 bit"
#endif

#define VERS1 0x2
#define VERS2 0x00

//version of preferences file, used to verify data
#define PREF_VERSION "2.9.19 "

// String style of global version
#define VERSION "2.9.19 "

// Numeric style of global version, same as VERSION with more precision
#define PTVERSION_FILEVERSIONNUMBER 2,9,19,0
#define PTVERSION_NAME_LONG "LongVersion"
#define LONGVERSION VERSION ", Copyright (c) 1998-2006, H. Dersch, der@fh-furtwangen.de"

#define PTVERSION_FILEDESCRIPTION "Panorama Tools " PTVERSIONBIT " Library\0"

#define PTVERSION_NAME_FILEVERSION "FileVersion"
#define PTVERSION_FILEVERSION VERSION "\0"
#define PROGRESS_VERSION VERSION "\0"

#define PTVERSION_NAME_LEGALCOPYRIGHT "LegalCopyright"
#define PTVERSION_LEGALCOPYRIGHT "Copyright © 1999, 2000, 2001, 2005, 2006 Helmut Dersch\0"

#define PTVERSION_NAME_COMMENT "Comments"

#define PTVERSION_COMMENT "http://sourceforge.net/projects/panotools/"
#define PTVERSION_COMPNAME "http://wiki.panotools.org/"

#endif  //PTVERSIONINFO
