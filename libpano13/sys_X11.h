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

#ifndef SYS_X11_H
#define SYS_X11_H

#include <libgimp/gimp.h>
#include <gtk/gtk.h>


#include "filter.h"		

#define ENTRY_WIDTH 30


struct wdata
{
	GtkWidget 	*widg;
	gpointer 	data;
};

typedef struct wdata wdata;

extern int ptools_isOK;

void pt_widget_destroy			(GtkWidget *widget);
void pt_main_destroy			(GtkWidget *widget);

void DispPrg		(GtkWidget *widget, gpointer data);        
void SetLumOpt		(GtkWidget *widget, gpointer data); 
void SetCPrefs		(GtkWidget *widget, gpointer data); 
void SetRadOpt		(GtkWidget *widget, gpointer data);        
void SetHorOpt		(GtkWidget *widget, gpointer data);        
void SetVerOpt		(GtkWidget *widget, gpointer data);        
void SetScOpt		(GtkWidget *widget, gpointer data);        
void SetShOpt		(GtkWidget *widget, gpointer data);        
void SetAdPrefs		(GtkWidget *widget, gpointer data);        
void SetCrOpt		(GtkWidget *widget, gpointer data);        
void SetPerspPrefs	(GtkWidget *widget, gpointer data);        
void SetRem			(GtkWidget *widget, gpointer data);        
void SetSiz			(GtkWidget *widget, gpointer data);        
void SetIntp		(GtkWidget *widget, gpointer data);   
void InfoPrg		(GtkWidget *widget, gpointer data);        
void SetCutOpt		(GtkWidget *widget, gpointer data);        
void SetPanOpt		(GtkWidget *widget, gpointer data); 
void SetFrPrefs		(GtkWidget *widget, gpointer data); 


void ptool_load_callback		(GtkWidget *widget, gpointer data);
void ptool_save_callback		(GtkWidget *widget, gpointer data);

void pt_set_size_source			(GtkWidget *widget, gpointer data);
void pt_load_corr				(GtkWidget *widget, gpointer data);
void pt_save_corr				(GtkWidget *widget, gpointer data);
void pt_set_file				(GtkWidget *widget, gpointer data);
void pt_find_file				(GtkWidget *widget, gpointer data);


#define 	CheckButton( kRes, isChecked )											\
			gtk_toggle_button_set_state( GTK_TOGGLE_BUTTON (dp[kRes]), isChecked ); \
			isBool[kRes]  = ( (isChecked) ? 1 : 2 );
			
#define		SetText( kRes, string, var )											\
			sprintf( numString,  string, var);										\
    		gtk_entry_set_text(GTK_ENTRY(dp[kRes]), numString);							

#define		SetLbl( kRes, string)													\
			gtk_label_set(GTK_LABEL(dp[kRes]), string);	
			

#define		GetText( kRes, string, varaddr )										\
			sscanf( gtk_entry_get_text(GTK_ENTRY(dp[kRes])), string, varaddr);				


#define MAKE_TABLE( hor, ver )														\
    table = gtk_table_new(hor, ver, FALSE); 										\
    gtk_container_border_width(GTK_CONTAINER(table), 10);							\
    gtk_container_add(GTK_CONTAINER(frame), table);									\
    gtk_widget_show(table);															\


#define  MAKE_TEXT_WID( kRes , left, right, top, bottom ) 							\
    dp[kRes] = gtk_entry_new( );													\
    gtk_widget_set_usize( dp[kRes], ENTRY_WIDTH, 0);								\
    gtk_table_attach(GTK_TABLE(table), dp[kRes],left, right, top, bottom,			\
        GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);										\
    gtk_widget_show(dp[kRes]);

#define  MAKE_FTEXT_WID( kRes , left, right, top, bottom ) 							\
    dp[kRes] = gtk_entry_new( );													\
    gtk_widget_set_usize( dp[kRes], ENTRY_WIDTH, 0);								\
    gtk_table_attach(GTK_TABLE(ftable), dp[kRes],left, right, top, bottom,			\
        GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);										\
    gtk_widget_show(dp[kRes]);



#define  MAKE_LABEL_WID( kRes , left, right, top, bottom ) 							\
	dp[kRes] = gtk_label_new("");													\
    gtk_table_attach(GTK_TABLE(table),dp[kRes],left, right, top, bottom,     		\
        GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);										\
    gtk_widget_show(dp[kRes]);		

#define  MAKE_TEXTLABEL( text , table, left, right, top, bottom ) 					\
	label = gtk_label_new(text);													\
    gtk_table_attach(GTK_TABLE(table),label,left, right, top, bottom,     			\
        GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);										\
    gtk_widget_show(label);		



#define MAKE_CHECK_BOX( kRes, label, left, right, top, bottom ) 					\
	dp[kRes] = gtk_check_button_new_with_label ( label );							\
    gtk_table_attach(GTK_TABLE(table),dp[kRes],left, right, top, bottom,     		\
        GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);										\
    gtk_widget_show(dp[kRes]);		

#define MAKE_FCHECK_BOX( kRes, label, left, right, top, bottom ) 					\
	dp[kRes] = gtk_check_button_new_with_label ( label );							\
    gtk_table_attach(GTK_TABLE(ftable),dp[kRes],left, right, top, bottom,     		\
        GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);										\
    gtk_widget_show(dp[kRes]);		

#define MAKE_RADIO_BUTTON(  kRes, label )											\
   dp[kRes] = gtk_radio_button_new_with_label( group, label );						\
   gtk_box_pack_start (GTK_BOX (box), dp[kRes], TRUE, TRUE, 0);						\
   group = gtk_radio_button_group (GTK_RADIO_BUTTON (dp[kRes]));					\
   gtk_widget_show (dp[kRes]);

#define INSERT_BUTTON(  left, right, top, bottom ) 									\
   gtk_table_attach(GTK_TABLE(table), button, left, right, top, bottom,       		\
        GTK_FILL | GTK_EXPAND, GTK_FILL, 1, 1);										

#define INSERT_FBUTTON(  left, right, top, bottom ) 								\
   gtk_table_attach(GTK_TABLE(ftable), button, left, right, top, bottom,       		\
        GTK_FILL | GTK_EXPAND, GTK_FILL, 1, 1);										



#define   NUMRES	30


#define		GenCallBack( VarType,  DlgRes, Title, SetControl,SText, 			\
						GText, DlgAction, WinFunc)								\
void WinFunc (GtkWidget *widget, gpointer data)									\
{																				\
    static GtkWidget 		*dlg;												\
    GtkWidget 				*table, *button, *frame, *box, *label;				\
    GSList 					*group = NULL;										\
    static int 				isBool[ NUMRES ];									\
    int i;																		\
   	static GtkWidget*		dp[ NUMRES ];										\
	static VarType 			localPrefs, *thePrefs;								\
	char numString[32];															\
	static int isMain;															\
																				\
	if( data != NULL )															\
	{																			\
		isMain = ( widget == NULL );											\
		thePrefs = (VarType*)data;												\
		memcpy( &localPrefs, thePrefs, sizeof( VarType ));						\
    	for(i=0; i<NUMRES; i++) isBool[i] = FALSE;								\
    																			\
																				\
    	dlg = gtk_dialog_new();													\
    	gtk_window_set_title(GTK_WINDOW(dlg), Title);							\
    	gtk_window_position(GTK_WINDOW(dlg), GTK_WIN_POS_MOUSE);				\
    	gtk_signal_connect_object(GTK_OBJECT(dlg), "destroy",					\
        	(GtkSignalFunc) (isMain ? pt_main_destroy : pt_widget_destroy), 	\
        	GTK_OBJECT (dlg) );													\
																				\
    	frame = gtk_frame_new("Parameter Settings");							\
    	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);		\
    	gtk_container_border_width(GTK_CONTAINER(frame), 10);					\
    	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox), frame, TRUE, TRUE, 0);\
																				\
  		button = gtk_button_new_with_label ("OK");								\
  		GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);							\
  		gtk_signal_connect (GTK_OBJECT (button), "clicked",						\
                      (GtkSignalFunc) WinFunc,									\
                      NULL );													\
  		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), 			\
  				button, TRUE, TRUE, 0);											\
  		gtk_widget_grab_default (button);										\
  		gtk_widget_show (button);												\
																				\
  		button = gtk_button_new_with_label ("Cancel");							\
  		GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);							\
  		gtk_signal_connect_object (GTK_OBJECT (button), "clicked",				\
        (GtkSignalFunc) (isMain ? pt_main_destroy : pt_widget_destroy),			\
                             GTK_OBJECT (dlg) );								\
  		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), 			\
  				button, TRUE, TRUE, 0);											\
  		gtk_widget_show (button);												\
    	gtk_widget_show(frame);													\
  																				\
  		DlgRes																	\
 		SText																	\
 		SetControl																\
																				\
    	gtk_widget_show(dlg);													\
    }																			\
	else																		\
	{																			\
		GText																	\
																				\
 		for(i=0; i < NUMRES; i++)												\
 		{																		\
 			if( isBool[i] == 1 )												\
 			{																	\
 				switch(i)														\
 					DlgAction													\
 			}																	\
 		}																		\
 		for(i=0; i < NUMRES; i++)												\
 		{																		\
 			if( isBool[i] && (GTK_TOGGLE_BUTTON (dp[i])->active) )				\
 			{																	\
 				switch(i)														\
 					DlgAction													\
 			}																	\
 		}																		\
 		memcpy( thePrefs, &localPrefs, sizeof( VarType) );						\
 		ptools_isOK =TRUE;														\
 		if(isMain)  gtk_main_quit();											\
 		else        gtk_widget_destroy( dlg );									\
	}																			\





#define		GenDialog( VarType,  DlgRes, Title, SetControl,SText, 				\
						GText, DlgAction, WinFunc)								\
    gchar 			**argv;														\
    gint 			argc;														\
																				\
   	argc 			= 1;														\
    argv 			= g_new(gchar *, 1);										\
    argv[0] 		= g_strdup("dummy");										\
																				\
    gtk_init(&argc, &argv);														\
    gtk_rc_parse(gimp_gtkrc());													\
																				\
	WinFunc( NULL, (gpointer) thePrefs );										\
 																				\
    gtk_main();																	\
    gdk_flush();																\
    return ptools_isOK;															\
}																				\
	GenCallBack( VarType,  DlgRes, Title, SetControl,SText, 					\
						GText, DlgAction, WinFunc)								\



//-------------------------- Dialog Resources-----------------------------------------

//-------------------------- Dialogs for remap  -------------------------------------------

#define		kSetRemapPrefs_InRect				0
#define		kSetRemapPrefs_InPano				1
#define		kSetRemapPrefs_InErect				2
#define		kSetRemapPrefs_InSphereCenter		3
#define		kSetRemapPrefs_InSphereTop			4
#define		kSetRemapPrefs_OutRect				5
#define		kSetRemapPrefs_OutPano				6
#define		kSetRemapPrefs_OutErect				7
#define		kSetRemapPrefs_OutSphereCenter		8
#define		kSetRemapPrefs_OutSphereTop			9
#define		kSetRemapPrefs_Hfov					10
#define		kSetRemapPrefs_Vfov					11
#define		kSetRemapPrefs_InMirror				12
#define		kSetRemapPrefs_OutMirror			13
#define     kSetRemapPrefs_SetPrefs				14
#define		kSetRemapPrefs_dlg						\
	{																	\
    MAKE_TABLE(8, 4) 													\
																		\
    MAKE_TEXTLABEL("HFoV:", 			table, 0, 1, 7, 8);				\
    MAKE_TEXTLABEL("VFoV:", 			table, 2, 3, 7, 8);				\
    frame = gtk_frame_new("From:");										\
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);	\
    	gtk_container_border_width(GTK_CONTAINER(frame), 2);			\
    gtk_table_attach(GTK_TABLE(table), frame, 0, 2, 1, 7,				\
        GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);							\
    box = gtk_vbox_new(FALSE, 6);										\
    gtk_container_border_width(GTK_CONTAINER(box), 5);					\
    gtk_container_add(GTK_CONTAINER(frame), box);						\
  	gtk_widget_show (box);												\
  	gtk_widget_show (frame);											\
    group = NULL;														\
    MAKE_RADIO_BUTTON( kSetRemapPrefs_InRect,        "Normal" )			\
    MAKE_RADIO_BUTTON( kSetRemapPrefs_InPano,        "QTVR" )			\
    MAKE_RADIO_BUTTON( kSetRemapPrefs_InErect,       "PSphere" )		\
    MAKE_RADIO_BUTTON( kSetRemapPrefs_InSphereTop,   "Fisheye hor." )	\
    MAKE_RADIO_BUTTON( kSetRemapPrefs_InSphereCenter,"Fisheye vert.") 	\
    MAKE_RADIO_BUTTON( kSetRemapPrefs_InMirror, 	 "Convex Mirror")	\
                                       									\
    frame = gtk_frame_new("To:");										\
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);	\
    	gtk_container_border_width(GTK_CONTAINER(frame), 2);			\
    gtk_table_attach(GTK_TABLE(table), frame, 2, 4, 1, 7,				\
        GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);							\
    box = gtk_vbox_new(FALSE, 6);										\
    gtk_container_border_width(GTK_CONTAINER(box), 5);					\
    gtk_container_add(GTK_CONTAINER(frame), box);						\
  	gtk_widget_show (box);												\
  	gtk_widget_show (frame);											\
    group = NULL;														\
    MAKE_RADIO_BUTTON( kSetRemapPrefs_OutRect,        "Normal" )		\
    MAKE_RADIO_BUTTON( kSetRemapPrefs_OutPano,        "QTVR" )			\
    MAKE_RADIO_BUTTON( kSetRemapPrefs_OutErect,       "PSphere" )		\
    MAKE_RADIO_BUTTON( kSetRemapPrefs_OutSphereTop,   "Fisheye hor." )	\
    MAKE_RADIO_BUTTON( kSetRemapPrefs_OutSphereCenter,"Fisheye vert.")	\
    MAKE_RADIO_BUTTON( kSetRemapPrefs_OutMirror, 	  "Convex Mirror")	\
                                       									\
 	MAKE_TEXT_WID( kSetRemapPrefs_Hfov , 		1, 2, 7, 8 );			\
	MAKE_TEXT_WID( kSetRemapPrefs_Vfov , 		3, 4, 7, 8 );			\
    button = gtk_button_new_with_label ("Prefs");						\
	INSERT_BUTTON( 3, 4, 0, 1 );										\
  	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) SetSiz,					\
                             (gpointer) gsPrPtr);						\
    gtk_widget_show(button);											\
																		\
    }
	


//-------------------------- Dialogs for perspective  -------------------------------------------

#define		kSetPerspectivePrefs_InRect			0		
#define		kSetPerspectivePrefs_InSphere		1
#define		kSetPerspectivePrefs_Degree			2
#define		kSetPerspectivePrefs_Points			3
#define		kSetPerspectivePrefs_Keep			4
#define		kSetPerspectivePrefs_X				5
#define		kSetPerspectivePrefs_Y				6
#define		kSetPerspectivePrefs_Gamma			7
#define		kSetPerspectivePrefs_Hfov			8
#define		kSetPerspectivePrefs_Width			9
#define     kSetPerspectivePrefs_Height			10
#define		kSetPerspectivePrefs_SetPrefs		11
#define		kSetPerspectivePrefs_dlg			\
	{																	\
    MAKE_TABLE(6, 4) 													\
																		\
    MAKE_TEXTLABEL("Format:", 		table, 0, 1, 0, 1);					\
    MAKE_TEXTLABEL("Turn to:", 		table, 0, 1, 1, 2);					\
    MAKE_TEXTLABEL("Horizontal:", 	table, 1, 2, 1, 2);					\
    MAKE_TEXTLABEL("Vertical:", 	table, 1, 2, 2, 3);					\
    MAKE_TEXTLABEL("Rotate:", 		table, 0, 1, 3, 4);					\
    MAKE_TEXTLABEL("Size:", 		table, 0, 1, 4, 5);					\
    MAKE_TEXTLABEL("HFoV:", 		table, 2, 3, 3, 4);					\
    MAKE_TEXTLABEL("Width:", 		table, 2, 3, 4, 5);					\
    MAKE_TEXTLABEL("Height:", 		table, 2, 3, 5, 6);					\
																		\
    box = gtk_hbox_new(FALSE, 2);										\
    gtk_container_border_width(GTK_CONTAINER(box), 5);					\
    gtk_table_attach(GTK_TABLE(table), box, 1, 3, 0, 1,					\
        GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);							\
  	gtk_widget_show (box);												\
    group = NULL;														\
    MAKE_RADIO_BUTTON( kSetPerspectivePrefs_InSphere, "Fisheye" )		\
    MAKE_RADIO_BUTTON( kSetPerspectivePrefs_InRect,   "Rectilinear" )	\
																		\
	MAKE_TEXT_WID( kSetPerspectivePrefs_X , 		2, 3, 1, 2 );		\
	MAKE_TEXT_WID( kSetPerspectivePrefs_Y , 		2, 3, 2, 3 );		\
	MAKE_TEXT_WID( kSetPerspectivePrefs_Gamma , 	1, 2, 3, 4 );		\
	MAKE_TEXT_WID( kSetPerspectivePrefs_Hfov , 		3, 4, 3, 4 );		\
	MAKE_TEXT_WID( kSetPerspectivePrefs_Width, 		3, 4, 4, 5 );		\
	MAKE_TEXT_WID( kSetPerspectivePrefs_Height , 	3, 4, 5, 6 );		\
    box = gtk_vbox_new(FALSE, 2);										\
    gtk_container_border_width(GTK_CONTAINER(box), 5);					\
    gtk_table_attach(GTK_TABLE(table), box, 3, 4, 1, 3,					\
        GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);							\
  	gtk_widget_show (box);												\
    group = NULL;														\
    MAKE_RADIO_BUTTON( kSetPerspectivePrefs_Points, "Pixels" )			\
    MAKE_RADIO_BUTTON( kSetPerspectivePrefs_Degree, "Degrees" )			\
																		\
    button = gtk_button_new_with_label ("Prefs");						\
	INSERT_BUTTON( 3, 4, 0, 1 )								     		\
  	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) SetSiz,					\
                             (gpointer) gsPrPtr);						\
    gtk_widget_show(button);											\
																		\
    button = gtk_button_new_with_label ("Source");						\
    INSERT_BUTTON( 1, 2, 4, 5)											\
  	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) pt_set_size_source,		\
                             (gpointer) dp );							\
    gtk_widget_show(button);											\
    }

// ------------------------- Dialogs for correct ------------------------------------------------

#define		kSetCorrectPrefs_Save				0
#define		kSetCorrectPrefs_Load				1
#define		kSetCorrectPrefs_Radial				2
#define		kSetCorrectPrefs_RadialOption		3
#define		kSetCorrectPrefs_Horizontal			4
#define		kSetCorrectPrefs_HorizontalOption	5
#define		kSetCorrectPrefs_Vertical			6
#define		kSetCorrectPrefs_VerticalOption		7
#define		kSetCorrectPrefs_Shear				8
#define		kSetCorrectPrefs_ShearOption		9
#define		kSetCorrectPrefs_Scale				10
#define		kSetCorrectPrefs_ScaleOption		11
#define		kSetCorrectPrefs_Lum				12
#define		kSetCorrectPrefs_LumOpt				13
#define		kSetCorrectPrefs_SetPrefs			14
#define		kSetCorrectPrefs_CutFrame			15
#define		kSetCorrectPrefs_CutOpt         	16
#define		kSetCorrectPrefs_Fourier			17
#define		kSetCorrectPrefs_FourierOpt         18
#define		kSetCorrectPrefs_dlg					\
	{																	\
  	button = gtk_button_new_with_label ("Load");						\
  	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);						\
  	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) ptool_load_callback,		\
                             (gpointer) &localPrefs );					\
  	gtk_box_pack_end (GTK_BOX (GTK_DIALOG (dlg)->action_area), 			\
  				button, TRUE, TRUE, 0);									\
  	gtk_widget_show (button);											\
  	button = gtk_button_new_with_label ("Save");						\
  	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);						\
  	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) ptool_save_callback,		\
                             (gpointer) &localPrefs );					\
  	gtk_box_pack_end (GTK_BOX (GTK_DIALOG (dlg)->action_area), 			\
  				button, TRUE, TRUE, 0);									\
  	gtk_widget_show (button);											\
    MAKE_TABLE(8, 3); 													\
																		\
	MAKE_CHECK_BOX( kSetCorrectPrefs_Radial,    "Radial",            0, 1, 0, 1 ) 	\
	MAKE_CHECK_BOX( kSetCorrectPrefs_Vertical,  "Vertical Shift", 	 0, 1, 1, 2 ) 	\
	MAKE_CHECK_BOX( kSetCorrectPrefs_Horizontal,"Horizontal Shift",  0, 1, 2, 3 ) 	\
	MAKE_CHECK_BOX( kSetCorrectPrefs_Shear,     "Shear",             0, 1, 3, 4 ) 	\
	MAKE_CHECK_BOX( kSetCorrectPrefs_Scale,     "Scale",             0, 1, 4, 5 ) 	\
	MAKE_CHECK_BOX( kSetCorrectPrefs_Lum,       "Radial Luminance",  0, 1, 5, 6 ) 	\
	MAKE_CHECK_BOX( kSetCorrectPrefs_CutFrame,  "Cut Frame",         0, 1, 6, 7 ) 	\
	MAKE_CHECK_BOX( kSetCorrectPrefs_Fourier,  	"Fourier Filter",    0, 1, 7, 8 ) 	\
																		\
    button = gtk_button_new_with_label ("Options");						\
    INSERT_BUTTON( 1, 2, 0, 1)								      		\
  	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) SetRadOpt,					\
                             (gpointer) &localPrefs);					\
    gtk_widget_show(button);											\
																		\
    button = gtk_button_new_with_label ("Options");						\
    INSERT_BUTTON( 1, 2, 1, 2)								     		\
  	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) SetVerOpt,					\
                             (gpointer) &localPrefs);					\
    gtk_widget_show(button);											\
																		\
    button = gtk_button_new_with_label ("Options");						\
    INSERT_BUTTON( 1, 2, 2, 3)								     		\
  	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) SetHorOpt,					\
                             (gpointer) &localPrefs);					\
    gtk_widget_show(button);											\
																		\
    button = gtk_button_new_with_label ("Options");						\
    INSERT_BUTTON( 1, 2, 3, 4)								     		\
  	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) SetShOpt,					\
                             (gpointer) &localPrefs);					\
    gtk_widget_show(button);											\
																		\
    button = gtk_button_new_with_label ("Options");						\
    INSERT_BUTTON( 1, 2, 4, 5)								     		\
   	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) SetScOpt,					\
                             (gpointer) &localPrefs);					\
    gtk_widget_show(button);											\
																		\
    button = gtk_button_new_with_label ("Options");						\
    INSERT_BUTTON( 1, 2, 5, 6)								      		\
   	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) SetLumOpt,					\
                             (gpointer) &localPrefs);					\
    gtk_widget_show(button);											\
																		\
    button = gtk_button_new_with_label ("Options");						\
    INSERT_BUTTON( 1, 2, 6, 7)								      		\
  	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) SetCutOpt,					\
                             (gpointer) &localPrefs);					\
    gtk_widget_show(button);											\
																		\
    button = gtk_button_new_with_label ("Options");						\
    INSERT_BUTTON( 1, 2, 7, 8)								      		\
  	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) SetFrPrefs,				\
                             (gpointer) &localPrefs);					\
    gtk_widget_show(button);											\
																		\
																		\
    button = gtk_button_new_with_label ("Prefs");						\
    INSERT_BUTTON( 2, 3, 0, 1)								      		\
   	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) SetSiz,					\
                             (gpointer) gsPrPtr);						\
    gtk_widget_show(button);											\
    }
	

#define 	kSetLumOptions_Red					0
#define 	kSetLumOptions_Green				1
#define 	kSetLumOptions_Blue					2
#define 	kSetLumOptions_dlg					\
	{																	\
    MAKE_TABLE(3, 2); 													\
																		\
    MAKE_TEXTLABEL("Red:", 			table, 0, 1, 0, 1);					\
    MAKE_TEXTLABEL("Green:", 		table, 0, 1, 1, 2);					\
    MAKE_TEXTLABEL("Blue:", 		table, 0, 1, 2, 3);					\
 																		\
																		\
	MAKE_TEXT_WID( kSetLumOptions_Red , 		1, 2, 0, 1 );			\
	MAKE_TEXT_WID( kSetLumOptions_Green , 		1, 2, 1, 2 );			\
	MAKE_TEXT_WID( kSetLumOptions_Blue , 		1, 2, 2, 3 );			\
    }

#define 	kSetRadialOptions_Slit				0
#define     kSetRadialOptions_Red0				1
#define     kSetRadialOptions_Red1				2
#define     kSetRadialOptions_Red2				3
#define     kSetRadialOptions_Red3				4
#define     kSetRadialOptions_Green0			5
#define     kSetRadialOptions_Green1			6
#define     kSetRadialOptions_Green2			7
#define     kSetRadialOptions_Green3			8
#define     kSetRadialOptions_Blue0				9
#define     kSetRadialOptions_Blue1				10
#define     kSetRadialOptions_Blue2				11
#define     kSetRadialOptions_Blue3				12
#define 	kSetRadialOptions_dlg				\
	{																	\
    MAKE_TABLE(5, 5); 													\
																		\
    MAKE_TEXTLABEL("Red:", 			table, 0, 1, 1, 2);					\
    MAKE_TEXTLABEL("Green:", 		table, 0, 1, 2, 3);					\
    MAKE_TEXTLABEL("Blue:", 		table, 0, 1, 3, 4);					\
    MAKE_TEXTLABEL("a * r^4 +", 	table, 1, 2, 0, 1);					\
    MAKE_TEXTLABEL("b * r^3 +", 	table, 2, 3, 0, 1);					\
    MAKE_TEXTLABEL("c * r^2 +", 	table, 3, 4, 0, 1);					\
    MAKE_TEXTLABEL("d * r", 		table, 4, 5, 0, 1);					\
 																		\
																		\
	MAKE_TEXT_WID( kSetRadialOptions_Red0 , 	4, 5, 1, 2 );			\
	MAKE_TEXT_WID( kSetRadialOptions_Red1 , 	3, 4, 1, 2 );			\
	MAKE_TEXT_WID( kSetRadialOptions_Red2 , 	2, 3, 1, 2 );			\
	MAKE_TEXT_WID( kSetRadialOptions_Red3 , 	1, 2, 1, 2 );			\
	MAKE_TEXT_WID( kSetRadialOptions_Green0 , 	4, 5, 2, 3 );			\
	MAKE_TEXT_WID( kSetRadialOptions_Green1 , 	3, 4, 2, 3 );			\
	MAKE_TEXT_WID( kSetRadialOptions_Green2 , 	2, 3, 2, 3 );			\
	MAKE_TEXT_WID( kSetRadialOptions_Green3 , 	1, 2, 2, 3 );			\
	MAKE_TEXT_WID( kSetRadialOptions_Blue0 , 	4, 5, 3, 4 );			\
	MAKE_TEXT_WID( kSetRadialOptions_Blue1 , 	3, 4, 3, 4 );			\
	MAKE_TEXT_WID( kSetRadialOptions_Blue2 , 	2, 3, 3, 4 );			\
	MAKE_TEXT_WID( kSetRadialOptions_Blue3 , 	1, 2, 3, 4 );			\
																		\
    MAKE_CHECK_BOX( kSetRadialOptions_Slit,"Scanning Slit", 0, 2, 4, 5) \
    }


#define    	kSetHorizontalOptions_Red			 0
#define    	kSetHorizontalOptions_Green			 1
#define    	kSetHorizontalOptions_Blue			 2
#define 	kSetLumOptions_RedText				 3
#define 	kSetLumOptions_GreenText			 4
#define 	kSetLumOptions_BlueText				 5
#define 	kSetHorizontalOptions_dlg			\
	{																	\
    MAKE_TABLE(3, 2); 													\
																		\
	MAKE_LABEL_WID( kSetLumOptions_RedText ,   0, 1, 0, 1);				\
	MAKE_LABEL_WID( kSetLumOptions_GreenText , 0, 1, 1, 2);				\
	MAKE_LABEL_WID( kSetLumOptions_BlueText	 , 0, 1, 2, 3);				\
																		\
	MAKE_TEXT_WID( kSetHorizontalOptions_Red , 		1, 2, 0, 1 );		\
	MAKE_TEXT_WID( kSetHorizontalOptions_Green	 , 	1, 2, 1, 2 );		\
	MAKE_TEXT_WID( kSetHorizontalOptions_Blue, 		1, 2, 2, 3 );		\
    }



#define 	kSetShearOptions_vname				 0
#define 	kSetShearOptions_vvar				 1
#define 	kSetShearOptions_hname				 2
#define 	kSetShearOptions_hvar				 3
#define 	kSetShearOptions_dlg				\
	{																	\
    MAKE_TABLE(2, 2); 													\
																		\
	MAKE_LABEL_WID( kSetShearOptions_vname , 0, 1, 0, 1);				\
	MAKE_LABEL_WID( kSetShearOptions_hname , 0, 1, 1, 2);				\
																		\
	MAKE_TEXT_WID( kSetShearOptions_vvar , 	1, 2, 0, 1 );				\
	MAKE_TEXT_WID( kSetShearOptions_hvar , 	1, 2, 1, 2 );				\
    }

//-------------------  Dialogs for Adjust --------------------------


#define		kSetAdjustPrefs_Insert				0
#define		kSetAdjustPrefs_Extract				1
#define		kSetAdjustPrefs_SetCtrlPts			2
#define		kSetAdjustPrefs_RunOptimizer		3
#define		kSetAdjustPrefs_Options  			4
#define     kSetAdjustPrefs_Script		    	5
#define     kSetAdjustPrefs_FindScript    		6
#define		kSetAdjustPrefs_SetOpt				7
#define		kSetAdjustPrefs_SetPrefs			8
#define		kSetAdjustPrefs_dlg					\
	{																	\
    MAKE_TABLE(5, 4); 													\
																		\
    frame = gtk_frame_new("Action:");									\
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);	\
    	gtk_container_border_width(GTK_CONTAINER(frame), 2);			\
    gtk_table_attach(GTK_TABLE(table), frame, 0, 2, 1, 5,				\
        GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);							\
    box = gtk_vbox_new(FALSE, 4);										\
    gtk_container_border_width(GTK_CONTAINER(box), 5);					\
    gtk_container_add(GTK_CONTAINER(frame), box);						\
  	gtk_widget_show (box);												\
  	gtk_widget_show (frame);											\
    group = NULL;														\
    MAKE_RADIO_BUTTON( kSetAdjustPrefs_Insert,       "Insert  into Panorama" )			\
    MAKE_RADIO_BUTTON( kSetAdjustPrefs_Extract,      "Extract from Panorama" )		\
    MAKE_RADIO_BUTTON( kSetAdjustPrefs_SetCtrlPts,   "Read Marked Controlpoints" )	\
    MAKE_RADIO_BUTTON( kSetAdjustPrefs_RunOptimizer,  "Run Position Optimizer" )	\
																		\
    frame = gtk_frame_new("Parameters:");								\
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);	\
    	gtk_container_border_width(GTK_CONTAINER(frame), 2);			\
    gtk_table_attach(GTK_TABLE(table), frame, 2, 3, 1, 3,				\
        GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);							\
	gtk_widget_show (frame);											\
  	{																	\
		GtkWidget 				*ftable;								\
    	ftable = gtk_table_new(2, 2, FALSE); 							\
    	gtk_container_border_width(GTK_CONTAINER(ftable), 2);			\
    	gtk_container_add(GTK_CONTAINER(frame), ftable);				\
    	gtk_widget_show(ftable);										\
    	box = gtk_vbox_new(FALSE, 2);									\
    	gtk_container_border_width(GTK_CONTAINER(box), 2);				\
    	gtk_table_attach(GTK_TABLE(ftable), box, 0, 1, 0, 2,			\
        	GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);						\
 		gtk_widget_show (box);											\
    	group = NULL;													\
    	MAKE_RADIO_BUTTON( kSetAdjustPrefs_Options,     "Use Options")	\
    	MAKE_RADIO_BUTTON( kSetAdjustPrefs_Script,      "Use Script" )	\
	 																	\
																		\
    	button = gtk_button_new_with_label ("Set");						\
    	INSERT_FBUTTON( 1, 2, 0, 1)										\
  		gtk_signal_connect (GTK_OBJECT (button), "clicked",				\
                             (GtkSignalFunc) SetCrOpt,					\
                             (gpointer) &localPrefs );					\
    	gtk_widget_show(button);										\
																		\
    	button = gtk_button_new_with_label ("Browse");					\
    	INSERT_FBUTTON( 1, 2, 1, 2)										\
  		gtk_signal_connect (GTK_OBJECT (button), "clicked",				\
                             (GtkSignalFunc) pt_find_file,				\
                             (gpointer) &(localPrefs.scriptFile) );		\
    	gtk_widget_show(button);										\
    }																	\
    button = gtk_button_new_with_label ("Prefs");						\
    INSERT_BUTTON( 4, 5, 0, 1)								     		\
   	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                            (GtkSignalFunc) SetSiz,						\
                            (gpointer) gsPrPtr);						\
    gtk_widget_show(button);											\
    }



#if 0
    MAKE_TEXTLABEL("Panning:", 			table, 3, 4, 13, 14);			\
    button = gtk_button_new_with_label ("Set");							\
    gtk_table_attach(GTK_TABLE(table), button, 3, 4, 14, 15,      		\
        GTK_FILL | GTK_EXPAND, GTK_FILL, 5, 5);							\
  	gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) pt_set_pan_opt,			\
                             NULL);										\
    gtk_widget_show(button);											\

#endif																		


#define		kSetCreateOptions_ImR				0
#define		kSetCreateOptions_ImP				1
#define		kSetCreateOptions_ImFf				2
#define		kSetCreateOptions_ImFc				3
#define		kSetCreateOptions_ImHfov			4
#define		kSetCreateOptions_ImW				5
#define		kSetCreateOptions_ImH				6
#define		kSetCreateOptions_Correct			7
#define		kSetCreateOptions_PRe				8
#define		kSetCreateOptions_PPa				9
#define		kSetCreateOptions_PSp				10
#define		kSetCreateOptions_PCu				11
#define		kSetCreateOptions_PHfov				12
#define		kSetCreateOptions_PWi				13
#define		kSetCreateOptions_PHe				14
#define		kSetCreateOptions_PSave				15
#define		kSetCreateOptions_TY				16
#define		kSetCreateOptions_TP				17
#define		kSetCreateOptions_TR				18
#define		kSetCreateOptions_SLoad				19
#define		kSetCreateOptions_SPaste			20
#define		kSetCreateOptions_SBlend			21
#define		kSetCreateOptions_SF				22
#define		kSetCreateOptions_SIm				23
#define		kSetCreateOptions_SBuf				24
#define		kSetCreateOptions_Sboth				25
#define		kSetCreateOptions_Snone				26
#define		kSetCreateOptions_Pan				27
#define		kSetCreateOptions_dlg				\
	{																	\
    MAKE_TABLE(2, 2); 													\
																		\
	{																		\
		GtkWidget 				*ftable;									\
    	frame = gtk_frame_new("Image:");									\
    	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);	\
    		gtk_container_border_width(GTK_CONTAINER(frame), 2);			\
    	gtk_table_attach(GTK_TABLE(table), frame, 0, 1, 0, 1,				\
        	GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);							\
  		gtk_widget_show (frame);											\
    	ftable = gtk_table_new(4, 3, FALSE); 								\
    	gtk_container_border_width(GTK_CONTAINER(ftable), 2);				\
    	gtk_container_add(GTK_CONTAINER(frame), ftable);					\
    	gtk_widget_show(ftable);											\
     	frame = gtk_frame_new("Format:");									\
    	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);	\
    		gtk_container_border_width(GTK_CONTAINER(frame), 2);			\
    	gtk_table_attach(GTK_TABLE(ftable), frame, 2, 3, 0, 4,				\
        	GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);							\
    	box = gtk_vbox_new(FALSE, 4);										\
    	gtk_container_border_width(GTK_CONTAINER(box), 2);					\
    	gtk_container_add(GTK_CONTAINER(frame), box);						\
  		gtk_widget_show (box);												\
  		gtk_widget_show (frame);											\
    	group = NULL;														\
    	MAKE_RADIO_BUTTON( kSetCreateOptions_ImR,     "Rectilinear")		\
    	MAKE_RADIO_BUTTON( kSetCreateOptions_ImP,     "Panoramic")			\
    	MAKE_RADIO_BUTTON( kSetCreateOptions_ImFf,    "Fisheye fullfr.")	\
    	MAKE_RADIO_BUTTON( kSetCreateOptions_ImFc,    "Fisheye circ.")		\
   		MAKE_TEXTLABEL("HFoV:", 			ftable, 0, 1, 0, 1);			\
    	MAKE_TEXTLABEL("Width:", 		ftable, 0, 1, 1, 2);				\
    	MAKE_TEXTLABEL("Height:", 		ftable, 0, 1, 2, 3);				\
    	button = gtk_button_new_with_label ("Correct");						\
    	INSERT_FBUTTON( 1, 2, 3, 4)								    		\
  		gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) SetCPrefs,						\
                             (gpointer) &localPrefs.im.cP);					\
    	gtk_widget_show(button);											\
		MAKE_FTEXT_WID( kSetCreateOptions_ImHfov , 	1, 2, 0, 1 );			\
		MAKE_FTEXT_WID( kSetCreateOptions_ImW , 	1, 2, 1, 2 );			\
		MAKE_FTEXT_WID( kSetCreateOptions_ImH , 	1, 2, 2, 3 );			\
  	}																		\
	{																		\
		GtkWidget 				*ftable;									\
    	frame = gtk_frame_new("Panorama:");									\
    	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);	\
    		gtk_container_border_width(GTK_CONTAINER(frame), 2);			\
    	gtk_table_attach(GTK_TABLE(table), frame, 0, 1, 1, 2,				\
        	GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);							\
  		gtk_widget_show (frame);											\
    	ftable = gtk_table_new(4, 3, FALSE); 								\
    	gtk_container_border_width(GTK_CONTAINER(ftable), 2);				\
    	gtk_container_add(GTK_CONTAINER(frame), ftable);					\
    	gtk_widget_show(ftable);											\
     	frame = gtk_frame_new("Format:");									\
    	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);	\
    		gtk_container_border_width(GTK_CONTAINER(frame), 2);			\
    	gtk_table_attach(GTK_TABLE(ftable), frame, 2, 3, 0, 3,				\
        	GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);							\
    	box = gtk_vbox_new(FALSE, 3);										\
    	gtk_container_border_width(GTK_CONTAINER(box), 2);					\
    	gtk_container_add(GTK_CONTAINER(frame), box);						\
  		gtk_widget_show (box);												\
  		gtk_widget_show (frame);											\
    	group = NULL;														\
   		MAKE_RADIO_BUTTON( kSetCreateOptions_PRe,  "Rectilinear")			\
    	MAKE_RADIO_BUTTON( kSetCreateOptions_PPa,  "QTVR-Panorama")			\
    	MAKE_RADIO_BUTTON( kSetCreateOptions_PSp,  "PSphere")				\
		MAKE_FCHECK_BOX(kSetCreateOptions_PSave, "Save to Buffer", 0, 2, 3, 4 ) \
    	MAKE_TEXTLABEL("HFoV:", 			ftable, 0, 1, 0, 1);			\
    	MAKE_TEXTLABEL("Width:", 			ftable, 0, 1, 1, 2);			\
    	MAKE_TEXTLABEL("Height:", 			ftable, 0, 1, 2, 3);			\
		MAKE_FTEXT_WID( kSetCreateOptions_PHfov , 	1, 2, 0, 1 );			\
		MAKE_FTEXT_WID( kSetCreateOptions_PWi , 	1, 2, 1, 2 );			\
		MAKE_FTEXT_WID( kSetCreateOptions_PHe , 	1, 2, 2, 3 );			\
  	}																		\
	{																		\
		GtkWidget 				*ftable;									\
    	frame = gtk_frame_new("Position:");									\
    	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);	\
    		gtk_container_border_width(GTK_CONTAINER(frame), 2);			\
    	gtk_table_attach(GTK_TABLE(table), frame, 1, 2, 0, 1,				\
        	GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);							\
  		gtk_widget_show (frame);											\
    	ftable = gtk_table_new(3, 2, FALSE); 								\
    	gtk_container_border_width(GTK_CONTAINER(ftable), 2);				\
    	gtk_container_add(GTK_CONTAINER(frame), ftable);					\
    	gtk_widget_show(ftable);											\
    	MAKE_TEXTLABEL("Yaw: -180...+180", 	ftable, 0, 1, 0, 1);			\
    	MAKE_TEXTLABEL("Pitch: -90...+90", 	ftable, 0, 1, 1, 2);			\
    	MAKE_TEXTLABEL("Roll:", 				ftable, 0, 1, 2, 3);			\
		MAKE_FTEXT_WID( kSetCreateOptions_TY , 		1, 2, 0, 1 );			\
		MAKE_FTEXT_WID( kSetCreateOptions_TP , 		1, 2, 1, 2 );			\
		MAKE_FTEXT_WID( kSetCreateOptions_TR , 		1, 2, 2, 3 );			\
  	}																		\
	{																		\
		GtkWidget 				*ftable;									\
    	frame = gtk_frame_new("Stitching:");								\
    	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);	\
    		gtk_container_border_width(GTK_CONTAINER(frame), 2);			\
    	gtk_table_attach(GTK_TABLE(table), frame, 1, 2, 1, 2,				\
        	GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);							\
  		gtk_widget_show (frame);											\
    	ftable = gtk_table_new(4, 4, FALSE); 								\
    	gtk_container_border_width(GTK_CONTAINER(ftable), 2);				\
    	gtk_container_add(GTK_CONTAINER(frame), ftable);					\
    	gtk_widget_show(ftable);											\
		MAKE_FCHECK_BOX(kSetCreateOptions_SLoad, "Load Buffer and ", 0, 2, 0, 1 ) \
    	box = gtk_vbox_new(FALSE, 2);										\
    	gtk_container_border_width(GTK_CONTAINER(box), 2);					\
    	gtk_table_attach(GTK_TABLE(ftable), box, 0, 2, 1, 3,				\
        	GTK_FILL | GTK_EXPAND, GTK_FILL, 5, 0);							\
		gtk_widget_show (box);												\
    	group = NULL;														\
    	MAKE_RADIO_BUTTON(kSetCreateOptions_SBlend,  "blend or")			\
    	MAKE_RADIO_BUTTON(kSetCreateOptions_SPaste,  "paste")				\
    	MAKE_TEXTLABEL("Feather:", 			ftable, 0, 1, 3, 4);			\
		MAKE_FTEXT_WID( kSetCreateOptions_SF , 		1, 2, 3, 4 );			\
    	frame = gtk_frame_new("Color Adjustment:");							\
    	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);	\
    		gtk_container_border_width(GTK_CONTAINER(frame), 2);			\
    	gtk_table_attach(GTK_TABLE(ftable), frame, 2, 4, 0, 4,				\
        	GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);							\
    	box = gtk_vbox_new(FALSE, 4);										\
    	gtk_container_border_width(GTK_CONTAINER(box), 2);					\
    	gtk_container_add(GTK_CONTAINER(frame), box);						\
  		gtk_widget_show (box);												\
  		gtk_widget_show (frame);											\
    	group = NULL;														\
     	MAKE_RADIO_BUTTON(kSetCreateOptions_SIm,    "Image")				\
    	MAKE_RADIO_BUTTON(kSetCreateOptions_SBuf,   "Buffer")				\
    	MAKE_RADIO_BUTTON(kSetCreateOptions_Sboth,  "Both")					\
    	MAKE_RADIO_BUTTON(kSetCreateOptions_Snone,  "None")					\
  	}																		\
    }

//-------------------- Interpolator Selection ------------------------------
	
#define kSetIntpPrefs_Poly          0
#define kSetIntpPrefs_Sp36          1
#define kSetIntpPrefs_Sp64			2
#define kSetIntpPrefs_Sinc256		3
#define kSetIntpPrefs_AAHammering   4
#define kSetIntpPrefs_AAGaussian    5
#define kSetIntpPrefs_AAQuadratic   6
#define kSetIntpPrefs_AAMitchell    7
#define kSetIntpPrefs_AALauczos2    8
#define kSetIntpPrefs_AALauczos3    9
#define kSetIntpPrefs_FastTNorm		10
#define kSetIntpPrefs_FastTMed		11
#define kSetIntpPrefs_FastTFast		12
#define kSetIntpPrefs_Gamma	        13
#define kSetIntpPrefs_SetIntp													\
	MAKE_TABLE(3, 6);												\
                                                                        \
    MAKE_TEXTLABEL("<- Faster",         table, 2, 3, 1, 2);             \
    MAKE_TEXTLABEL("Better ->",         table, 2, 3, 2, 3);             \
                                                                        \
    box = gtk_vbox_new(FALSE, 4);                                       \
    gtk_container_border_width(GTK_CONTAINER(box), 5);                  \
    gtk_table_attach(GTK_TABLE(table), box, 0, 2, 0, 4,                 \
        GTK_FILL | GTK_EXPAND, GTK_FILL, 5, 0);                         \
    gtk_widget_show (box);                                              \
    group = NULL;                                                       \
	MAKE_RADIO_BUTTON( kSetIntpPrefs_Poly,	  _("Polynomial 16 pixels"))	\
	MAKE_RADIO_BUTTON( kSetIntpPrefs_Sp36,	  _("Spline		36 pixels"))	\
	MAKE_RADIO_BUTTON( kSetIntpPrefs_Sp64,	  _("Spline		64 pixels"))	\
	MAKE_RADIO_BUTTON( kSetIntpPrefs_Sinc256,  _("Sinc		256 pixels"))	\
	MAKE_RADIO_BUTTON( kSetIntpPrefs_AAHammering,  _("Hammering		1.0"))	\
	MAKE_RADIO_BUTTON( kSetIntpPrefs_AAGaussian,  _("Gaussian	1.2"))	\
	MAKE_RADIO_BUTTON( kSetIntpPrefs_AAQuadratic,  _("Quadratic	1.5"))	\
	MAKE_RADIO_BUTTON( kSetIntpPrefs_AAMitchell,  _("Mitchell	2.0"))	\
	MAKE_RADIO_BUTTON( kSetIntpPrefs_AALauczos2,  _("Lauczos2	2.0"))	\
	MAKE_RADIO_BUTTON( kSetIntpPrefs_AALauczos3,  _("Lauczos3	3.0"))	\
																	\
	MAKE_BFRAME(table, _("Interpolation speed:"), 0, 2, 4, 5, 3);	\
	MAKE_RADIO_BUTTON( kSetIntpPrefs_FastTNorm, _("Normal") )		\
	MAKE_RADIO_BUTTON( kSetIntpPrefs_FastTMed,  _("Medium") )		\
	MAKE_RADIO_BUTTON( kSetIntpPrefs_FastTFast, _("Fast") )			\
																	\
	MAKE_TEXTLABEL(_("Gamma:"),		  table, 0, 1, 5, 6);			\
	MAKE_TEXT_WID( kSetIntpPrefs_Gamma ,		1, 2, 5, 6 );



#define kSetSizePrefs_Crop				0
#define kSetSizePrefs_SFile  			1
#define kSetSizePrefs_SetInt			2
#define kSetSizePrefs_dlg				\
  	    MAKE_TABLE(3, 3); 															\
																					\
    	MAKE_TEXTLABEL("If source and result size differ:", table, 0, 2, 0, 1);		\
																					\
    	box = gtk_vbox_new(FALSE, 2);												\
    	gtk_container_border_width(GTK_CONTAINER(box), 5);							\
    	gtk_table_attach(GTK_TABLE(table), box, 0, 2, 1, 3,							\
        	GTK_FILL | GTK_EXPAND, GTK_FILL,0, 0);									\
		gtk_widget_show (box);														\
    	group = NULL;																\
    	MAKE_RADIO_BUTTON( kSetSizePrefs_Crop, "Display cropped/framed image")		\
    	MAKE_RADIO_BUTTON( kSetSizePrefs_SFile, "Resize image")						\
     																				\
    	button = gtk_button_new_with_label ("More");								\
    	INSERT_BUTTON( 2, 3, 0, 1)								    				\
  		gtk_signal_connect (GTK_OBJECT (button), "clicked",							\
                             (GtkSignalFunc) SetIntp,								\
                             (gpointer) &localPrefs);								\
    	gtk_widget_show(button);													\
	

#define		kSetFourierOptions_PSDname			0
#define		kSetFourierOptions_findPSD			1
#define		kSetFourierOptions_addBlur			2
#define		kSetFourierOptions_remBlur			3
#define		kSetFourierOptions_internal			4
#define		kSetFourierOptions_custom			5
#define		kSetFourierOptions_findNFF			6
#define		kSetFourierOptions_filterfactor		7
#define		kSetFourierOptions_fourier_frame	8
#define		kSetFourierOptions_dlg				\
		MAKE_TABLE(4,7);		\
    	MAKE_TEXTLABEL("Point Spread Image:", table, 0, 2, 0, 1);			\
    	button = gtk_button_new_with_label ("Browse");						\
		INSERT_BUTTON( 3, 4, 0, 1)								     		\
  		gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) pt_find_file,					\
                             (gpointer) &(localPrefs.psf) );					\
    	gtk_widget_show(button);											\
																			\
    	MAKE_TEXTLABEL("Mode:", table, 0, 1, 2, 3);							\
    	box = gtk_vbox_new(FALSE, 2);										\
    	gtk_container_border_width(GTK_CONTAINER(box), 5);					\
    	gtk_table_attach(GTK_TABLE(table), box, 1, 3, 2, 4,					\
        	GTK_FILL | GTK_EXPAND, GTK_FILL, 5, 0);							\
		gtk_widget_show (box);												\
    	group = NULL;														\
    	MAKE_RADIO_BUTTON( kSetFourierOptions_addBlur,    "Add Blur")		\
    	MAKE_RADIO_BUTTON( kSetFourierOptions_remBlur,    "Remove Blur")	\
    	MAKE_TEXTLABEL("Noise Reduction", table, 0, 1, 4, 5);				\
    	box = gtk_vbox_new(FALSE, 2);										\
    	gtk_container_border_width(GTK_CONTAINER(box), 5);					\
    	gtk_table_attach(GTK_TABLE(table), box, 1, 3, 4, 6,					\
        	GTK_FILL | GTK_EXPAND, GTK_FILL, 5, 0);							\
		gtk_widget_show (box);												\
    	group = NULL;														\
    	MAKE_RADIO_BUTTON( kSetFourierOptions_internal,    "Internal")		\
    	MAKE_RADIO_BUTTON( kSetFourierOptions_custom,    "External")		\
    	button = gtk_button_new_with_label ("Browse");						\
		INSERT_BUTTON( 3, 4, 5, 6)								     		\
  		gtk_signal_connect (GTK_OBJECT (button), "clicked",					\
                             (GtkSignalFunc) pt_find_file,					\
                             (gpointer) &(localPrefs.nff) );				\
    	gtk_widget_show(button);											\
    	MAKE_TEXTLABEL("Filter Factor:", table, 0, 1, 6, 7);				\
		MAKE_TEXT_WID( kSetFourierOptions_filterfactor , 1, 2, 6, 7 );		\
    	MAKE_TEXTLABEL("Frame", table, 2, 3, 6, 7);							\
		MAKE_TEXT_WID( kSetFourierOptions_fourier_frame , 3, 4, 6, 7 );		\
																			\
		




#endif
