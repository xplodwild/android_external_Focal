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


#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include "filter.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <WINDOWSX.H>


#ifndef SYS_WIN_H
#define SYS_WIN_H


#define		SetLbl( kRes, string)												\
			SetDlgItemText(hDlg, kRes, string); 


#define 	CheckButton( rNum, isChecked )										\
			CheckDlgButton (hDlg, rNum,	(isChecked));


#define		SetText( rNum, string, var )										\
			sprintf(numString, string, var);									\
			SetDlgItemText(hDlg, rNum, numString);


#define		GetLbl( rNum, string, length )									\
			GetDlgItemText(hDlg, rNum, string, length);							\

#define		GetText( rNum, string, varaddr )									\
			GetDlgItemText(hDlg, rNum, numString, 255);							\
			sscanf( numString, string, varaddr );

#define		GenDialog( VarType, DlgRes, Title,  SetControl,SText,			\
			GText, DlgAction, WinFunc)										\
																			\
			return( DialogBoxParam(hDllInstance,							\
							 (LPSTR) DlgRes,								\
							 (HWND)wndOwner,								\
							 (DLGPROC)WinFunc,								\
							 (LPARAM)thePrefs) == 1);						\
																			\
		}																	\
BOOL WINAPI WinFunc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)		\
		{																	\
			int			item, cmd;											\
			static 		VarType localPrefs, *thePrefs;						\
			char		numString[256];										\
																			\
			(numString);													\
			SetWindowOwner( hDlg );											\
			switch  (wMsg)													\
			{																\
				case  WM_INITDIALOG:										\
					thePrefs = (VarType*)lParam;							\
					memcpy(&localPrefs,thePrefs, sizeof( VarType ));		\
					SetWindowText(hDlg, Title);								\
					CenterDialog(hDlg);										\
					SetControl; SText;										\
				case WM_PAINT:												\
					return FALSE;											\
					break;													\
				case  WM_COMMAND:											\
	  				item = LOWORD(wParam);               					\
					cmd = HIWORD (wParam);									\
				switch  (item)												\
					{														\
						case 1: 											\
							if (cmd == BN_CLICKED)							\
							{												\
								GText;										\
								memcpy(thePrefs,&localPrefs, sizeof( VarType ));\
								EndDialog(hDlg, item); 						\
								return TRUE;								\
							}												\
							break;											\
						case 2: 											\
							if (cmd == BN_CLICKED)							\
							{												\
								EndDialog(hDlg, item);          			\
								return TRUE;								\
							}												\
							break;											\
						DlgAction;											\
						default:											\
							break;											\
					}														\
					SetControl;												\
					break; 													\
				default:													\
					return FALSE;											\
					break;													\
			} 																\
	return  TRUE;															\

typedef LPSTR *Handle;

#ifdef _DLL
extern HINSTANCE hDllInstance;
#else
#define hDllInstance 0
#endif
HWND wndOwner;

void SetWindowOwner(HWND Owner);
void CenterDialog(HWND hDlg);
BOOL GetPrefsFileName( char *prefname );
INT_PTR CALLBACK DispPrg			(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);       // Win32 Change
BOOL WINAPI SetLumOpt		(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI SetCPrefs		(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI SetRadOpt		(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);       // Win32 Change
BOOL WINAPI SetTiltOpt		(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);       // Win32 Change
BOOL WINAPI SetHorOpt		(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);       // Win32 Change
BOOL WINAPI SetVerOpt		(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);       // Win32 Change
BOOL WINAPI SetScOpt		(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);       // Win32 Change
BOOL WINAPI SetShOpt		(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);       // Win32 Change
BOOL WINAPI SetAdPrefs		(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);       // Win32 Change
BOOL WINAPI SetCrOpt		(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);       // Win32 Change
BOOL WINAPI SetPerspPrefs	(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);       // Win32 Change
BOOL WINAPI SetRem			(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);       // Win32 Change
BOOL WINAPI SetSiz			(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);       // Win32 Change
BOOL WINAPI SetIntp			(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);  
INT_PTR CALLBACK InfoPrg			(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);       // Win32 Change
BOOL WINAPI SetCutOpt		(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);       // Win32 Change
BOOL WINAPI SetPanOpt		(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);       // Win32 Change
BOOL WINAPI SetFrPrefs		(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam); 

//-------------------------- Dialog Resources-----------------------------------------

//-------------------------- Dialogs for remap  -------------------------------------------

#define		kSetRemapPrefs_dlg					"REMAP"	
#define		kSetRemapPrefs_InRect				350
#define		kSetRemapPrefs_InPano				351
#define		kSetRemapPrefs_InErect				352
#define		kSetRemapPrefs_InSphereCenter		354
#define		kSetRemapPrefs_InSphereTop			353
#define		kSetRemapPrefs_OutRect				356
#define		kSetRemapPrefs_OutPano				359
#define		kSetRemapPrefs_OutErect				358
#define		kSetRemapPrefs_OutSphereCenter		361
#define		kSetRemapPrefs_OutSphereTop			360
#define		kSetRemapPrefs_Hfov					362
#define		kSetRemapPrefs_Vfov					363
#define		kSetRemapPrefs_InMirror				355
#define		kSetRemapPrefs_OutMirror			357
#define     kSetRemapPrefs_SetPrefs				100

//-------------------------- Dialogs for perspective  -------------------------------------------

#define		kSetPerspectivePrefs_dlg			"PERSPECT"		
#define		kSetPerspectivePrefs_InRect			301		
#define		kSetPerspectivePrefs_InSphere		302
#define		kSetPerspectivePrefs_Degree			321
#define		kSetPerspectivePrefs_Points			320
#define		kSetPerspectivePrefs_Keep			325
#define		kSetPerspectivePrefs_X				305
#define		kSetPerspectivePrefs_Y				306
#define		kSetPerspectivePrefs_Gamma			307
#define		kSetPerspectivePrefs_Hfov			309
#define		kSetPerspectivePrefs_Width			311
#define     kSetPerspectivePrefs_Height			310
#define		kSetPerspectivePrefs_SetPrefs		100


// ------------------------- Dialogs for correct ------------------------------------------------

#define		kSetCorrectPrefs_dlg				"SETCPREFDLG"
#define		kSetCorrectPrefs_Save				6
#define		kSetCorrectPrefs_Load				5
#define		kSetCorrectPrefs_Radial				10
#define		kSetCorrectPrefs_RadialOption		40
#define		kSetCorrectPrefs_Horizontal			12
#define		kSetCorrectPrefs_HorizontalOption	42
#define		kSetCorrectPrefs_Vertical			11
#define		kSetCorrectPrefs_VerticalOption		41
#define		kSetCorrectPrefs_Shear				13
#define		kSetCorrectPrefs_ShearOption		43
#define		kSetCorrectPrefs_Scale				14
#define		kSetCorrectPrefs_ScaleOption		44
#define		kSetCorrectPrefs_Lum				26
#define		kSetCorrectPrefs_LumOpt				46
#define		kSetCorrectPrefs_SetPrefs			100
#define		kSetCorrectPrefs_CutFrame			27
#define		kSetCorrectPrefs_CutOpt         	47
#define		kSetCorrectPrefs_Fourier			28
#define		kSetCorrectPrefs_FourierOpt         48


#define 	kSetLumOptions_dlg					"THREEPARAM"
#define 	kSetLumOptions_Red					185
#define 	kSetLumOptions_Green				186
#define 	kSetLumOptions_Blue					187
#define 	kSetLumOptions_RedText				180
#define 	kSetLumOptions_GreenText			181
#define 	kSetLumOptions_BlueText				182



#define 	kSetRadialOptions_dlg				"RADIALOPT"
#define 	kSetRadialOptions_Slit				150
#define     kSetRadialOptions_Red0				113
#define     kSetRadialOptions_Red1				112
#define     kSetRadialOptions_Red2				111
#define     kSetRadialOptions_Red3				110
#define     kSetRadialOptions_Green0			123
#define     kSetRadialOptions_Green1			122
#define     kSetRadialOptions_Green2			121
#define     kSetRadialOptions_Green3			120
#define     kSetRadialOptions_Blue0				133
#define     kSetRadialOptions_Blue1				132
#define     kSetRadialOptions_Blue2				131
#define     kSetRadialOptions_Blue3				130
#define		kSetRadialOptions_radial			350
#define		kSetRadialOptions_vertical			351
#define		kSetRadialOptions_horizontal		352

#define 	kSetHorizontalOptions_dlg			"THREEPARAM"
#define    	kSetHorizontalOptions_Red			 185
#define    	kSetHorizontalOptions_Green			 186
#define    	kSetHorizontalOptions_Blue			 187

#define 	kSetShearOptions_dlg				"TWOPARAM"
#define 	kSetShearOptions_vname				 150
#define 	kSetShearOptions_vvar				 155
#define 	kSetShearOptions_hname				 151
#define 	kSetShearOptions_hvar				 156


//-------------------  Dialogs for Adjust --------------------------


#define		kSetAdjustPrefs_dlg					"ADJUSTPREFS"
#define		kSetAdjustPrefs_Insert				201
#define		kSetAdjustPrefs_Extract				200
#define		kSetAdjustPrefs_SetCtrlPts			205
#define		kSetAdjustPrefs_RunOptimizer		202
#define		kSetAdjustPrefs_Options  			206
#define     kSetAdjustPrefs_Script		    	207
#define     kSetAdjustPrefs_FindScript    		204
#define		kSetAdjustPrefs_SetOpt				203
#define		kSetAdjustPrefs_SetPrefs			100

#define		kSetCreateOptions_dlg				"ADJUSTOPT"	
#define		kSetCreateOptions_ImR				214
#define		kSetCreateOptions_ImP				215
#define		kSetCreateOptions_ImFf				216
#define		kSetCreateOptions_ImFc				217
#define		kSetCreateOptions_ImEq				219
#define		kSetCreateOptions_ImHfov			234
#define		kSetCreateOptions_ImW				228
#define		kSetCreateOptions_ImH				227
#define		kSetCreateOptions_Correct			100
#define		kSetCreateOptions_PRe				245
#define		kSetCreateOptions_PPa				246
#define		kSetCreateOptions_PSp				247
#define		kSetCreateOptions_PCu				248
#define		kSetCreateOptions_PHfov				240
#define		kSetCreateOptions_PWi				239
#define		kSetCreateOptions_PHe				238
#define		kSetCreateOptions_PSave				10
#define		kSetCreateOptions_TY				251
#define		kSetCreateOptions_TP				250
#define		kSetCreateOptions_TR				244
#define		kSetCreateOptions_SLoad				11
#define		kSetCreateOptions_SPaste			253
#define		kSetCreateOptions_SBlend			252
#define		kSetCreateOptions_SF				259
#define		kSetCreateOptions_SIm				254
#define		kSetCreateOptions_SBuf				255
#define		kSetCreateOptions_Sboth				256
#define		kSetCreateOptions_Snone				257
#define		kSetCreateOptions_Pan				101

#define kSetIntpPrefs_Poly 350
//#define kSetIntpPrefs_Sp16 351
#define kSetIntpPrefs_Sp36 352
#define kSetIntpPrefs_Sp64 353
#define kSetIntpPrefs_Sinc256 354
#define kSetIntpPrefs_AAHammering 360
#define kSetIntpPrefs_AAGaussian 361
#define kSetIntpPrefs_AAQuadratic 362
#define kSetIntpPrefs_AAMitchell 370
#define kSetIntpPrefs_AALauczos2 371
#define kSetIntpPrefs_AALauczos3 372
#define kSetIntpPrefs_FastTNorm 380
#define kSetIntpPrefs_FastTMed 381
#define kSetIntpPrefs_FastTFast 382
#define kSetIntpPrefs_Gamma  300
#define kSetIntpPrefs_SetIntp "INTPOL"

#define		kSetFourierOptions_dlg				"FOURIEROPT"
#define		kSetFourierOptions_PSDname			305
#define		kSetFourierOptions_findPSD			10
#define		kSetFourierOptions_addBlur			320
#define		kSetFourierOptions_remBlur			321
#define		kSetFourierOptions_internal			322
#define		kSetFourierOptions_custom			323
#define		kSetFourierOptions_findNFF			11
#define		kSetFourierOptions_filterfactor		307
#define		kSetFourierOptions_fourier_frame	308
#define		kSetFourierOptions_scale			324

#define		kSetSizePrefs_dlg					"SIZE_SRC"
#define		kSetSizePrefs_Crop					213
#define		kSetSizePrefs_SFile  				214
#define		kSetSizePrefs_OpenF  				215
#define		kSetSizePrefs_BrFile 				217
#define		kSetSizePrefs_SetInt				219
#define		kSetSizePrefs_AppName				216
#define		kSetSizePrefs_FileName				212
#define		kSetSizePrefs_BrApp					218
#define		kSetSizePrefs_NoAlpha				220


#endif


