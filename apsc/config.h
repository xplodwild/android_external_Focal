
#ifndef APSIFT_CONFIG_H
#define APSIFT_CONFIG_H

#define VERSION_MAJOR 2
#define VERSION_MINOR 5
#define VERSION_PATCH 1

#define PACKAGE_VERSION "2.5.1-Nemesis"

/* Define if PANO13 should be used */
#define PANO13_FOUND 1

#if defined PANO13_FOUND
#define HAS_PANO13 1
#elif defined PANO12_FOUND
#define HAS_PANO12 1
#endif

#endif // APSIFT_CONFIG_H

