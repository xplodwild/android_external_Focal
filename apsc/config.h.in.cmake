
#ifndef APSIFT_CONFIG_H
#define APSIFT_CONFIG_H

#define VERSION_MAJOR ${V_MAJOR}
#define VERSION_MINOR ${V_MINOR}
#define VERSION_PATCH ${V_PATCH}

#define PACKAGE_VERSION "${PACKAGE_VERSION}"

/* Define if PANO13 should be used */
#cmakedefine PANO13_FOUND 1

#if defined PANO13_FOUND
#define HAS_PANO13 1
#elif defined PANO12_FOUND
#define HAS_PANO12 1
#endif

#cmakedefine APPLE

#ifdef APPLE
#define HAS_MALLOC 1
#endif
 
#endif // APSIFT_CONFIG_H

