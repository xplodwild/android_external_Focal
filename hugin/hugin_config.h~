#ifndef __CONFIG_H__

/* various libraries. For compatability with the old source code
 * most cmake variables are defined under a second name as well */

/* Define if you have JPEG library */
#define JPEG_FOUND 1
/* Define if you have JPEG library (old style) */
#ifdef JPEG_FOUND
#define HasJPEG 1
#endif

/* Define if you have PNG library */
#define PNG_FOUND 1
#ifdef PNG_FOUND
#define HasPNG 1
#endif

/* Define if you have TIFF library */
#define TIFF_FOUND 1
#ifdef TIFF_FOUND
#define HasTIFF 1
#endif

/* Define if you have OpenEXR library */
#undef OPENEXR_FOUND
#ifdef OPENEXR_FOUND
#define HasEXR 1
#endif

/* Define if you have Panotools library (pano13) */
#define TLALLI_FOUND 1
#define PANO13_FOUND 1

#ifdef TLALLI_FOUND
#define HasTLALLI 1
#elif defined PANO13_FOUND
#define HasPANO13 1
#endif

/* Define if you have log2 function */
#undef HAVE_LOG2

/* locate of the xrc files, as defined during configuration */
#define INSTALL_LOCALE_DIR "."

/* Location for data, as defined during configuration*/
#define INSTALL_DATA_DIR "./"

/* Location for XRC files and other data, as defined during configuration*/
#define INSTALL_XRC_DIR "./"

/* Use exiv2, if found */
#undef EXIV2_FOUND
#ifdef EXIV2_FOUND
#define HUGIN_USE_EXIV2 1
#endif

/* Build a fully self contained OSX bundle (with embedded ressources) */
#undef MAC_SELF_CONTAINED_BUNDLE

/* contains directory of HuginStitchProject.app, if MAC_SELF_CONTAINED_BUNDLE 
   is not set. */
#define INSTALL_OSX_BUNDLE_DIR "${INSTALL_OSX_BUNDLE_DIR}"

#endif
