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

#ifndef SYS_MAC_H
#define SYS_MAC_H			


#include "filter.h"
#include <Carbon/Carbon.h> // added by Kekus Digital
/*			   // commented by Kekus Digital
#include <Controls.h>
#include <TextUtils.h>
#include <Menus.h>
#include <Errors.h>
#include <StandardFile.h>
#include <ToolUtils.h>
#include <Devices.h>
#include <Folders.h>
#include <Events.h>
#include <LowMem.h>
#include <Dialogs.h>
#include <MacMemory.h>
#include <Files.h>
#include <LowMem.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <AppleEvents.h>
#include <Aliases.h>
#include <Files.h>
#include <MixedMode.h>
#include <CodeFragments.h>
#include <Fonts.h>
*/			// till here

#ifndef MAC_OS_X_VERSION_10_4
#define c2pstr(x)  MyCtoPStr(x) // added by Kekus Digital
#endif



#define		SetLbl( kRes, string)												\
			sprintf(numString,  "%s", string);									\
			GetDialogItem( dialog, kRes, &itemType, &myHandle, &itemRect);		\
			SetDialogItemText (myHandle, 	c2pstr( numString ) );

#define 	CheckButton( rNum, isChecked )										\
			GetDialogItem(dialog, rNum, &itemType, 	&myHandle, 	&itemRect);		\
			SetControlValue( (ControlHandle) myHandle, (isChecked) ) ;			


#define		SetText( rNum, string, var )										\
			sprintf(numString,  string, var);									\
			GetDialogItem( dialog, rNum, &itemType, &myHandle, &itemRect);		\
			SetDialogItemText (myHandle, 	c2pstr( numString ) );

#define		GetText( rNum, string, varaddr )									\
			GetDialogItem( dialog, rNum, &itemType, &myHandle, &itemRect);		\
			GetDialogItemText (myHandle, 	(unsigned char*)numString );		\
			sscanf(p2cstr((unsigned char*)numString), string, varaddr);				



#define		GenDialog( VarType,  DlgRes, Title, SetControl,SText, 				\
						GText, DlgAction, WinFunc)								\
																				\
			DialogPtr		dialog;												\
			Boolean			dialogDone = false;									\
			short			itemHit, itemType;									\
			Rect			itemRect;											\
			Handle			myHandle;											\
			char			numString[256];										\
			VarType			localPrefs;											\
																				\
			memcpy((char*)&localPrefs,(char*)thePrefs, sizeof(VarType));		\
																				\
			dialog = GetNewDialog( DlgRes , nil , (WindowPtr)-1L);				\
			strcpy( numString, Title);											\
			SetWTitle( GetDialogWindow( dialog), c2pstr( numString ));							\
			ShowWindow( dialog );												\
			SetPort( GetWindowPort(GetDialogWindow(dialog)) );													\
																				\
			SetDialogDefaultItem( dialog, ok );									\
			SetDialogCancelItem( dialog, cancel );								\
			SetDialogTracksCursor(dialog, true);								\
																				\
			SetControl;															\
			SText;																\
																				\
			while( !dialogDone ) 												\
			{																	\
				ModalDialog( nil, &itemHit);									\
				switch ( itemHit )												\
				{																\
			case ok:															\
			case cancel:														\
				dialogDone = true;												\
				break;															\
			DlgAction;															\
			default:															\
				break;															\
		}																		\
		SetControl;																\
	}																			\
	if ( itemHit == cancel )													\
	{																			\
		DisposeDialog( dialog );												\
		return FALSE;															\
	}																			\
	else																		\
	{																			\
		GText;																	\
		memcpy((char*)thePrefs, (char*)&localPrefs, sizeof(VarType));			\
		DisposeDialog( dialog );												\
		return TRUE;															\
	}																			\

			




//-------------------------- Dialog Resources-----------------------------------------

//-------------------------- Dialogs for remap  -------------------------------------------

#define		kSetRemapPrefs_dlg					310	
#define		kSetRemapPrefs_InRect				4
#define		kSetRemapPrefs_InPano				5
#define		kSetRemapPrefs_InErect				6
#define		kSetRemapPrefs_InSphereCenter		15
#define		kSetRemapPrefs_InSphereTop			7
#define		kSetRemapPrefs_OutRect				9
#define		kSetRemapPrefs_OutPano				10
#define		kSetRemapPrefs_OutErect				11
#define		kSetRemapPrefs_OutSphereCenter		16
#define		kSetRemapPrefs_OutSphereTop			12
#define		kSetRemapPrefs_Hfov					14
#define		kSetRemapPrefs_Vfov					18
#define		kSetRemapPrefs_InMirror				19
#define		kSetRemapPrefs_OutMirror			20
#define     kSetRemapPrefs_SetPrefs				21


//-------------------------- Dialogs for perspective  -------------------------------------------

#define		kSetPerspectivePrefs_dlg			320		
#define		kSetPerspectivePrefs_InRect			4		
#define		kSetPerspectivePrefs_InSphere		5
#define		kSetPerspectivePrefs_Degree			13
#define		kSetPerspectivePrefs_Points			14
#define		kSetPerspectivePrefs_Keep			22
#define		kSetPerspectivePrefs_X				10
#define		kSetPerspectivePrefs_Y				11
#define		kSetPerspectivePrefs_Gamma			16
#define		kSetPerspectivePrefs_Hfov			7
#define		kSetPerspectivePrefs_Width			18
#define     kSetPerspectivePrefs_Height			20
#define		kSetPerspectivePrefs_SetPrefs		23


// ------------------------- Dialogs for correct ------------------------------------------------

#define		kSetCorrectPrefs_dlg				300
#define		kSetCorrectPrefs_Save				3
#define		kSetCorrectPrefs_Load				4
#define		kSetCorrectPrefs_Radial				5
#define		kSetCorrectPrefs_RadialOption		6
#define		kSetCorrectPrefs_Horizontal			9
#define		kSetCorrectPrefs_HorizontalOption	10
#define		kSetCorrectPrefs_Vertical			7
#define		kSetCorrectPrefs_VerticalOption		8
#define		kSetCorrectPrefs_Shear				11
#define		kSetCorrectPrefs_ShearOption		12
#define		kSetCorrectPrefs_Scale				13
#define		kSetCorrectPrefs_ScaleOption		14
#define		kSetCorrectPrefs_Lum				15
#define		kSetCorrectPrefs_LumOpt				16
#define		kSetCorrectPrefs_SetPrefs			17
#define		kSetCorrectPrefs_CutFrame			18
#define		kSetCorrectPrefs_CutOpt         	19
#define		kSetCorrectPrefs_Fourier			20
#define		kSetCorrectPrefs_FourierOpt         21


#define 	kSetLumOptions_dlg					302
#define 	kSetLumOptions_Red					6
#define 	kSetLumOptions_Green				7
#define 	kSetLumOptions_Blue					8
#define 	kSetLumOptions_RedText				3
#define 	kSetLumOptions_GreenText			4
#define 	kSetLumOptions_BlueText				5


#define 	kSetRadialOptions_dlg				301
#define 	kSetRadialOptions_radial			22
#define 	kSetRadialOptions_vertical			23
#define 	kSetRadialOptions_horizontal		24
#define     kSetRadialOptions_Red0				10
#define     kSetRadialOptions_Red1				11
#define     kSetRadialOptions_Red2				12
#define     kSetRadialOptions_Red3				13
#define     kSetRadialOptions_Green0			14
#define     kSetRadialOptions_Green1			15
#define     kSetRadialOptions_Green2			16
#define     kSetRadialOptions_Green3			17
#define     kSetRadialOptions_Blue0				18
#define     kSetRadialOptions_Blue1				19
#define     kSetRadialOptions_Blue2				20
#define     kSetRadialOptions_Blue3				21

#define 	kSetHorizontalOptions_dlg			302
#define    	kSetHorizontalOptions_Red			 6
#define    	kSetHorizontalOptions_Green			 7
#define    	kSetHorizontalOptions_Blue			 8

#define 	kSetShearOptions_dlg				303
#define 	kSetShearOptions_vname				 3
#define 	kSetShearOptions_vvar				 4
#define 	kSetShearOptions_hname				 5
#define 	kSetShearOptions_hvar				 6

#define		kSetFourierOptions_dlg				450
#define		kSetFourierOptions_PSDname			5
#define		kSetFourierOptions_findPSD			6
#define		kSetFourierOptions_addBlur			7
#define		kSetFourierOptions_remBlur			8
#define		kSetFourierOptions_internal			11
#define		kSetFourierOptions_custom			12
#define		kSetFourierOptions_findNFF			13
#define		kSetFourierOptions_filterfactor		15
#define		kSetFourierOptions_fourier_frame	17
#define		kSetFourierOptions_scale			18


//-------------------  Dialogs for Adjust --------------------------


#define		kSetAdjustPrefs_dlg					330	
#define		kSetAdjustPrefs_Insert				7
#define		kSetAdjustPrefs_Extract				10
#define		kSetAdjustPrefs_SetCtrlPts			9
#define		kSetAdjustPrefs_RunOptimizer		5
#define		kSetAdjustPrefs_Options  			3
#define     kSetAdjustPrefs_Script		    	11
#define     kSetAdjustPrefs_FindScript    		 6
#define		kSetAdjustPrefs_SetOpt				4
#define		kSetAdjustPrefs_SetPrefs			8

#define		kSetCreateOptions_dlg				331		
#define		kSetCreateOptions_ImR				4
#define		kSetCreateOptions_ImP				14
#define		kSetCreateOptions_ImFf				5
#define		kSetCreateOptions_ImFc				20
#define		kSetCreateOptions_ImEq				51
#define		kSetCreateOptions_ImHfov			6
#define		kSetCreateOptions_ImW				21
#define		kSetCreateOptions_ImH				22
#define		kSetCreateOptions_Correct			43
#define		kSetCreateOptions_PRe				26
#define		kSetCreateOptions_PPa				27
#define		kSetCreateOptions_PSp				28
#define		kSetCreateOptions_PHfov				30
#define		kSetCreateOptions_PWi				32
#define		kSetCreateOptions_PHe				34
#define		kSetCreateOptions_PSave				44
#define		kSetCreateOptions_TY				9
#define		kSetCreateOptions_TP				11
#define		kSetCreateOptions_TR				13
#define		kSetCreateOptions_SLoad				37
#define		kSetCreateOptions_SPaste			39
#define		kSetCreateOptions_SBlend			40
#define		kSetCreateOptions_SF				42
#define		kSetCreateOptions_SIm				46
#define		kSetCreateOptions_SBuf				47
#define		kSetCreateOptions_Sboth				48
#define		kSetCreateOptions_Snone				49


#define kSetIntpPrefs_Gamma  1
#define kSetIntpPrefs_Poly 2
//#define kSetIntpPrefs_Sp16 2
#define kSetIntpPrefs_Sp36 3
#define kSetIntpPrefs_Sp64 4
#define kSetIntpPrefs_Sinc256 5
#define kSetIntpPrefs_AAHammering 6
#define kSetIntpPrefs_AAGaussian 7
#define kSetIntpPrefs_AAQuadratic 8
#define kSetIntpPrefs_AAMitchell 9
#define kSetIntpPrefs_AALauczos2 10
#define kSetIntpPrefs_AALauczos3 11
#define kSetIntpPrefs_FastTNorm 12
#define kSetIntpPrefs_FastTMed 13
#define kSetIntpPrefs_FastTFast 14
#define kSetIntpPrefs_SetIntp 400


#define		kSetSizePrefs_dlg					(can_resize ? 350 : 360)
#define		kSetSizePrefs_Crop					4
#define		kSetSizePrefs_SFile  				5
#define		kSetSizePrefs_OpenF  				8
#define		kSetSizePrefs_BrFile 				7
#define		kSetSizePrefs_SetInt				6
#define		kSetSizePrefs_NoAlpha				9


void    		open_selection( FSSpec *filespec );

unsigned char *MyCtoPStr(char *x); // added by Kekus Digital

#endif

