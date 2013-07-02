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

//#include "SysTypes.r"//commented by Kekus Digital
//#include "Types.r"
//#include "version.h"//till here
#include <Carbon.r>//added by Kekus Digital

#define SystemSevenOrLater 1

/*resource 'vers' (1) { // commented by Kekus Digital
	VERS1, VERS2, release, 0,
	verUS,
	VERSION,
	LONGVERSION
};*/

data 'SLEP' (128, "Sleep Value") {
	$"0000 0000"
	};


resource 'DITL' (200) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{104, 168, 125, 237},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{104, 56, 125, 124},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{44, 39, 65, 241},
		EditText {
			enabled,
			""
		},
		/* [4] */
		{6, 9, 30, 208},
		StaticText {
			disabled,
			"Static Text"
		}
	}
};

resource 'DLOG' (200) {
	{40, 40, 220, 320},
	dBoxProc,
	visible,
	goAway,
	0x0,
	200,
	"",centerParentWindowScreen;
};

resource 'dlgx' (200) {
	versionZero {
		kDialogFlagsUseThemeControls | kDialogFlagsHandleMovableModal/* | kDialogFlagsUseControlHierarchy*/ | kDialogFlagsUseThemeBackground
	}
};


resource 'ALRT' (130,purgeable) {
	{40, 40, 154, 342},
	130,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};


resource 'DITL' (130, purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{72, 222, 92, 280},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{14, 60, 75, 297},
		StaticText {
			disabled,
			"^0"
		}
	}
};


resource 'DLOG' (110) {
	/*{40, 40, 160, 280}*/{40, 40, 160, 285}, //changed by Kekus Digital 11 August 2003
	/*documentProc*/noGrowDocProc,//changed by Kekus Digital
	visible,
	noGoAway,
	0x0,
	110,
	"Name comes here"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;

};

resource 'DITL' (110) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{80, 162, 100, 220},
		Button {
			enabled,
			"Stop"
		},
		/* [2] */
		{30, 20, 46, 220},
		Control {
			enabled,
			128
		}
	}
};

resource 'dlgx' (110) {
	versionZero {
		kDialogFlagsUseThemeControls | kDialogFlagsHandleMovableModal /*| kDialogFlagsUseControlHierarchy*/ | kDialogFlagsUseThemeBackground
	}
};


resource 'CNTL' (128, "Standard Progress Bar", purgeable) {
	{0, 0, 16, 200},
	67,
	visible,
	100,
	0,
	80/*3200*/,//changed by Kekus Digital
	0,
	"Standard progress bar"
};



resource 'DITL' (300) {
	{	/* array DITLarray: 21 elements */
		/* [1] */
		{260, 272, 280, 340},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{260, 192, 280, 260},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{260, 112, 280, 180},
		Button {
			enabled,
			"Save"
		},
		/* [4] */
		{260, 32, 280, 100},
		Button {
			enabled,
			"Load"
		},
		/* [5] */
		{14, 20, 32, 126},
		CheckBox {
			enabled,
			"Radial Shift"
		},
		/* [6] */
		{14, 169, 34, 253},
		Button {
			enabled,
			"Options…"
		},
		/* [7] */
		{44, 20, 62, 126},
		CheckBox {
			enabled,
			"Vertical Shift"
		},
		/* [8] */
		{44, 169, 64, 253},
		Button {
			enabled,
			"Options…"
		},
		/* [9] */
		{74, 20, 92, 155},
		CheckBox {
			enabled,
			"Horizontal Shift"
		},
		/* [10] */
		{74, 169, 94, 253},
		Button {
			enabled,
			"Options…"
		},
		/* [11] */
		{104, 20, 122, 126},
		CheckBox {
			enabled,
			"Shear"
		},
		/* [12] */
		{104, 169, 124, 253},
		Button {
			enabled,
			"Options…"
		},
		/* [13] */
		{134, 20, 152, 126},
		CheckBox {
			enabled,
			"Scale"
		},
		/* [14] */
		{134, 169, 154, 253},
		Button {
			enabled,
			"Options…"
		},
		/* [15] */
		{164, 20, 182, 155},
		CheckBox {
			enabled,
			"Radial Luminance"
		},
		/* [16] */
		{164, 169, 184, 253},
		Button {
			enabled,
			"Options…"
		},
		/* [17] */
		{14, 272, 34, 340},
		Button {
			enabled,
			"Pref…"
		},
		/* [18] */
		{194, 20, 212, 155},
		CheckBox {
			enabled,
			"Cut Frame"
		},
		/* [19] */
		{194, 169, 214, 253},
		Button {
			enabled,
			"Options…"
		},
		/* [20] */
		{224, 20, 242, 155},
		CheckBox {
			enabled,
			"Fourier Filter"
		},
		/* [21] */
		{224, 169, 244, 253},
		Button {
			enabled,
			"Options…"
		}
	}
};

resource 'dlgx' (300) {
	versionZero {
		kDialogFlagsUseThemeControls | kDialogFlagsHandleMovableModal /*| kDialogFlagsUseControlHierarchy*/ | kDialogFlagsUseThemeBackground
	}
};


resource 'DITL' (301) {
	{	/* array DITLarray: 25 elements */
		/* [1] */
		/*{190, 224, 210, 292}*/{190, 324, 210, 392},//changed by Kekus Digital 12 August 2003
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		/*{190, 144, 210, 212}*/{190, 244, 210, 312},//changed by Kekus Digital 12 August 2003
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{36, 20, 53, 50},
		StaticText {
			disabled,
			"red"
		},
		/* [4] */
		{60, 20, 77, 64},
		StaticText {
			disabled,
			"green"
		},
		/* [5] */
		{87, 20, 104, 55},
		StaticText {
			disabled,
			"blue"
		},

/* I changed the order of the parameters to a, b, c, d from the original d, c, b, a. Some users were getting confused as the order of the parameters differed between the Windows and Mac versions.  These changes make the order the same.  Kekus Digital. Changed 12 August 2003 from here*/
		/* [6] */
                /*{12, 77, 28, 103}*/{12, 356, 28, 366},
		StaticText {
			disabled,
			"d"
		},
                /* [7] */
                /*{12, 134, 28, 161}*/{12, 274, 28, 284},
                StaticText {
                        disabled,
                        "c"
                },
                /* [8] */
                /*{12, 189, 28, 255}*/{12, 190, 28, 200},
                StaticText {
                        disabled,
                        "b"
                },
                /* [9] */
                /*{12, 251, 28, 277}*/{12, 107, 28, 117},
                StaticText {
                         disabled,
                         "a"
                },
		/* [10] */
		/*{35, 72, 51, 118}*/{35, 321, 51, 392},
		EditText {
			enabled,
			""
		},
		/* [11] */
		/*{35, 130, 51, 176}*/{35, 238, 51, 309},
		EditText {
			enabled,
			""
		},
		/* [12] */
		/*{35, 188, 51, 234}*/{35, 155, 51, 226},
		EditText {
			enabled,
			""
		},
		/* [13] */
		/*{35, 246, 51, 292}*/{35, 72, 51, 143},
		EditText {
			enabled,
			""
		},
		/* [14] */
		/*{63, 72, 79, 118}*/{63, 321, 79, 392},
		EditText {
			enabled,
			""
		},
		/* [15] */
		/*{63, 130, 79, 176}*/{63, 238, 79, 309},
		EditText {
			enabled,
			""
		},
		/* [16] */
		/*{63, 188, 79, 234}*/{63, 155, 79, 226},
		EditText {
			enabled,
			""
		},
		/* [17] */
		/*{63, 246, 79, 292}*/{63, 72, 79, 143},
		EditText {
			enabled,
			""
		},
		/* [18] */
		/*{91, 72, 107, 118}*/{91, 321, 107, 392},
		EditText {
			enabled,
			""
		},
		/* [19] */
		/*{91, 130, 107, 176}*/{91, 238, 107, 309},
		EditText {
			enabled,
			""
		},
		/* [20] */
		/*{91, 188, 107, 234}*/{91, 155, 107, 226},
		EditText {
			enabled,
			""
		},
		/* [21] */
		/*{91, 246, 107, 292}*/{91, 72, 107, 143}, //End of 12 August 2003 Kekus changes
		EditText {
			enabled,
			""
		},
		/* [22] */
		{117, 68, 133, 131},
		RadioButton {
			enabled,
			"Radial"
		},
		/* [23] */
		{138, 68, 154, 139},
		RadioButton {
			enabled,
			"Vertical"
		},
		/* [24] */
		{159, 68, 175, 158},
		RadioButton {
			enabled,
			"Horizontal"
		},
		/* [25] */
		{117, 20, 135, 68},
		StaticText {
			disabled,
			"Mode:"
		}
	}
};

resource 'dlgx' (301) {
	versionZero {
		kDialogFlagsUseThemeControls | kDialogFlagsHandleMovableModal/* | kDialogFlagsUseControlHierarchy */| kDialogFlagsUseThemeBackground
	}
};

resource 'DITL' (302) {
	{	/* array DITLarray: 8 elements */
		/* [1] */
		{120, 172, 140, 240},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{120, 92, 140, 160},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{26, 57, 42, 120},
		StaticText {
			disabled,
			"Red"
		},
		/* [4] */
		{50, 57, 66, 120},
		StaticText {
			disabled,
			"Green"
		},
		/* [5] */
		{76, 57, 92, 120},
		StaticText {
			disabled,
			"Blue"
		},
		/* [6] */
		{25, 122, 39, 188},
		EditText {
			enabled,
			""
		},
		/* [7] */
		{51, 122, 65, 188},
		EditText {
			enabled,
			""
		},
		/* [8] */
		{77, 122, 91, 188},
		EditText {
			enabled,
			""
		}
	}
};

resource 'dlgx' (302) {
	versionZero {
		kDialogFlagsUseThemeControls | kDialogFlagsHandleMovableModal /*| kDialogFlagsUseControlHierarchy */| kDialogFlagsUseThemeBackground
	}
};

resource 'DITL' (303) {
	{	/* array DITLarray: 6 elements */
		/* [1] */
		{120, 172, 140, 240},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{120, 92, 140, 160},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{34, 47, 52, 112},
		StaticText {
			disabled,
			"Width:"
		},
		/* [4] */
		{34, 129, 50, 204},
		EditText {
			enabled,
			""
		},
		/* [5] */
		{64, 47, 82, 112},
		StaticText {
			disabled,
			"Height:"
		},
		/* [6] */
		{64, 129, 80, 204},
		EditText {
			enabled,
			""
		}
	}
};

resource 'dlgx' (303) {
	versionZero {
		kDialogFlagsUseThemeControls | kDialogFlagsHandleMovableModal /*| kDialogFlagsUseControlHierarchy*/ | kDialogFlagsUseThemeBackground
	}
};

resource 'DITL' (310) {
	{	/* array DITLarray: 21 elements */
		/* [1] */
		{190, 232, 210, 300},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{190, 152, 210, 220},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{14, 23, 32, 91},
		StaticText {
			disabled,
			"From:"
		},
		/* [4] */
		{34, 20, 52, 113},
		RadioButton {
			enabled,
			"Normal"
		},
		/* [5] */
		{53, 20, 71, 112},
		RadioButton {
			enabled,
			/* "QTVR" */ "Cylindrical" //Changed by Kekus Digital 11 August 2003
		},
		/* [6] */
		{72, 20, 90, 111},
		RadioButton {
			enabled,
			"PSphere"
		},
		/* [7] */
		{91, 20, 109, 142},
		RadioButton {
			enabled,
			"Fisheye Hor"
		},
		/* [8] */
		{14, 161, 32, 230},
		StaticText {
			disabled,
			"To:"
		},
		/* [9] */
		{34, 159, 52, 230},
		RadioButton {
			enabled,
			"Normal"
		},
		/* [10] */
		{53, 159, 71, 251},
		RadioButton {
			enabled,
			/* "QTVR" */ "Cylindrical" // Changed by Kekus Digital 11 August 2003
		},
		/* [11] */
		{72, 159, 90, 250},
		RadioButton {
			enabled,
			"PSphere"
		},
		/* [12] */
		{91, 159, 109, 261},
		RadioButton {
			enabled,
			"Fisheye Hor"
		},
		/* [13] */
		{155, 24, 174, 60},
		StaticText {
			disabled,
			"Hfov:"
		},
		/* [14] */
		{157, 64, 174, 108},
		EditText {
			enabled,
			""
		},
		/* [15] */
		{110, 20, 128, 142},
		RadioButton {
			enabled,
			"Fisheye Vrt"
		},
		/* [16] */
		{110, 159, 128, 251},
		RadioButton {
			enabled,
			"Fisheye Vrt"
		},
		/* [17] */
		{154, 163, 174, 199},
		StaticText {
			disabled,
			"Vfov:"
		},
		/* [18] */
		{157, 203, 174, 247},
		EditText {
			enabled,
			""
		},
		/* [19] */
		{129, 20, 147, 142},
		RadioButton {
			enabled,
			"Convex Mirror"
		},
		/* [20] */
		{129, 159, 147, 271},
		RadioButton {
			enabled,
			"Convex Mirror"
		},
		/* [21] */
		{14, 232, 34, 300},
		Button {
			enabled,
			"Pref…"
		}
	}
};

resource 'dlgx' (310) {
	versionZero {
		kDialogFlagsUseThemeControls | kDialogFlagsHandleMovableModal /*| kDialogFlagsUseControlHierarchy*/ | kDialogFlagsUseThemeBackground
	}
};


resource 'DITL' (320) {
	{	/* array DITLarray: 23 elements */
		/* [1] */
		{205, 232, 225, 300},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{205, 152, 225, 220},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{14, 20, 32, 75},
		StaticText {
			disabled,
			"Format:"
		},
		/* [4] */
		{14, 82, 32, 153},
		RadioButton {
			enabled,
			"Normal"
		},
		/* [5] */
		{33, 82, 51, 153},
		RadioButton {
			enabled,
			"Fisheye"
		},
		/* [6] */
		{119, 166, 135, 205},
		StaticText {
			disabled,
			"Hfov:"
		},
		/* [7] */
		{119, 221, 135, 275},
		EditText {
			enabled,
			""
		},
		/* [8] */
		{61, 20, 76, 75},
		StaticText {
			disabled,
			"Turn to:"
		},
		/* [9] */
		{62, 82, 78, 117},
		StaticText {
			disabled,
			"Hor"
		},
		/* [10] */
		{62, 121, 78, 178},
		EditText {
			enabled,
			""
		},
		/* [11] */
		{90, 121, 106, 178},
		EditText {
			enabled,
			""
		},
		/* [12] */
		{90, 82, 106, 115},
		StaticText {
			disabled,
			"Vert"
		},
		/* [13] */
		{85, 196, 102, 270},
		RadioButton {
			enabled,
			"Degrees"
		},
		/* [14] */
		{64, 196, 83, 260},
		RadioButton {
			enabled,
			"Points"
		},
		/* [15] */
		{119, 25, 134, 75},
		StaticText {
			disabled,
			"Rotate:"
		},
		/* [16] */
		{119, 85, 135, 147},
		EditText {
			enabled,
			""
		},
		/* [17] */
		{175, 166, 191, 213},
		StaticText {
			disabled,
			"Width:"
		},
		/* [18] */
		{175, 221, 191, 275},
		EditText {
			enabled,
			""
		},
		/* [19] */
		{147, 166, 163, 213},
		StaticText {
			disabled,
			"Height:"
		},
		/* [20] */
		{147, 221, 163, 275},
		EditText {
			enabled,
			""
		},
		/* [21] */
		{145, 38, 165, 75},
		StaticText {
			disabled,
			"Size:"
		},
		/* [22] */
		{145, 83, 165, 151},
		Button {
			enabled,
			"Source"
		},
		/* [23] */
		{15, 242, 35, 300},
		Button {
			enabled,
			"Pref…"
		}
	}
};

resource 'dlgx' (320) {
	versionZero {
		kDialogFlagsUseThemeControls | kDialogFlagsHandleMovableModal /*| kDialogFlagsUseControlHierarchy */| kDialogFlagsUseThemeBackground
	}
};

resource 'DLOG' (300) {
	{40, 40, 340, 400},
	/*noGrowDocProc*/movableDBoxProc,//changed by Kekus Digital
	visible,
	noGoAway,
	0x0,
	300,
	"Correction"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (301) {
	/*{40, 40, 270, 352}*/{40, 40, 270, 452},//changed by Kekus Digital 12 August 2003
	/*noGrowDocProc*/movableDBoxProc,//changed by Kekus Digital
	visible,
	noGoAway,
	0x0,
	301,
	"Options for Radial Shift"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (302) {
	{40, 40, 200, 300},
	/*documentProc*/movableDBoxProc,//changed by Kekus Digital
	visible,
	noGoAway,
	0x0,
	302,
	"Vertical Shift"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (303) {
	{40, 40, 200, 300},
	/*documentProc*/movableDBoxProc,//changed by Kekus Digital
	visible,
	goAway,
	0x0,
	303,
	"Enter Size"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};


resource 'DLOG' (310) {
	{40, 40, 270, 360},
	/*noGrowDocProc*/movableDBoxProc, //changed by Kekus Digital
	visible,
	noGoAway,
	0x0,
	310,
	"Map Options"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (320) {
	{40, 40, 285, 360},
	/*noGrowDocProc*/movableDBoxProc, //changed by Kekus Digital
	visible,
	noGoAway,
	0x0,
	320,
	"Perspective Options"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};


/*  Resources for adjust	*/

resource 'DLOG' (330) {
	{40, 40, 275, 360},
	/*noGrowDocProc*/movableDBoxProc, //changed by Kekus Digital
	visible,
	noGoAway,
	0x0,
	330,
	"Create Panorama"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (331) {
	{40, 40, 370, 625},
	/*noGrowDocProc*/movableDBoxProc, //changed by Kekus Digital
	visible,
	noGoAway,
	0x0,
	331,
	"Options for Insert/Extract"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};


resource 'DLOG' (332) {
	{40, 40, 220, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	332,
	"Fit Into Panorama"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (333) {
	{40, 40, 220, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	333,
	"Options for Optimize"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (334) {
	{40, 40, 247, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	334,
	"Specify Matching Points"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DITL' (330) {
	{	/* array DITLarray: 14 elements */
		/* [1] */
		{195, 232, 215, 300},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{195, 152, 215, 220},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{49, 103, 68, 201},
		RadioButton {
			enabled,
			"Use Options"
		},
		/* [4] */
		{48, 209, 68, 286},
		Button {
			enabled,
			"Set…"
		},
		/* [5] */
		{166, 20, 183, 217},
		RadioButton {
			enabled,
			"Run Position Optimizer"
		},
		/* [6] */
		{80, 209, 100, 286},
		Button {
			enabled,
			"Browse…"
		},
		/* [7] */
		{49, 20, 68, 84},
		RadioButton {
			enabled,
			"Insert"
		},
		/* [8] */
		{14, 242, 33, 300},
		Button {
			enabled,
			"Pref…"
		},
		/* [9] */
		{142, 20, 159, 222},
		RadioButton {
			enabled,
			"Read  marked Control Points"
		},
		/* [10] */
		{75, 20, 95, 89},
		RadioButton {
			enabled,
			"Extract"
		},
		/* [11] */
		{75, 103, 95, 190},
		RadioButton {
			enabled,
			"Use Script"
		},
		/* [12] */
		{102, 18, 119, 285},
		StaticText {
			disabled,
			"---------------------------------------"
		},
		/* [13] */
		{119, 20, 137, 204},
		StaticText {
			disabled,
			"Tools for Script Generation:"
		},
		/* [14] */
		{28, 20, 46, 182},
		StaticText {
			disabled,
			"Insert or Extract Image:"
		}
	}
};

resource 'dlgx' (330) {
	versionZero {
		kDialogFlagsUseThemeControls | kDialogFlagsHandleMovableModal /*| kDialogFlagsUseControlHierarchy*/ | kDialogFlagsUseThemeBackground
	}
};

resource 'DITL' (331) {
	{	/* array DITLarray: 51 elements */
		/* [1] */
		{290, 497, 310, 565},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{290, 417, 310, 485},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{40, 153, 58, 210},
		StaticText {
			disabled,
			"Format:"
		},
		/* [4] */
		{38, 208, 56, 301},
		RadioButton {
			enabled,
			/* "Rectilinear" */ "Normal" //Changed by Kekus Digital
		},
		/* [5] */
		{76, 208, 94, 317},
		RadioButton {
			enabled,
			"Fisheye fullfr."
		},
		/* [6] */
		{40, 74, 56, 127},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [7] */
		{40, 20, 58, 60},
		StaticText {
			disabled,
			"HFov:"
		},
		/* [8] */
		/*{40, 342, 58, 467}*/{40, 342, 58, 475}, //Changed by Kekus Digital 11 August 2003
		StaticText {
			disabled,
			"Yaw: -180°...+180°"
		},
		/* [9] */
		/*{40, 476, 56, 527}*/ {40, 476, 56, 555},  //Changed by Kekus Digital 18 December 2003
		EditText {
			enabled,
			"Edit Text"
		},
		/* [10] */
		/*{66, 342, 84, 467}*/{66, 342, 84, 475}, //Changed by Kekus Digital 11 August 2003
		StaticText {
			disabled,
			"Pitch: -90°...+90°"
		},
		/* [11] */
		/*{66, 476, 82, 527}*/{66, 476, 82, 555}, //Changed by Kekus Digital 18 December 2003
		EditText {
			enabled,
			"Edit Text"
		},
		/* [12] */
		{92, 342, 110, 387},
		StaticText {
			disabled,
			"Roll:"
		},
		/* [13] */
		/*{92, 476, 108, 527}*/{92, 476, 108, 555}, //Changed by Kekus Digital 18 December 2003
		EditText {
			enabled,
			"Edit Text"
		},
		/* [14] */
		{57, 208, 75, 314},
		RadioButton {
			enabled,
			/*"Panoramic"*/ "Cylindrical" //Changed by Kekus Digital 11 August 2003
		},
		/* [15] */
		{14, 20, 30, 74},
		StaticText {
			disabled,
			"Image:"
		},
		/* [16] */
		{39, 135, 58, 141},
		StaticText {
			disabled,
			"°"
		},
		/* [17] */
		{155, 20, 173, 98},
		StaticText {
			disabled,
			"Panorama:"
		},
		/* [18] */
		{141, 14, 151, 290},
		StaticText {
			disabled,
			"------------------------------------------------"
		},
		/* [19] */
		{14, 341, 30, 405},
		StaticText {
			disabled,
			"Position:"
		},
		/* [20] */
		{95, 208, 113, 314},
		RadioButton {
			enabled,
			"Fisheye circ."
		},
		/* [21] */
		{66, 74, 82, 127},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [22] */
		{92, 74, 108, 127},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [23] */
		{66, 20, 84, 67},
		StaticText {
			disabled,
			"Width:"
		},
		/* [24] */
		{92, 20, 110, 71},
		StaticText {
			disabled,
			"Height:"
		},
		/* [25] */
		{183, 153, 201, 210},
		StaticText {
			disabled,
			"Format:"
		},
		/* [26] */
		{181, 208, 199, 302},
		RadioButton {
			enabled,
			/* "Rectilinear"*/ "Normal" // Changed by Kekus Digital 11 August 2003
		},
		/* [27] */
		{200, 208, 218, 314},
		RadioButton {
			enabled,
			/* "QTVR-pan." */ "Cylindrical" // Changed by Kekus Digital 11 August 2003
		},
		/* [28] */
		{219, 208, 237, 287},
		RadioButton {
			enabled,
			"PSphere"
		},
		/* [29] */
		{182, 20, 200, 60},
		StaticText {
			disabled,
			"HFov:"
		},
		/* [30] */
		{182, 74, 198, 127},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [31] */
		{208, 20, 226, 67},
		StaticText {
			disabled,
			"Width:"
		},
		/* [32] */
		{208, 74, 224, 127},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [33] */
		{234, 20, 252, 71},
		StaticText {
			disabled,
			"Height:"
		},
		/* [34] */
		{234, 74, 250, 127},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [35] */
		{183, 135, 198, 141},
		StaticText {
			disabled,
			"°"
		},
		/* [36] */
		{141, 290, 151, 567},
		StaticText {
			disabled,
			"----------------------------------------------------"
		},
		/* [37] */
		{183, 341, 201, 440},
		CheckBox {
			enabled,
			"Load  Buffer "
		},
		/* [38] */
		{155, 340, 173, 413},
		StaticText {
			disabled,
			"Stitching:"
		},
		/* [39] */
		{202, 341, 220, 426},
		RadioButton {
			enabled,
			"and Paste"
		},
		/* [40] */
		{221, 341, 239, 431},
		RadioButton {
			enabled,
			"or Blend"
		},
		/* [41] */
		{246, 340, 264, 400},
		StaticText {
			disabled,
			"Feather:"
		},
		/* [42] */
		{246, 405, 262, 442},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [43] */
		{118, 20, 138, 100},
		Button {
			enabled,
			"Correct…"
		},
		/* [44] */
		{260, 20, 278, 134},
		CheckBox {
			enabled,
			"Save to Buffer"
		},
		/* [45] */
		{183, 448, 200, 567},
		StaticText {
			disabled,
			"Color Adjustment:"
		},
		/* [46] */
		{202, 499, 220, 562},
		RadioButton {
			enabled,
			"Image"
		},
		/* [47] */
		{221, 499, 239, 560},
		RadioButton {
			enabled,
			"Buffer"
		},
		/* [48] */
		{240, 499, 258, 549},
		RadioButton {
			enabled,
			"both"
		},
		/* [49] */
		{259, 499, 277, 553},
		RadioButton {
			enabled,
			"none"
		},
		/* [50] */
		{14, 325, 290, 330},
		StaticText {
			disabled,
			"|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|"
		},
		/* [51] */
		{114, 208, 132, 288},
		RadioButton {
			enabled,
			"PSphere"
		}
	}
};

resource 'dlgx' (331) {
	versionZero {
		kDialogFlagsUseThemeControls | kDialogFlagsHandleMovableModal /*| kDialogFlagsUseControlHierarchy*/ | kDialogFlagsUseThemeBackground
	}
};


resource 'DITL' (332) {
	{	/* array DITLarray: 23 elements */
		/* [1] */
		{156, 166, 176, 224},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{156, 56, 176, 114},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{16, 121, 34, 178},
		StaticText {
			disabled,
			"Format:"
		},
		/* [4] */
		{12, 179, 31, 272},
		RadioButton {
			enabled,
			/*"Rectilinear"*/ "Normal" //Changed by Kekus Digital 11 August 2003
		},
		/* [5] */
		{45, 179, 62, 281},
		RadioButton {
			enabled,
			"Fisheye fullfr."
		},
		/* [6] */
		{23, 49, 40, 100},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [7] */
		{20, 6, 40, 46},
		StaticText {
			disabled,
			"HFov:"
		},
		/* [8] */
		{48, 7, 65, 124},
		CheckBox {
			enabled,
			"Correct Image"
		},
		/* [9] */
		{29, 179, 47, 285},
		RadioButton {
			enabled,
			"Panorama"
		},
		/* [10] */
		{0, 98, 17, 183},
		StaticText {
			disabled,
			"Input Image"
		},
		/* [11] */
		{20, 104, 41, 114},
		StaticText {
			disabled,
			"°"
		},
		/* [12] */
		{76, 10, 89, 277},
		StaticText {
			disabled,
			"-------------------------------------"
		},
		/* [13] */
		{87, 12, 103, 81},
		StaticText {
			disabled,
			"Optimize:"
		},
		/* [14] */
		{106, 11, 124, 83},
		RadioButton {
			enabled,
			"Overlap"
		},
		/* [15] */
		{122, 11, 141, 74},
		RadioButton {
			enabled,
			"Points"
		},
		/* [16] */
		{108, 85, 124, 142},
		Button {
			enabled,
			"Initial"
		},
		/* [17] */
		{125, 85, 141, 142},
		Button {
			enabled,
			"Points"
		},
		/* [18] */
		{88, 156, 106, 227},
		StaticText {
			disabled,
			"Variables:"
		},
		/* [19] */
		{103, 154, 120, 208},
		CheckBox {
			enabled,
			"yaw"
		},
		/* [20] */
		{120, 154, 137, 211},
		CheckBox {
			enabled,
			"pitch"
		},
		/* [21] */
		{137, 154, 153, 204},
		CheckBox {
			enabled,
			"roll"
		},
		/* [22] */
		{103, 215, 122, 262},
		CheckBox {
			enabled,
			"HFov"
		},
		/* [23] */
		{61, 179, 78, 281},
		RadioButton {
			enabled,
			"Fisheye circ."
		}
	}
};

resource 'DITL' (333) {
	{	/* array DITLarray: 10 elements */
		/* [1] */
		{134, 172, 154, 230},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{134, 53, 154, 111},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{18, 8, 35, 136},
		StaticText {
			disabled,
			"Yaw(-180 ... +180):"
		},
		/* [4] */
		{18, 160, 34, 235},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [5] */
		{42, 8, 59, 128},
		StaticText {
			disabled,
			"Pitch(-90 ... +90):"
		},
		/* [6] */
		{43, 160, 59, 235},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [7] */
		{66, 8, 82, 83},
		StaticText {
			disabled,
			"Roll:"
		},
		/* [8] */
		{68, 160, 84, 235},
		EditText {
			enabled,
			"Edit Text"
		}
#if 0
		,
		/* [9] */

		{90, 8, 106, 83},
		StaticText {
			disabled,
			"HFov:"
		},
		/* [10] */
		{94, 160, 110, 235},
		EditText {
			enabled,
			"Edit Text"
		}
#endif
	}
};

resource 'DITL' (334) {
	{	/* array DITLarray: 28 elements */
		/* [1] */
		{158, 161, 178, 219},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{158, 50, 178, 108},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{5, 17, 21, 100},
		StaticText {
			disabled,
			"Panorama:"
		},
		/* [4] */
		{5, 150, 23, 249},
		StaticText {
			disabled,
			"Source Image:"
		},
		/* [5] */
		{21, 21, 38, 41},
		StaticText {
			disabled,
			"x"
		},
		/* [6] */
		{21, 78, 37, 97},
		StaticText {
			disabled,
			"y"
		},
		/* [7] */
		{21, 170, 39, 185},
		StaticText {
			disabled,
			"x"
		},
		/* [8] */
		{21, 215, 39, 231},
		StaticText {
			disabled,
			"y"
		},
		/* [9] */
		{41, 16, 57, 56},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [10] */
		{64, 16, 80, 56},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [11] */
		{87, 16, 103, 56},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [12] */
		{110, 16, 126, 56},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [13] */
		{133, 16, 149, 56},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [14] */
		{41, 65, 57, 105},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [15] */
		{64, 65, 80, 105},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [16] */
		{87, 65, 103, 105},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [17] */
		{110, 65, 126, 105},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [18] */
		{133, 65, 149, 105},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [19] */
		{42, 153, 58, 193},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [20] */
		{65, 153, 81, 193},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [21] */
		{88, 153, 104, 193},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [22] */
		{111, 153, 127, 193},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [23] */
		{134, 153, 150, 193},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [24] */
		{42, 205, 58, 245},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [25] */
		{65, 205, 81, 245},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [26] */
		{88, 205, 104, 245},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [27] */
		{111, 205, 127, 245},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [28] */
		{134, 205, 150, 245},
		EditText {
			enabled,
			"Edit Text"
		}
	}
};


/*  Resources for interpolate	*/

resource 'DLOG' (340) {
	{40, 40, 220, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	340,
	"Interpolate Views"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DITL' (340) {
	{	/* array DITLarray: 7 elements */
		/* [1] */
		{141, 156, 161, 214},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{141, 56, 161, 114},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{30, 21, 49, 157},
		RadioButton {
			enabled,
			"Use Options"
		},
		/* [4] */
		{46, 172, 64, 250},
		Button {
			enabled,
			"Options"
		},
		/* [5] */
		{90, 20, 109, 137},
		RadioButton {
			enabled,
			"Run Optimizer"
		},
		/* [6] */
		{91, 172, 110, 250},
		Button {
			enabled,
			"Find Script"
		},
		/* [7] */
		{61, 20, 79, 126},
		RadioButton {
			enabled,
			"Use Script"
		}
	}
};


resource 'DLOG' (360) {
	{40, 40, 250, 350},
	/*noGrowDocProc*/movableDBoxProc,//changed by Kekus Digital
	visible,
	noGoAway,
	0x0,
	360,
	"Size Options"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (350) {
	{40, 40, 220, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	350,
	"Size Options"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};


resource 'DITL' (350) {
	{	/* array DITLarray: 6 elements */
		/* [1] */
		{144, 156, 164, 214},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{144, 56, 164, 114},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{13, 17, 33, 229},
		StaticText {
			disabled,
			"If Source and Result size differ:"
		},
		/* [4] */
		{60, 18, 79, 245},
		CheckBox {
			enabled,
			"Display Cropped/Framed Image"
		},
		/* [5] */
		{89, 18, 110, 164},
		CheckBox {
			enabled,
			"Create New Image"
		},
		/* [6] */
		{2, 241, 20, 278},
		Button {
			enabled,
			"More"
		}
	}
};

resource 'dlgx' (350) {
	versionZero {
		kDialogFlagsUseThemeControls | kDialogFlagsHandleMovableModal /*| kDialogFlagsUseControlHierarchy*/ | kDialogFlagsUseThemeBackground
	}
};

resource 'DITL' (360) {
	{	/* array DITLarray: 9 elements */
		/* [1] */
		{170, 222, 190, 290},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{170, 142, 190, 210},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{39, 20, 59, 232},
		StaticText {
			disabled,
			"If Source and Result size differ:"
		},
		/* [4] */
		{61, 20, 79, 262},
		CheckBox {
			enabled,
			"(a) Display Cropped/Framed Image"
		},
		/* [5] */
		{83, 20, 101, 212},
		CheckBox {
			enabled,
			"(b) Create New Image File"
		},
		/* [6] */
		{14, 222, 34, 290},
		Button {
			enabled,
			"More…"
		},
		/* [7] */
		{83, 222, 103, 290},
		Button {
			enabled,
			"Set…"
		},
		/* [8] */
		{105, 20, 123, 213},
		CheckBox {
			enabled,
			"(c) Automatically Open File"
		},
		/* [9] */
		{127, 20, 145, 261},
		CheckBox {
			enabled,
			"(d) Don't Save Mask (Photoshop LE)"
		}
	}
};

resource 'dlgx' (360) {
	versionZero {
		kDialogFlagsUseThemeControls | kDialogFlagsHandleMovableModal /*| kDialogFlagsUseControlHierarchy*/ | kDialogFlagsUseThemeBackground
	}
};

resource 'DITL' (400) {
	{	/* array DITLarray: 11 elements */
		/* [1] */
		{160, 212, 180, 280},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{160, 132, 180, 200},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{37, 22, 53, 200},
		RadioButton {
			enabled,
			"Polynomial:   16 Pixels"
		},
		/* [4] */
		{57, 22, 73, 200},
		RadioButton {
			enabled,
			"Spline:          16 Pixels"
		},
		/* [5] */
		{78, 22, 94, 200},
		RadioButton {
			enabled,
			"Spline:          36 Pixels"
		},
		/* [6] */
		{98, 22, 115, 200},
		RadioButton {
			enabled,
			"Sinc:           256 Pixels"
		},
		/* [7] */
		{56, 226, 72, 279},
		StaticText {
			disabled,
			"Faster"
		},
		/* [8] */
		{78, 226, 94, 279},
		StaticText {
			disabled,
                        "Better"
		},
		/* [9] */
		{14, 20, 34, 173},
		StaticText {
			disabled,
			"Interpolation options:"
		},
		/* [10] */
		{125, 20, 142, 81},
		StaticText {
			disabled,
			"Gamma:"
		},
		/* [11] */
		{127, 83, 143, 123},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [12] */
		{72, 194, 104, 226},
		Icon {
			disabled,
                        128
		},
		/* [13] */
		{49, 194, 78, 226},
		Icon {
			disabled,
                        129
		}
	}
};

resource 'dlgx' (400) {
	versionZero {
		kDialogFlagsUseThemeControls | kDialogFlagsHandleMovableModal /*| kDialogFlagsUseControlHierarchy*/ | kDialogFlagsUseThemeBackground
	}
};

resource 'DLOG' (400) {
	{40, 40, 240, 340},
	/*documentProc*/movableDBoxProc,//changed by Kekus Digital
	visible,
	noGoAway,
	0x0,
	400,
	"Bicubic Interpolator"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'dlgx' (115) {
	versionZero {
		kDialogFlagsUseThemeControls | kDialogFlagsHandleMovableModal /*| kDialogFlagsUseControlHierarchy */| kDialogFlagsUseThemeBackground
	}
};

resource 'DITL' (115, "Info") {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{140, 202, 160, 260},
		Button {
			enabled,
			"Stop"
		},
		/* [2] */
		{20, 25, 42, 262},
		StaticText {
			disabled,
			"Static Text"
		},
		/* [3] */
		{53, 25, 122, 261},
		StaticText {
			disabled,
			"Static Text"
		}
	}
};

resource 'DLOG' (115) {
	{40, 40, 220, 320},
	/*documentProc*/noGrowDocProc,//changed by Kekus Digital
	visible,
	noGoAway,
	0x0,
	115,
	"Optimizer Info"
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DITL' (450) {
	{	/* array DITLarray: 18 elements */
		/* [1] */
		{220, 212, 240, 280},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{220, 132, 240, 200},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{2, 178, 13, 279},
		StaticText {
			disabled,
			""
		},
		/* [4] */
		{14, 20, 32, 166},
		StaticText {
			disabled,
			"Point Spread Image:"
		},
		/* [5] */
		{37, 104, 55, 280},
		StaticText {
			disabled,
			""
		},
		/* [6] */
		{35, 20, 55, 98},
		Button {
			enabled,
			"Browse…"
		},
		/* [7] */
		{88, 112, 104, 192},
		RadioButton {
			enabled,
			"Add Blur"
		},
		/* [8] */
		{109, 112, 125, 218},
		RadioButton {
			enabled,
			"Remove Blur"
		},
		/* [9] */
		{65, 70, 82, 110},
		StaticText {
			disabled,
			"Mode:"
		},
		/* [10] */
		{132, 30, 149, 110},
		StaticText {
			disabled,
			"Noise Filter:"
		},
		/* [11] */
		{134, 112, 150, 200},
		RadioButton {
			enabled,
			"Internal"
		},
		/* [12] */
		{155, 112, 171, 183},
		RadioButton {
			enabled,
			"Custom"
		},
		/* [13] */
		{135, 202, 156, 280},
		Button {
			enabled,
			"Browse…"
		},
		/* [14] */
		{180, 25, 197, 110},
		StaticText {
			disabled,
			"Filter Factor:"
		},
		/* [15] */
		{182, 111, 199, 163},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [16] */
		{180, 174, 197, 224},
		StaticText {
			disabled,
			"Frame:"
		},
		/* [17] */
		{182, 223, 199, 275},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [18] */
		{67, 112, 83, 185},
		RadioButton {
			enabled,
			"Scale"
		}
	}
};

resource 'dlgx' (450) {
	versionZero {
		kDialogFlagsUseThemeControls | kDialogFlagsHandleMovableModal /*| kDialogFlagsUseControlHierarchy*/ | kDialogFlagsUseThemeBackground
	}
};

resource 'DLOG' (450) {
	{40, 40, 300, 340},
	/*documentProc*/movableDBoxProc,//changed by Kekus Digital
	visible,
	noGoAway,
	0x0,
	450,
	"Fourier Filter",
	centerParentWindowScreen
};



