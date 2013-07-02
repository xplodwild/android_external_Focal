/*
 *  jpegicc.h
 *
 *  This source file was originally part of the LCMS package. I asked
 *  Marti for permission to change its license from MIT to GPL (Aug 4,
 *  2006). Daniel German
 * 
 *  Copyright (C) 1998-2006 Tom Lane and Marti Maria
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
 *  Author: Tom Lane and Marti Maria
 * 
 *
 *
 * This file provides code to read and write International Color Consortium
 * (ICC) device profiles embedded in JFIF JPEG image files.  The ICC has
 * defined a standard format for including such data in JPEG "APP2" markers.
 * The code given here does not know anything about the internal structure
 * of the ICC profile data; it just knows how to put the profile data into
 * a JPEG file being written, or get it back out when reading.
 *
 * This code depends on new features added to the IJG JPEG library as of
 * IJG release 6b; it will not compile or work with older IJG versions.
 *
 * NOTE: this code would need surgery to work on 16-bit-int machines
 * with ICC profiles exceeding 64K bytes in size.  See iccprofile.c
 * for details.
 */

#ifndef __JPEGICC_H__

#define __JPEGICC_H__

#include <stdlib.h>
#include <jpeglib.h>

/*
 * This routine writes the given ICC profile data into a JPEG file.
 * It *must* be called AFTER calling jpeg_start_compress() and BEFORE
 * the first call to jpeg_write_scanlines().
 * (This ordering ensures that the APP2 marker(s) will appear after the
 * SOI and JFIF or Adobe markers, but before all else.)
 */

extern void jpegICCWriteProfile(j_compress_ptr cinfo,
                                 const JOCTET *icc_data_ptr,
                                 unsigned int icc_data_len);


/*
 * Reading a JPEG file that may contain an ICC profile requires two steps:
 *
 * 1. After jpeg_create_decompress() but before jpeg_read_header(),
 *    call setup_read_icc_profile().  This routine tells the IJG library
 *    to save in memory any APP2 markers it may find in the file.
 *
 * 2. After jpeg_read_header(), call read_icc_profile() to find out
 *    whether there was a profile and obtain it if so.
 */


/*
 * Prepare for reading an ICC profile
 */

extern void jpegICCSetupReadICCProfile(j_decompress_ptr cinfo);



/*
 * See if there was an ICC profile in the JPEG file being read;
 * if so, reassemble and return the profile data.
 *
 * TRUE is returned if an ICC profile was found, FALSE if not.
 * If TRUE is returned, *icc_data_ptr is set to point to the
 * returned data, and *icc_data_len is set to its length.
 *
 * IMPORTANT: the data at **icc_data_ptr has been allocated with malloc()
 * and must be freed by the caller with free() when the caller no longer
 * needs it.  (Alternatively, we could write this routine to use the
 * IJG library's memory allocator, so that the data would be freed implicitly
 * at jpeg_finish_decompress() time.  But it seems likely that many apps
 * will prefer to have the data stick around after decompression finishes.)
 */

extern boolean jpegICCReadProfile(j_decompress_ptr cinfo,
                                  JOCTET **icc_data_ptr,
                                  unsigned int *icc_data_len);

#endif
