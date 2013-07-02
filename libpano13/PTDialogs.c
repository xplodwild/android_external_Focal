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

#ifdef __Mac__
  #include "sys_mac.h"
#endif

// Jim Watters 2008/01/11  need sys_win.h for dialogs to work in Windows
//                         need sys_ansi.h for command prompt to work
#ifdef __Ansi__
  #include "sys_ansi.h"
#elif defined (__Win__)
  #include "sys_win.h"
#endif


#if !__Mac__ && !__Win__ && !__Ansi__
  #include "sys_X11.h"
#endif

#if _MSC_VER > 1000
#pragma warning(disable: 4100) // disable unreferenced formal parameter warning
#endif

// Please do not change the way these dialogs work. You will certainly mess
// up one of the platform specific versions.

//----------------------- Dialogs for remap  -------------------------------------------




int SetRemapPrefs(  rPrefs * thePrefs )
{
	GenDialog( rPrefs,  
			   kSetRemapPrefs_dlg, "Remap Options",
			   {
			   		CheckButton( kSetRemapPrefs_InRect, 			localPrefs.from == _rectilinear);
					CheckButton( kSetRemapPrefs_InPano, 			localPrefs.from == _panorama);
					CheckButton( kSetRemapPrefs_InErect, 			localPrefs.from == _equirectangular);
					CheckButton( kSetRemapPrefs_InSphereCenter, 	localPrefs.from == _spherical_cp);
					CheckButton( kSetRemapPrefs_InSphereTop,		localPrefs.from == _spherical_tp);
					CheckButton( kSetRemapPrefs_InMirror, 			localPrefs.from == _mirror);

					CheckButton( kSetRemapPrefs_OutRect, 			localPrefs.to == _rectilinear);
					CheckButton( kSetRemapPrefs_OutPano, 			localPrefs.to == _panorama);
					CheckButton( kSetRemapPrefs_OutErect, 			localPrefs.to == _equirectangular);
					CheckButton( kSetRemapPrefs_OutSphereCenter, 	localPrefs.to == _spherical_cp);
					CheckButton( kSetRemapPrefs_OutSphereTop, 		localPrefs.to == _spherical_tp);
					CheckButton( kSetRemapPrefs_OutMirror, 			localPrefs.to == _mirror);
				},
				{
					SetText( kSetRemapPrefs_Hfov, "%g", localPrefs.hfov );
					SetText( kSetRemapPrefs_Vfov, "%g", localPrefs.vfov );
				}, 
				{
					GetText( kSetRemapPrefs_Hfov, "%lf", &localPrefs.hfov );
					GetText( kSetRemapPrefs_Vfov, "%lf", &localPrefs.vfov );
				},
				{
					case kSetRemapPrefs_InRect:
						localPrefs.from = _rectilinear;
						break;
					case kSetRemapPrefs_InPano:
						localPrefs.from = _panorama;
						break;
					case kSetRemapPrefs_InErect:
						localPrefs.from = _equirectangular;
						break;
					case kSetRemapPrefs_InSphereCenter:
						localPrefs.from = _spherical_cp;
						break;
					case kSetRemapPrefs_InSphereTop:
						localPrefs.from = _spherical_tp;
						break;
					case kSetRemapPrefs_InMirror:
						localPrefs.from = _mirror;
						break;
					case kSetRemapPrefs_OutRect:
						localPrefs.to = _rectilinear;
						break;
					case kSetRemapPrefs_OutPano:
						localPrefs.to = _panorama;
						break;
					case kSetRemapPrefs_OutErect:
						localPrefs.to = _equirectangular;
						break;
					case kSetRemapPrefs_OutSphereCenter:
						localPrefs.to = _spherical_cp;
						break;
					case kSetRemapPrefs_OutSphereTop:
						localPrefs.to = _spherical_tp;
						break;
					case kSetRemapPrefs_OutMirror:
						localPrefs.to = _mirror;
						break;
					case kSetRemapPrefs_SetPrefs:
						if( setSizePrefs( gsPrPtr, gTrPtr->mode & _hostCanResize ))
						{
							writePrefs((char*) gsPrPtr, _sizep );
						}
						break;
				}, SetRem)
}

//-------------------------- Dialogs for perspective  -------------------------------------------

int SetPerspectivePrefs(  pPrefs * thePrefs )
{
	GenDialog( pPrefs,  
			   kSetPerspectivePrefs_dlg	, "Perspective Options",
			   {
					CheckButton( kSetPerspectivePrefs_InRect, 		localPrefs.format == _rectilinear);
					CheckButton( kSetPerspectivePrefs_InSphere, 	localPrefs.format == _spherical_tp);
					CheckButton( kSetPerspectivePrefs_Degree, 		!localPrefs.unit_is_cart );
					CheckButton( kSetPerspectivePrefs_Points, 		localPrefs.unit_is_cart);
				},
				{
					SetText( kSetPerspectivePrefs_X, 		"%g", localPrefs.x_alpha );
					SetText( kSetPerspectivePrefs_Y, 		"%g", localPrefs.y_beta );
					SetText( kSetPerspectivePrefs_Hfov, 	"%g", localPrefs.hfov );
					SetText( kSetPerspectivePrefs_Gamma, 	"%g", localPrefs.gamma );
					SetText( kSetPerspectivePrefs_Width, 	"%ld", localPrefs.width );
					SetText( kSetPerspectivePrefs_Height, 	"%ld", localPrefs.height );
				},
				{
					GetText( kSetPerspectivePrefs_X, 		"%lf", &localPrefs.x_alpha );
					GetText( kSetPerspectivePrefs_Y, 		"%lf", &localPrefs.y_beta );
					GetText( kSetPerspectivePrefs_Hfov, 	"%lf", &localPrefs.hfov );
					GetText( kSetPerspectivePrefs_Gamma, 	"%lf", &localPrefs.gamma );
					GetText( kSetPerspectivePrefs_Width, 	"%ld", &localPrefs.width );
					GetText( kSetPerspectivePrefs_Height, 	"%ld", &localPrefs.height );
				},
				{
					case kSetPerspectivePrefs_InRect:
						localPrefs.format = _rectilinear;
						break;
					case kSetPerspectivePrefs_InSphere:
						localPrefs.format = _spherical_tp;
						break;
					case kSetPerspectivePrefs_Degree:
						localPrefs.unit_is_cart = FALSE;
						break;
					case kSetPerspectivePrefs_Points	:
						localPrefs.unit_is_cart = TRUE;
						break;
					case kSetPerspectivePrefs_Keep:
						SetText( kSetPerspectivePrefs_Width, "%ld", gTrPtr->src->width );
						SetText( kSetPerspectivePrefs_Height, "%ld", gTrPtr->src->height );
						break;
					case kSetPerspectivePrefs_SetPrefs:
						if( setSizePrefs( gsPrPtr, gTrPtr->mode & _hostCanResize ))
						{
							writePrefs((char*) gsPrPtr, _sizep );
						}
						break;
				}, SetPerspPrefs )
}
		


// ---------------- Dialogs for correct ------------------------------------------------

int SetCorrectPrefs( cPrefs * thePrefs )
{
	GenDialog( cPrefs,  
			   kSetCorrectPrefs_dlg, "Correction", 
			   {
					CheckButton( kSetCorrectPrefs_Radial, 		localPrefs.radial);
					CheckButton( kSetCorrectPrefs_Horizontal, 	localPrefs.horizontal );
					CheckButton( kSetCorrectPrefs_Vertical, 	localPrefs.vertical);
					CheckButton( kSetCorrectPrefs_Shear, 		localPrefs.shear);
					CheckButton( kSetCorrectPrefs_Scale, 		localPrefs.resize);
					CheckButton( kSetCorrectPrefs_Lum, 			localPrefs.luminance);
					CheckButton( kSetCorrectPrefs_CutFrame, 	localPrefs.cutFrame);
					CheckButton( kSetCorrectPrefs_Fourier, 		localPrefs.fourier);
				},;,;,
				{
					case kSetCorrectPrefs_Save:
						SaveOptions( &localPrefs );
						break;
					case kSetCorrectPrefs_Load:
						LoadOptions( &localPrefs );				
						break;
					case kSetCorrectPrefs_Radial:
						localPrefs.radial = !localPrefs.radial;
						break;
					case kSetCorrectPrefs_Horizontal:
						localPrefs.horizontal = !localPrefs.horizontal;
						break;
					case kSetCorrectPrefs_Vertical:
						localPrefs.vertical = !localPrefs.vertical;
						break;
					case kSetCorrectPrefs_Shear:
						localPrefs.shear = !localPrefs.shear;
						break;
					case kSetCorrectPrefs_Scale:
						localPrefs.resize = !localPrefs.resize;
						break;
					case kSetCorrectPrefs_RadialOption:
						SetRadialOptions(  &localPrefs );
						SetCorrectionRadius( &localPrefs );
						break;
					case kSetCorrectPrefs_HorizontalOption:
						SetHorizontalOptions(  &localPrefs );
						break;
					case kSetCorrectPrefs_VerticalOption:
						SetVerticalOptions(  &localPrefs );
						break;
					case kSetCorrectPrefs_ShearOption:
						SetShearOptions(  &localPrefs );
						break;
					case kSetCorrectPrefs_ScaleOption:
						SetScaleOptions(  &localPrefs );
						break;
					case kSetCorrectPrefs_Lum:
						localPrefs.luminance = !localPrefs.luminance;
						break;
					case kSetCorrectPrefs_LumOpt:
						SetLumOptions(  &localPrefs);
						break;
					case kSetCorrectPrefs_SetPrefs:
						if( setSizePrefs( gsPrPtr, gTrPtr->mode & _hostCanResize ))
						{
							writePrefs((char*) gsPrPtr, _sizep );
						}
						break;
					case kSetCorrectPrefs_CutFrame:
						localPrefs.cutFrame = !localPrefs.cutFrame;
						break;
					case kSetCorrectPrefs_CutOpt:
						SetCutOptions(  &localPrefs );
						break;
					case kSetCorrectPrefs_Fourier:
						localPrefs.fourier = !localPrefs.fourier;
						break;
					case kSetCorrectPrefs_FourierOpt:
						SetFourierOptions(  &localPrefs );
						break;
			}, SetCPrefs )
}

#if !__Mac__ && !__Win__
int SetFourierOptions( cPrefs * thePrefs )
{
	GenDialog( cPrefs,  
			   kSetFourierOptions_dlg, "Fourier Filtering Options", 
			   {
					CheckButton( kSetFourierOptions_addBlur, 	localPrefs.fourier_mode == _faddBlurr );
					CheckButton( kSetFourierOptions_remBlur, 	localPrefs.fourier_mode == _fremoveBlurr);
					CheckButton( kSetFourierOptions_internal, 	localPrefs.fourier_nf   == _nf_internal);
					CheckButton( kSetFourierOptions_custom, 	localPrefs.fourier_nf   == _nf_custom);
				},
				{
					SetText( kSetFourierOptions_filterfactor, 	"%g", localPrefs.filterfactor );
					SetText( kSetFourierOptions_fourier_frame,	"%g", localPrefs.fourier_frame);
				},
				{
					GetText( kSetFourierOptions_filterfactor, 	"%lf", &localPrefs.filterfactor );
					GetText( kSetFourierOptions_fourier_frame,	"%lf", &localPrefs.fourier_frame);
				},
				{
					case kSetFourierOptions_findPSD:
						FindFile( &(localPrefs.psf) );
						break;
					case kSetFourierOptions_addBlur:
						localPrefs.fourier_mode = _faddBlurr;
						break;
					case kSetFourierOptions_remBlur:
						localPrefs.fourier_mode = _fremoveBlurr;
						break;
					case kSetFourierOptions_internal:
						localPrefs.fourier_nf   = _nf_internal;
						break;
					case kSetFourierOptions_custom:
						localPrefs.fourier_nf   = _nf_custom;
						break;
					case kSetFourierOptions_findNFF:
						FindFile( &(localPrefs.nff) );
						break;
				}, SetFrPrefs )
}
#endif


#ifdef __Win__
int SetFourierOptions( cPrefs * thePrefs )
{
	GenDialog( cPrefs,  
			   kSetFourierOptions_dlg, "Fourier Filtering Options", 
			   {
					CheckButton( kSetFourierOptions_addBlur, 	localPrefs.fourier_mode == _faddBlurr );
					CheckButton( kSetFourierOptions_remBlur, 	localPrefs.fourier_mode == _fremoveBlurr);
					CheckButton( kSetFourierOptions_internal, 	localPrefs.fourier_nf   == _nf_internal);
					CheckButton( kSetFourierOptions_custom, 	localPrefs.fourier_nf   == _nf_custom);
				},
				{
					SetLbl(  kSetFourierOptions_PSDname,  (char*)&(localPrefs.psf) );
					SetText( kSetFourierOptions_filterfactor, 	"%g", localPrefs.filterfactor );
					SetText( kSetFourierOptions_fourier_frame,	"%g", localPrefs.fourier_frame);
				},
				{
					GetLbl( kSetFourierOptions_PSDname, (char*)&(localPrefs.psf), 255 );
					GetText( kSetFourierOptions_filterfactor, 	"%lf", &localPrefs.filterfactor );
					GetText( kSetFourierOptions_fourier_frame,	"%lf", &localPrefs.fourier_frame);
				},
				{
					case kSetFourierOptions_findPSD:
						{
							fullPath path;

							if( ! FindFile( &path ))
							{
								memcpy( &localPrefs.psf, &path, sizeof( fullPath ) );
								SetLbl(  kSetFourierOptions_PSDname,  (char*)&(localPrefs.psf) );
							}
						}
						break;
					case kSetFourierOptions_addBlur:
						localPrefs.fourier_mode = _faddBlurr;
						break;
					case kSetFourierOptions_remBlur:
						localPrefs.fourier_mode = _fremoveBlurr;
						break;
					case kSetFourierOptions_internal:
						localPrefs.fourier_nf   = _nf_internal;
						break;
					case kSetFourierOptions_custom:
						localPrefs.fourier_nf   = _nf_custom;
						break;
					case kSetFourierOptions_findNFF:
						FindFile( &(localPrefs.nff) );
						break;
				}, SetFrPrefs )
}
#endif



#ifdef __Mac__
int SetFourierOptions( cPrefs * thePrefs )
{
	GenDialog( cPrefs,  
			   kSetFourierOptions_dlg, "Fourier Filtering Options", 
			   {
					CheckButton( kSetFourierOptions_addBlur, 	localPrefs.fourier_mode == _faddBlurr );
					CheckButton( kSetFourierOptions_remBlur, 	localPrefs.fourier_mode == _fremoveBlurr);
					CheckButton( kSetFourierOptions_scale, 		localPrefs.fourier_mode == _fresize);
					CheckButton( kSetFourierOptions_internal, 	localPrefs.fourier_nf   == _nf_internal);
					CheckButton( kSetFourierOptions_custom, 	localPrefs.fourier_nf   == _nf_custom);
					ConvFileName( &(localPrefs.psf), &numString[120]); 
					SetLbl(  kSetFourierOptions_PSDname, &numString[120]);
				},
				{
					SetText( kSetFourierOptions_filterfactor, 	"%g", localPrefs.filterfactor );
					SetText( kSetFourierOptions_fourier_frame,	"%g", localPrefs.fourier_frame);
				},
				{
					GetText( kSetFourierOptions_filterfactor, 	"%lf", &localPrefs.filterfactor );
					GetText( kSetFourierOptions_fourier_frame,	"%lf", &localPrefs.fourier_frame);
				},
				{
					case kSetFourierOptions_findPSD: 
						FindFile( &(localPrefs.psf) );
						break;
					case kSetFourierOptions_addBlur:
						localPrefs.fourier_mode = _faddBlurr;
						break;
					case kSetFourierOptions_scale:
						localPrefs.fourier_mode = _fresize;
						break;
					case kSetFourierOptions_remBlur:
						localPrefs.fourier_mode = _fremoveBlurr;
						break;
					case kSetFourierOptions_internal:
						localPrefs.fourier_nf   = _nf_internal;
						break;
					case kSetFourierOptions_custom:
						localPrefs.fourier_nf   = _nf_custom;
						break;
					case kSetFourierOptions_findNFF:
						FindFile( &(localPrefs.nff) );
						break;
				}, SetFrPrefs )
}
#endif

int SetLumOptions(  cPrefs * thePrefs )
{
	GenDialog( cPrefs,  
			   kSetLumOptions_dlg, "Luminance Correction Options",;,
			   {
			   		switch( gTrPtr->src->dataformat )
					{
						case _RGB:	SetLbl( kSetLumOptions_RedText,  	"Red");
									SetLbl( kSetLumOptions_GreenText,  	"Green");
									SetLbl( kSetLumOptions_BlueText,  	"Blue");
									break;
						case _Lab:	SetLbl( kSetLumOptions_RedText,  	"Lightness");
									SetLbl( kSetLumOptions_GreenText,  	"Color a");
									SetLbl( kSetLumOptions_BlueText,  	"Color b");
									break;
					}
					SetText( kSetLumOptions_Red, 	"%g", localPrefs.lum_params[0] );
					SetText( kSetLumOptions_Green, 	"%g", localPrefs.lum_params[1] );
					SetText( kSetLumOptions_Blue, 	"%g", localPrefs.lum_params[2] );
				},
				{
					GetText( kSetLumOptions_Red, 	"%lf", &localPrefs.lum_params[0] );
					GetText( kSetLumOptions_Green, 	"%lf", &localPrefs.lum_params[1] );
					GetText( kSetLumOptions_Blue, 	"%lf", &localPrefs.lum_params[2] );
				},;,
				SetLumOpt)
}

int SetRadialOptions(  cPrefs * thePrefs )
{
	GenDialog(cPrefs,  
			   kSetRadialOptions_dlg, /*"Radial Shift Options"*/"Set Polynomial Coefficients for Radial Shift" /*Changed by Kekus Digital August 14 2003*/,
			   {
					CheckButton( kSetRadialOptions_radial, 		localPrefs.correction_mode == correction_mode_radial );
					CheckButton( kSetRadialOptions_vertical, 	localPrefs.correction_mode == correction_mode_vertical  );
					CheckButton( kSetRadialOptions_horizontal, 	localPrefs.correction_mode == correction_mode_deregister );
				},
			   {
					SetText( kSetRadialOptions_Red0, 	"%g", localPrefs.radial_params[0][0] );
					SetText( kSetRadialOptions_Red1, 	"%g", localPrefs.radial_params[0][1] );
					SetText( kSetRadialOptions_Red2, 	"%g", localPrefs.radial_params[0][2] );
					SetText( kSetRadialOptions_Red3, 	"%g", localPrefs.radial_params[0][3] );
					SetText( kSetRadialOptions_Green0, 	"%g", localPrefs.radial_params[1][0] );
					SetText( kSetRadialOptions_Green1, 	"%g", localPrefs.radial_params[1][1] );
					SetText( kSetRadialOptions_Green2, 	"%g", localPrefs.radial_params[1][2] );
					SetText( kSetRadialOptions_Green3, 	"%g", localPrefs.radial_params[1][3] );
					SetText( kSetRadialOptions_Blue0, 	"%g", localPrefs.radial_params[2][0] );
					SetText( kSetRadialOptions_Blue1, 	"%g", localPrefs.radial_params[2][1] );
					SetText( kSetRadialOptions_Blue2, 	"%g", localPrefs.radial_params[2][2] );
					SetText( kSetRadialOptions_Blue3, 	"%g", localPrefs.radial_params[2][3] );
				},
				{
					GetText( kSetRadialOptions_Red0, 	"%lf", &localPrefs.radial_params[0][0] );
					GetText( kSetRadialOptions_Red1, 	"%lf", &localPrefs.radial_params[0][1] );
					GetText( kSetRadialOptions_Red2, 	"%lf", &localPrefs.radial_params[0][2] );
					GetText( kSetRadialOptions_Red3, 	"%lf", &localPrefs.radial_params[0][3] );
					GetText( kSetRadialOptions_Green0, 	"%lf", &localPrefs.radial_params[1][0] );
					GetText( kSetRadialOptions_Green1, 	"%lf", &localPrefs.radial_params[1][1] );
					GetText( kSetRadialOptions_Green2, 	"%lf", &localPrefs.radial_params[1][2] );
					GetText( kSetRadialOptions_Green3, 	"%lf", &localPrefs.radial_params[1][3] );
					GetText( kSetRadialOptions_Blue0, 	"%lf", &localPrefs.radial_params[2][0] );
					GetText( kSetRadialOptions_Blue1, 	"%lf", &localPrefs.radial_params[2][1] );
					GetText( kSetRadialOptions_Blue2, 	"%lf", &localPrefs.radial_params[2][2] );
					GetText( kSetRadialOptions_Blue3, 	"%lf", &localPrefs.radial_params[2][3] );
				},
				{
					case kSetRadialOptions_radial:
						localPrefs.correction_mode = correction_mode_radial;
						break;
					case kSetRadialOptions_vertical:
						localPrefs.correction_mode = correction_mode_vertical;
						break;
					case kSetRadialOptions_horizontal:
						localPrefs.correction_mode = correction_mode_deregister;
						break;
				},SetRadOpt)
}

int SetHorizontalOptions(  cPrefs * thePrefs )
{
	GenDialog( cPrefs,
			   kSetHorizontalOptions_dlg, "Horizontal Shift Options",;,
			   {
					SetText( kSetHorizontalOptions_Red, 	"%g", localPrefs.horizontal_params[0] );
					SetText( kSetHorizontalOptions_Green, 	"%g", localPrefs.horizontal_params[1] );
					SetText( kSetHorizontalOptions_Blue, 	"%g", localPrefs.horizontal_params[2] );
				},
				{
					GetText( kSetHorizontalOptions_Red, 	"%lf", &localPrefs.horizontal_params[0] );
					GetText( kSetHorizontalOptions_Green, 	"%lf", &localPrefs.horizontal_params[1] );
					GetText( kSetHorizontalOptions_Blue, 	"%lf", &localPrefs.horizontal_params[2] );
				}, ;,SetHorOpt)
}

int SetVerticalOptions(  cPrefs * thePrefs )
{
	GenDialog(cPrefs,
			   kSetHorizontalOptions_dlg, "Vertical Shift Options",;,
			   {
					SetText( kSetHorizontalOptions_Red, 	"%g", localPrefs.vertical_params[0] );
					SetText( kSetHorizontalOptions_Green, 	"%g", localPrefs.vertical_params[1] );
					SetText( kSetHorizontalOptions_Blue, 	"%g", localPrefs.vertical_params[2] );
				},
				{
					GetText( kSetHorizontalOptions_Red, 	"%lf", &localPrefs.vertical_params[0] );
					GetText( kSetHorizontalOptions_Green, 	"%lf", &localPrefs.vertical_params[1] );
					GetText( kSetHorizontalOptions_Blue, 	"%lf", &localPrefs.vertical_params[2] );
				},; ,SetVerOpt)
}

int SetShearOptions(  cPrefs * thePrefs )
{
	GenDialog(cPrefs,
			   kSetShearOptions_dlg, "Shear Options",;,
			   {
					SetLbl( kSetShearOptions_vname,  "Vertical");
					SetText( kSetShearOptions_vvar, "%g", localPrefs.shear_y);
					SetLbl( kSetShearOptions_hname,  "Horizontal");
					SetText( kSetShearOptions_hvar, "%g", localPrefs.shear_x);
				},
			   {
					GetText( kSetShearOptions_vvar, "%lf", &localPrefs.shear_y);
					GetText( kSetShearOptions_hvar, "%lf", &localPrefs.shear_x);
				}, ;, SetShOpt)
}

int SetScaleOptions(  cPrefs * thePrefs )
{
	GenDialog( cPrefs,
			   kSetShearOptions_dlg, "Resize Options",;,
			   {
					SetLbl( kSetShearOptions_vname,  "Width");
					SetText( kSetShearOptions_vvar, "%ld", localPrefs.width);
					SetLbl( kSetShearOptions_hname,  "Height");
					SetText( kSetShearOptions_hvar, "%ld", localPrefs.height);
				},
			   {
					GetText( kSetShearOptions_vvar, "%ld", &localPrefs.width);
					GetText( kSetShearOptions_hvar, "%ld", &localPrefs.height);
				}, ;,SetScOpt)
}


int SetCutOptions(  cPrefs * thePrefs )
{
	GenDialog(cPrefs,
			   kSetShearOptions_dlg, "Cut Frame Options",;,
			   {
					SetLbl( kSetShearOptions_vname,  "Width");
					SetText( kSetShearOptions_vvar, "%ld", localPrefs.fwidth);
					SetLbl( kSetShearOptions_hname,  "Height");
					SetText( kSetShearOptions_hvar, "%ld", localPrefs.fheight);
				},
			   {
					GetText( kSetShearOptions_vvar, "%ld", &localPrefs.fwidth);
					GetText( kSetShearOptions_hvar, "%ld", &localPrefs.fheight);
				},; ,SetCutOpt)
}


int SetAdjustPrefs( aPrefs * thePrefs )
{
	GenDialog( aPrefs,
			   kSetAdjustPrefs_dlg, "Adjust Options",
			   {
					CheckButton( kSetAdjustPrefs_Insert, 		((localPrefs.mode & 7) == _insert ));
					CheckButton( kSetAdjustPrefs_Extract, 		((localPrefs.mode & 7) == _extract ));
					CheckButton( kSetAdjustPrefs_SetCtrlPts, 	((localPrefs.mode & 7) == _readControlPoints));
					CheckButton( kSetAdjustPrefs_RunOptimizer, 	((localPrefs.mode & 7) == _runOptimizer));
					CheckButton( kSetAdjustPrefs_Options, 		!(localPrefs.mode & _useScript));
					CheckButton( kSetAdjustPrefs_Script, 		localPrefs.mode & _useScript  );
				},; ,; ,
				{
					case kSetAdjustPrefs_Insert:
						localPrefs.mode = (localPrefs.mode & _useScript) + _insert;
						break;
					case kSetAdjustPrefs_Extract:
						localPrefs.mode = (localPrefs.mode & _useScript) + _extract;
						break;
					case kSetAdjustPrefs_SetCtrlPts:
						localPrefs.mode = (localPrefs.mode & _useScript) + _readControlPoints;
						break;
					case kSetAdjustPrefs_RunOptimizer:
						localPrefs.mode = (localPrefs.mode & _useScript) + _runOptimizer;
						break;
					case kSetAdjustPrefs_Options:
						localPrefs.mode &= (~_useScript);
						break;
					case kSetAdjustPrefs_Script:
						localPrefs.mode |= _useScript;
						break;
					case kSetAdjustPrefs_SetOpt:
						SetCreateOptions(  &localPrefs );
						break;
					case kSetAdjustPrefs_FindScript	:
						FindScript( &localPrefs );
						break;
					case kSetAdjustPrefs_SetPrefs:
						if( setSizePrefs( gsPrPtr, gTrPtr->mode & _hostCanResize ))
						{
							writePrefs((char*) gsPrPtr, _sizep );
						}
						break;
				},SetAdPrefs)
}





//					CheckButton( kSetCreateOptions_PCu, 		FALSE);	


int SetCreateOptions(  aPrefs * thePrefs )
{
	GenDialog(aPrefs,
			   kSetCreateOptions_dlg, "Options for Insert/Extract",
			   {
					CheckButton( kSetCreateOptions_ImR, 		(localPrefs.im.format == _rectilinear ));	
					CheckButton( kSetCreateOptions_ImP, 		(localPrefs.im.format == _panorama ));	
					CheckButton( kSetCreateOptions_ImFf, 		(localPrefs.im.format == _fisheye_ff));	
					CheckButton( kSetCreateOptions_ImFc, 		(localPrefs.im.format == _fisheye_circ));	
					CheckButton( kSetCreateOptions_ImEq, 		(localPrefs.im.format == _equirectangular));	

					CheckButton( kSetCreateOptions_PRe, 		(localPrefs.pano.format == _rectilinear ));	
					CheckButton( kSetCreateOptions_PPa, 		(localPrefs.pano.format == _panorama ));	
					CheckButton( kSetCreateOptions_PSp, 		(localPrefs.pano.format == _equirectangular));	

					CheckButton( kSetCreateOptions_PSave, 		(*localPrefs.sBuf.destName != 0));	
					CheckButton( kSetCreateOptions_SLoad, 		(*localPrefs.sBuf.srcName != 0));
					
					CheckButton( kSetCreateOptions_SPaste, 		(localPrefs.sBuf.seam == _dest ));	
					CheckButton( kSetCreateOptions_SBlend, 		(localPrefs.sBuf.seam == _middle ));	
					CheckButton( kSetCreateOptions_SIm, 		(localPrefs.sBuf.colcorrect == 1 ));	
					CheckButton( kSetCreateOptions_SBuf, 		(localPrefs.sBuf.colcorrect == 2 ));	
					CheckButton( kSetCreateOptions_Sboth, 		(localPrefs.sBuf.colcorrect == 3 ));	
					CheckButton( kSetCreateOptions_Snone, 		(localPrefs.sBuf.colcorrect == 0 ));
				},
				{
					SetText( kSetCreateOptions_ImHfov, 	"%g", 	localPrefs.im.hfov);
					SetText( kSetCreateOptions_ImW, 	"%ld", 	localPrefs.im.width);
					SetText( kSetCreateOptions_ImH, 	"%ld", 	localPrefs.im.height);
					
					SetText( kSetCreateOptions_PHfov, 	"%g", 	localPrefs.pano.hfov);
					SetText( kSetCreateOptions_PWi, 	"%ld", 	localPrefs.pano.width);
					SetText( kSetCreateOptions_PHe, 	"%ld", 	localPrefs.pano.height);
					SetText( kSetCreateOptions_TY, 		"%g",	localPrefs.im.yaw);
					SetText( kSetCreateOptions_TP, 		"%g", 	localPrefs.im.pitch);
					SetText( kSetCreateOptions_TR, 		"%g", 	localPrefs.im.roll);
					SetText( kSetCreateOptions_SF, 		"%ld", 	localPrefs.sBuf.feather);
				},
				{
					GetText( kSetCreateOptions_ImHfov, 	"%lf", &localPrefs.im.hfov);
					GetText( kSetCreateOptions_ImW, 	"%ld", &localPrefs.im.width);
					GetText( kSetCreateOptions_ImH, 	"%ld", &localPrefs.im.height);
					
					GetText( kSetCreateOptions_PHfov, 	"%lf", &localPrefs.pano.hfov);
					GetText( kSetCreateOptions_PWi, 	"%ld", &localPrefs.pano.width);
					GetText( kSetCreateOptions_PHe, 	"%ld", &localPrefs.pano.height);
					GetText( kSetCreateOptions_TY, 		"%lf", &localPrefs.im.yaw);
					GetText( kSetCreateOptions_TP, 		"%lf", &localPrefs.im.pitch);
					GetText( kSetCreateOptions_TR, 		"%lf", &localPrefs.im.roll);
					GetText( kSetCreateOptions_SF, 		"%ld", &localPrefs.sBuf.feather);
				},
				{
					case kSetCreateOptions_ImR:
						localPrefs.im.format = _rectilinear;
						break;
					case kSetCreateOptions_ImP:
						localPrefs.im.format = _panorama;
						break;
					case kSetCreateOptions_ImFf:
						localPrefs.im.format = _fisheye_ff;
						break;
					case kSetCreateOptions_ImFc:
						localPrefs.im.format = _fisheye_circ;
						break;
					case kSetCreateOptions_ImEq:
						localPrefs.im.format = _equirectangular;
						break;
					case kSetCreateOptions_Correct:
						SetCorrectPrefs(  &localPrefs.im.cP );
						break;
					case kSetCreateOptions_PRe:
						localPrefs.pano.format = _rectilinear;
						break;
					case kSetCreateOptions_PPa:
						localPrefs.pano.format = _panorama;
						break;
					case kSetCreateOptions_PSp:
						localPrefs.pano.format = _equirectangular;
						break;
					case kSetCreateOptions_PSave:
						if(*localPrefs.sBuf.destName != 0)
							*localPrefs.sBuf.destName = 0;
						else
							sprintf( localPrefs.sBuf.destName, "buf" );
						break;
					case kSetCreateOptions_SLoad:
						if(*localPrefs.sBuf.srcName != 0)
							*localPrefs.sBuf.srcName = 0;
						else
							sprintf( localPrefs.sBuf.srcName, "buf" );
						break;
					case kSetCreateOptions_SPaste:
						localPrefs.sBuf.seam = _dest;
						break;
					case kSetCreateOptions_SBlend:
						localPrefs.sBuf.seam = _middle;
						break;
					case kSetCreateOptions_SIm:
						localPrefs.sBuf.colcorrect = 1;
						break;
					case kSetCreateOptions_SBuf:
						localPrefs.sBuf.colcorrect = 2;
						break;
					case kSetCreateOptions_Sboth:
						localPrefs.sBuf.colcorrect = 3;
						break;
					case kSetCreateOptions_Snone:
						localPrefs.sBuf.colcorrect = 0;
						break;
					}, SetCrOpt)
}



int	SetPanPrefs(  panControls *thePrefs )
{
	GenDialog( panControls,
			   kSetShearOptions_dlg, "Pan Options",;,
			   {
					SetLbl( kSetShearOptions_vname,  "Pan Angle");
					SetLbl( kSetShearOptions_hname,  "Zoom (%)");
					SetText(  kSetShearOptions_vvar, "%g", localPrefs.panAngle);
					SetText(  kSetShearOptions_hvar, "%g", localPrefs.zoomFactor);
				},
			   {
					GetText( kSetShearOptions_vvar, "%lf", &localPrefs.panAngle);
					GetText( kSetShearOptions_hvar, "%lf", &localPrefs.zoomFactor);
				}, ;, SetPanOpt)
}

 
int SetInterpolator( sPrefs *thePrefs) 
{
  GenDialog( sPrefs,  kSetIntpPrefs_SetIntp, "Interpolation Options",
      {
        CheckButton( kSetIntpPrefs_Poly,        localPrefs.interpolator == _poly3 );
        CheckButton( kSetIntpPrefs_Sp36,        localPrefs.interpolator == _spline36 );
        CheckButton( kSetIntpPrefs_Sp64,        localPrefs.interpolator == _spline64 );
        CheckButton( kSetIntpPrefs_Sinc256,     localPrefs.interpolator == _sinc256 );
        CheckButton( kSetIntpPrefs_AAHammering, localPrefs.interpolator == _aahamming );
        CheckButton( kSetIntpPrefs_AAGaussian,  localPrefs.interpolator == _aagaussian );
        CheckButton( kSetIntpPrefs_AAQuadratic, localPrefs.interpolator == _aaquadratic );
        CheckButton( kSetIntpPrefs_AAMitchell,  localPrefs.interpolator == _aamitchell );
        CheckButton( kSetIntpPrefs_AALauczos2,  localPrefs.interpolator == _aalanczos2 );
        CheckButton( kSetIntpPrefs_AALauczos3,  localPrefs.interpolator == _aalanczos3 );
        CheckButton( kSetIntpPrefs_FastTNorm,   localPrefs.fastStep == FAST_TRANSFORM_STEP_NONE );
        CheckButton( kSetIntpPrefs_FastTMed,    localPrefs.fastStep == FAST_TRANSFORM_STEP_MORPH );
        CheckButton( kSetIntpPrefs_FastTFast,   localPrefs.fastStep == FAST_TRANSFORM_STEP_NORMAL );
      },
      {
        SetText( kSetIntpPrefs_Gamma, 	"%g", 	localPrefs.gamma);
      },
      {
        GetText( kSetIntpPrefs_Gamma, 	"%lf",  &localPrefs.gamma);
      }, 
      {
        case kSetIntpPrefs_Poly:
        localPrefs.interpolator = _poly3;
        break;
        case kSetIntpPrefs_Sp36:
        localPrefs.interpolator = _spline36;
        break;
        case kSetIntpPrefs_Sp64:
        localPrefs.interpolator = _spline64;
        break;
        case kSetIntpPrefs_Sinc256:
        localPrefs.interpolator = _sinc256;
        break;
        case kSetIntpPrefs_AAHammering:
        localPrefs.interpolator = _aahamming;
        break;
        case kSetIntpPrefs_AAGaussian:
        localPrefs.interpolator = _aagaussian;
        break;
        case kSetIntpPrefs_AAQuadratic:
        localPrefs.interpolator = _aaquadratic;
        break;
        case kSetIntpPrefs_AAMitchell:
        localPrefs.interpolator = _aamitchell;
        break;
        case kSetIntpPrefs_AALauczos2:
        localPrefs.interpolator = _aalanczos2;
        break;
        case kSetIntpPrefs_AALauczos3:
        localPrefs.interpolator = _aalanczos3;
        break;

        case kSetIntpPrefs_FastTNorm:
        localPrefs.fastStep = FAST_TRANSFORM_STEP_NONE;
        break;
        case kSetIntpPrefs_FastTMed:
        localPrefs.fastStep = FAST_TRANSFORM_STEP_MORPH;
        break;
        case kSetIntpPrefs_FastTFast:
        localPrefs.fastStep = FAST_TRANSFORM_STEP_NORMAL;
        break;
      }, 
      SetIntp ) 
}


#ifdef __Mac__
// can_resize = 0 if not.

int setSizePrefs( sPrefs *thePrefs, int can_resize ) 
{
	GenDialog( sPrefs, kSetSizePrefs_dlg, "Preferences",
			{
				CheckButton( kSetSizePrefs_Crop,  localPrefs.displayPart );																			
				CheckButton( kSetSizePrefs_SFile,  localPrefs.saveFile );
				if(!can_resize)
				{
					CheckButton( kSetSizePrefs_OpenF,  localPrefs.launchApp );
					CheckButton( kSetSizePrefs_NoAlpha,  localPrefs.noAlpha );
				}
			},;,;,
			{
				case kSetSizePrefs_Crop:
					localPrefs.displayPart = !localPrefs.displayPart;
					break;
				case kSetSizePrefs_SFile:
					localPrefs.saveFile = !localPrefs.saveFile;
					break;
				case kSetSizePrefs_OpenF:
					localPrefs.launchApp = !localPrefs.launchApp;
					break;
				case kSetSizePrefs_BrFile:
					{	
						fullPath path;
						if( SaveFileAs( &path, "Save Results as...", "ptools_result" ) == 0 )
						{
							memcpy( &(localPrefs.sFile), &path,  sizeof( fullPath )); 
						}
					}
					break;
				case kSetSizePrefs_SetInt:
					SetInterpolator( &localPrefs );
					break;
				case kSetSizePrefs_NoAlpha:
					localPrefs.noAlpha = !localPrefs.noAlpha;
					break;
			 }, SetSiz )
}


#endif

#ifdef __Win__

int setSizePrefs( sPrefs *thePrefs, int can_resize ) 
{
	GenDialog( sPrefs, kSetSizePrefs_dlg, "Preferences",
			{
				CheckButton( kSetSizePrefs_Crop,  	localPrefs.displayPart );																			
				CheckButton( kSetSizePrefs_SFile,  	localPrefs.saveFile );
				CheckButton( kSetSizePrefs_OpenF,  	localPrefs.launchApp );
				CheckButton( kSetSizePrefs_NoAlpha,  	localPrefs.noAlpha );
			},
			{
				SetLbl( kSetSizePrefs_FileName,  (char*)&localPrefs.sFile);
				SetLbl( kSetSizePrefs_AppName,   (char*)&localPrefs.lApp);
			},
			{
				GetLbl( kSetSizePrefs_FileName, (char*)&(localPrefs.sFile), 255);
				GetLbl( kSetSizePrefs_AppName, (char*)&(localPrefs.lApp), 255);
			},
			{
				case kSetSizePrefs_Crop:
					localPrefs.displayPart = !localPrefs.displayPart;
					break;
				case kSetSizePrefs_SFile:
					localPrefs.saveFile = !localPrefs.saveFile;
					break;
				case kSetSizePrefs_OpenF:
					localPrefs.launchApp = !localPrefs.launchApp;
					break;
				case kSetSizePrefs_BrFile:
					{	
						if( !SaveFileAs( &(localPrefs.sFile), "Save Results as...", "ptools_result" )  )
						{
							SetLbl( kSetSizePrefs_FileName,  (char*)&localPrefs.sFile);
						}
					}
					break;
				case kSetSizePrefs_SetInt:
					SetInterpolator( &localPrefs );
					break;
				case kSetSizePrefs_BrApp:
					{	
						if( !FindFile( &(localPrefs.lApp )  ) )
						{
							SetLbl( kSetSizePrefs_AppName,   (char*)&localPrefs.lApp);
						}
					}
					break;
				case kSetSizePrefs_NoAlpha:
					localPrefs.noAlpha = !localPrefs.noAlpha;
					break;
			 }, SetSiz )
}

#endif

#if  !__Mac__  && !__Win__	// Gimp version
// can_resize  ignored

int setSizePrefs( sPrefs *thePrefs, int can_resize ) 
{
	GenDialog( sPrefs, kSetSizePrefs_dlg, "Preferences",
			{
				CheckButton( kSetSizePrefs_Crop,  localPrefs.displayPart );																			
				CheckButton( kSetSizePrefs_SFile,  localPrefs.saveFile );
			},;,;,
			{
				case kSetSizePrefs_Crop:
					localPrefs.displayPart = !localPrefs.displayPart;
					break;
				case kSetSizePrefs_SFile:
					localPrefs.saveFile = !localPrefs.saveFile;
					break;
				case kSetSizePrefs_SetInt:
					SetInterpolator( &localPrefs );
					break;
			 }, SetSiz )
}

#endif


