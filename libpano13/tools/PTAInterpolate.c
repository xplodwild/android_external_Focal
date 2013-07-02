/*
 *  Author: Max Lyons, January 2007
 *
 *  Description: This program is an enhanced version of Helmut Dersch's
 *  original PTInterpolate program.  PTInterpolate's source code was never
 *  released, so this program was written from scratch.  It is believed that 
 *  this program should function as a substitute for the original PTInterpolate 
 *  program in most circumstances.  Enhancements to the original PTInterpolate 
 *  program include:
 *
 *  1. When used with the accompanying source code in the original distribution, 
 *     the output is in TIFF format, not PSD (with an incorrect extension)
 *  2. The output files produced by this program include a numeric value
 *     indicating the amount of the interplation position between left and 
 *     right images
 *  3. If no valid "i" lines are included in script, then this program
 *     triangulates the c lines, and ovrewrites the script file
 * 
 *  Usage:
 *    
 *    PTAInterpolate.exe script.txt [param2]
 *
 *  Where "param2" is either an image index (0=left, 1=right) to be used when 
 *     triangulating, or a filename prefix to be used when interpolating.
 *
 * License:
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
 */

#include "filter.h"
#include "file.h"
#include <stdio.h>

int writeProject( AlignInfo *g, fullPath *pFile);

#define PTA_INTERPOLATE_VERSION "PTAInterpolate Version " VERSION ", originally written by Helmut Dersch, rewritten and enhanced by Max Lyons\n"

int                     CheckParams( AlignInfo *g );

int main(int argc,char *argv[])
{
	aPrefs		aP;

	char*		script;
	OptInfo		opt;
	AlignInfo	ainf;

	fullPath	infile;
	int left_retVal, right_retVal;

	fullPath sourcefile0, sourcefile1;
	fullPath tempFile0, tempFile1;
	fullPath outfile_prefix, outfile;
	int imgIdx = 0;

	double s;
	double p_a = 0, p_b = 1, p_c = 0.5;
	int outputCtr = 0;

	SetAdjustDefaults(&aP);

	if(argc < 2)
	{
		printf(PTA_INTERPOLATE_VERSION);
		printf("Usage: %s script.txt [outputname]\n", argv[0]);
		exit(1);
	}

	//First arg is always a script file
	StringtoFullPath(&infile, argv[1]);

	script = LoadScript( &infile );
	
	if( script == NULL ) exit(1);
	if (ParseScript( script, &ainf ) != 0) exit(1);
	
	//If no images have been provided in script, then we clearly don't have 
	//enough information to optimize or interpolate this project.  We
	//assume that the user wants us to triangulate the control points
	//instead
	if (ainf.numIm==0)
	{
		//Do triangulation in 'imgIdx'th image
		if (argc > 2) imgIdx = atoi(argv[2]);

		TriangulatePoints( &ainf, imgIdx );
		
		writeProject( &ainf, &infile);	//overwrite input script
	}
	else
	{
		StringtoFullPath(&outfile_prefix, (argc>=3 ? argv[2] : "interpolate_output"));
		
		//Exit if not enough correct data has been provided for us to optimize
		if( CheckParams( &ainf ) != 0 )	exit (1);

		ainf.fcn	= fcnPano;
		
		SetGlobalPtr( &ainf ); 
		
		opt.numVars 		= ainf.numParam;
		opt.numData 		= ainf.numPts;
		opt.SetVarsToX		= SetLMParams;
		opt.SetXToVars		= SetAlignParams;
		opt.fcn				= ainf.fcn;
		*opt.message		= 0;

		//----------------------------------------------------------//
		//First...Optimize the project. This seems to work best if
		//only one of the two images (y,p,r) is optimized.  Not sure
		//why yet.
		RunLMOptimizer( &opt );
		ainf.data		= opt.message;
		WriteResults( script, &infile, &ainf, distSquared, 0);
		
		//----------------------------------------------------------//
		//Second...Create as many output images as requested.
		//To create each output image, interpolate each image twice: 
		//Once for triangles in left image, and a second time for 
		//triangles in second image, and then blend the results back 
		//together to create final image

        strcpy(sourcefile0.name, ainf.im[0].name);
        strcpy(sourcefile1.name, ainf.im[1].name);
		
		//PTools parser puts the values for a,b,c here:
		p_a = ainf.pano.cP.radial_params[0][3];
		p_b = ainf.pano.cP.radial_params[0][2];
		p_c = ainf.pano.cP.radial_params[0][1];
				
		
		//Iterate and create output images
		for (s = p_a; s <= p_b+0.001; s += p_c)
		{
			//PTools expects the output position (0 <= s <= 1) here
			ainf.pano.cP.vertical_params[0] = s;
			
			strcpy(tempFile0.name, "");
	        if (panoFileMakeTemp(&tempFile0) == 0) 
	        {
	            PrintError("Could not create temporary file");
	            return -1;
	        }
	        
	        //Interpolate triangles from left image (image 0)
	        left_retVal = InterpolateImageFile( &sourcefile0, &tempFile0, &ainf, 0 );
	        
	        strcpy(tempFile1.name, "");
	        if (panoFileMakeTemp(&tempFile1) == 0) 
	        {
	            PrintError("Could not create temporary file");
	            return -1;
	        }			        
	        
	        //Interpolate triangles from right image (image 1)
	        right_retVal = InterpolateImageFile( &sourcefile1, &tempFile1, &ainf, 1 );

			//Create a useful output name, and composite left/right triangles
			sprintf(outfile.name, "%s_%.3d_%.2d.tif", outfile_prefix.name, (int)(s * 100), ++outputCtr);
			
			//a value of 1, indicates no interpolation happened (no triangles in image)
			//0 indicates success
			if (left_retVal!=0 && right_retVal!=0)		//no triangle in either image, or error
				return -1;
			else if (left_retVal==0 && right_retVal!=0)	//triangles in left image only
				rename(tempFile0.name, outfile.name);	
			else if (left_retVal!=0 && right_retVal==0) //triangles in right image only
				rename(tempFile1.name, outfile.name);
			else										//triangle in both images
				blendImages( &tempFile0,  &tempFile1, &outfile, 0.5);
			
			if (left_retVal == 0) remove(tempFile0.name);
			if (right_retVal == 0) remove(tempFile1.name);

		}
	}
	
	exit(0);

}
