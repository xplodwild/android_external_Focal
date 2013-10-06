/*
  multiblend (c) 2012 David Horman

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.

  The author can be contacted at: david.horman@jerseymail.co.uk

  Discussion at http://tawbaware.com/forum2/viewtopic.php?f=3&t=6396
             or http://groups.google.com/group/hugin-ptx/
*/

#include <algorithm>
using namespace std;
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __APPLE__
  #define memalign(a,b) malloc((b))
#else
  #include <malloc.h>
#endif

#include "simd_soft.cpp"

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#endif

#include <stdarg.h>

extern "C" {
#include <jpeglib.h>
}

#include <png.h>
#include <tiffio.h>
#include "globals.cpp"
#include "functions.cpp"
#include "geotiff.cpp"
#include "loadimages.cpp"
#include "seaming.cpp"
#include "maskpyramids.cpp"
#include "blending.cpp"
#include "write.cpp"
#include "pseudowrap.cpp"
#include "go.cpp"

#ifdef WIN32
#pragma comment(lib,"libtiff.lib")
#pragma comment(lib,"turbojpeg-static.lib")
#pragma comment(lib,"libpng.lib")
#pragma comment(lib,"zlib.lib")
#endif

static void help() {
	output(0, "Usage: multiblend [options] [-o OUTPUT] INPUT...\n");
	output(0, "Options:\n");
	output(0, "   -l X                  X > 0: limit number of blending levels to x\n");
	output(0, "                         X < 0: reduce number of blending levels by -x\n");
	output(0, "   -d DEPTH              override automatic output image depth (8 or 16)\n");
	output(0, "  --nocrop               do not crop output\n");
	output(0, "  --bgr                  swap RGB order\n");
	output(0, "  --wideblend            calculate number of levels based on output image size,\n");
	output(0, "                         rather than input image size\n");
	output(0, "  --compression=X        output file compression. For TIFF output, X may be:\n");
	output(0, "                         NONE (default), PACKBITS, or LZW\n");
	output(0, "                         For JPEG output, X is JPEG quality (0-100, default 75)\n");
  output(0, "  --save-seams <file>    Save seams to PNG file for external editing\n");
  output(0, "  --no-output            Don't perform blend (for use with --save-seams)\n");
  output(0, "  --load-seams <file>    Load seams from PNG file\n");
	output(0, "  --bigtiff              BigTIFF output (not well tested)\n");
	output(0, "  --reverse              reverse image priority (last=highest)\n");
	output(0, "  --quiet                suppress output (except warnings)\n");
  output(0, "\n");
  output(0, "Pass a single image as input to blend around the left/right boundary.\n");
	exit(0);
}

int main(int argc, char* argv[]) {
  int i;
  int input_args;
	int temp;
	my_timer timer_all;

	if(setjmp(jmpbuf))
	{
		return 1;
	}

	timer_all.set();
	TIFFSetWarningHandler(NULL);

  if (argc==1 || !strcmp(argv[1],"-h") || !strcmp(argv[1],"--help") || !strcmp(argv[1],"/?")) return 2;

  output(0, "\n");

  if (argc<3) die("not enough arguments (try -h for help)");

	for (i=1; i<argc-1; i++) {
    if (!strcmp(argv[i],"-d")) {
			g_workbpp_cmd=atoi(argv[++i]);
			if (g_workbpp_cmd!=8 && g_workbpp_cmd!=16) {
				die("invalid output depth specified");
			}
		}
		else if (!strcmp(argv[i],"-l")) {
			temp=atoi(argv[++i]);
			if (temp>=0) g_max_levels=max(1,temp); else g_sub_levels=-temp;
		}
		else if (!strcmp(argv[i],"--nomask")) g_nomask=true;
		else if (!strcmp(argv[i],"--nocrop")) g_crop=false;
		else if (!strcmp(argv[i],"--bigtiff")) g_bigtiff=true;
		else if (!strcmp(argv[i],"--bgr")) g_bgr=true;
		else if (!strcmp(argv[i],"--wideblend")) g_wideblend=true;
		else if (!strcmp(argv[i],"--noswap")) g_swap=false;
		else if (!strcmp(argv[i],"--reverse")) g_reverse=true;
		else if (!strcmp(argv[i],"--timing")) g_timing=true;

		else if (!strcmp(argv[i],"-w")) output(0,"ignoring enblend option -w\n");
		else if (!strncmp(argv[i],"-f",2)) output(0,"ignoring enblend option -f\n");
		else if (!strcmp(argv[i],"-a")) output(0,"ignoring enblend option -a\n");

		else if (!strncmp(argv[i],"--compression",13)) {
			char* comp=argv[i]+14;
			if (strcmp(comp,"0")==0) g_jpegquality=0;
			else if (atoi(comp)>0) g_jpegquality=atoi(comp);
			else if (_stricmp(comp,"lzw")==0) g_compression=COMPRESSION_LZW;
			else if (_stricmp(comp,"packbits")==0) g_compression=COMPRESSION_PACKBITS;
//			else if (_stricmp(comp,"deflate")==0) g_compression=COMPRESSION_DEFLATE;
			else if (_stricmp(comp,"none")==0) g_compression=COMPRESSION_NONE;
			else die("unknown compression codec!");
		}
    else if (!strcmp(argv[i],"-v") || !strcmp(argv[i],"--verbose")) g_verbosity++;
		else if (!strcmp(argv[i],"-q") || !strcmp(argv[i],"--quiet")) g_verbosity--;
    else if (!strcmp(argv[i],"--debug")) g_debug=true;
		else if (!strcmp(argv[i],"--saveseams") || !strcmp(argv[i],"--save-seams")) g_seamsave_filename=argv[++i];
    else if (!strcmp(argv[i],"--loadseams") || !strcmp(argv[i],"--load-seams")) g_seamload_filename=argv[++i];
    else if (!strcmp(argv[i],"--savemasks") || !strcmp(argv[i],"--save-masks")) g_savemasks=true;
		else if (!strcmp(argv[i],"--savexor") || !strcmp(argv[i],"--save-xor")) g_xor_filename=argv[++i];
    else if (!strcmp(argv[i],"--no-output")) g_nooutput=true;
    else if (!strcmp(argv[i],"-o") || !strcmp(argv[i],"--output")) {
      g_output_filename=argv[++i];
			char* ext=strrchr(g_output_filename,'.')+1;
			if (!(_stricmp(ext,"jpg") && _stricmp(ext,"jpeg"))) {
				if (g_compression!=-1) {
					output(0,"Warning: JPEG output; ignoring TIFF compression setting\n");
					g_compression=-1;
				}
				if (g_jpegquality==-1) g_jpegquality=75;
			} else if (!(_stricmp(ext,"tif") && _stricmp(ext,"tiff"))) {
				if (g_jpegquality!=-1) {
					output(0,"Warning: TIFF output; ignoring JPEG quality setting\n");
					g_jpegquality=-1;
				} else if (g_compression==-1) {
					g_compression=COMPRESSION_NONE;
				}
			} else {
				die("unknown file extension!");
			}

      i++;
      break;
    } else {
      output(0, "unknown argument \"%s\"",argv[i]);
    }
  }

  if (!g_output_filename && !g_seamsave_filename) die("no output file specified");

  if (!strcmp(argv[i],"--")) i++;

  input_args=argc-i;
  if (input_args==0) die("no input files specified");
  if (input_args>255) die("too many input images specified (current limit is 255");

	go(&argv[i],input_args);

  if (g_timing) timer_all.report("Execution time");

  if (g_debug) {
		output(0, "\nPress Enter to end\n");
		getchar();
	}

	return 0;
}
