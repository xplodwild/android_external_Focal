/*
 *  PTuncrop
 *
 *  This program takes as input a cropped TIFF and generates an uncropped TIFF
 *
 *  May 2005
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


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "PTcommon.h"

#define DEFAULT_PREFIX    "uncropped"

#define PT_UNCROP_USAGE "PTuncrop [options] <inputFile> <outputFile>\n\n"\
    "Options:\n"                                                        \
    "\t-p <prefix>\tPrefix for output files (defaults to " DEFAULT_PREFIX "%%4d)\n"	\
    "-f\t\tForce processing: Overwrite output file if it exists\n"      \
    "\t-x\t\tDelete source files (use with care)\n"\
    "\t-q\t\tQuiet run\n"\
    "\t-h\t\tShow this message\n"\
    "\n"

#define PT_UNCROP_VERSION "PTuncrop Version " VERSION ", by Daniel M German\n"


int main(int argc, char *argv[])
{
    return panoCroppingMain(argc, argv, PANO_CROPPING_UNCROP, PT_UNCROP_VERSION, PT_UNCROP_USAGE, DEFAULT_PREFIX);
}

