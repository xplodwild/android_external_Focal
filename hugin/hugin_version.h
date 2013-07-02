#ifndef __HUGIN_VERSION_H__

#define VERSION_MAJOR 2013
#define VERSION_MINOR 7
#define VERSION_PATCH 0
#define HUGIN_WC_REVISION 0
#define HUGIN_API_VERSION "2013.7"

#if defined (_MSC_VER) || (__APPLE__)
#define PACKAGE_VERSION "${HUGIN_PACKAGE_VERSION} built by ${HUGIN_BUILDER}"
#define DISPLAY_VERSION "${DISPLAY_VERSION} built by ${HUGIN_BUILDER}"
#else
#define PACKAGE_VERSION "2013.7-Nemesis"
#define DISPLAY_VERSION "2013.7-Nemesis"
#endif

/* this is a hg checkout, tag is as such
 * all builds from HG will be considered development versions
 */
#define HUGIN_DEVELOPMENT_VERSION

#endif
