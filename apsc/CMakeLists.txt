###
# CMakeLists.txt for autopano-sift-c 
#
# Major upgrade March 2008 by TKSharpless 
# adds sterographic projection and fast keypoint matching
# I have assigned it version mumber 2.5.0
###

# minimum version set to suppress project developer warning
cmake_minimum_required(VERSION 2.4)

project(autopano-sift-C)

# version
set(V_MAJOR 2)
set(V_MINOR 5)
set(V_PATCH 1)
set(PACKAGE_VERSION ${V_MAJOR}.${V_MINOR}.${V_PATCH})

#use local CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules )

# Put source root on the include search list
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

IF (EXISTS ${PROJECT_SOURCE_DIR}/.svn)
  IF (UPDATE_CHANGELOG)
    EXECUTE_PROCESS(COMMAND "svn2cl" "--group-by-day" "--include-rev" "--linelen=78"
      "--output=${PROJECT_SOURCE_DIR}/ChangeLog" "${PROJECT_SOURCE_DIR}" )
  ENDIF (UPDATE_CHANGELOG)
ENDIF (EXISTS ${PROJECT_SOURCE_DIR}/.svn)

# The parent of root directory, for package searches
GET_FILENAME_COMPONENT( SOURCE_BASE_DIR ${CMAKE_SOURCE_DIR} PATH CACHE )

IF (WIN32)
    # install into place in build-dir
    SET( CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/INSTALL/FILES CACHE FILEPATH "install prefix" FORCE)

  # look in wxWidgets distribution for depending packages.
  SET(wxWidgets_LIB_DIR ${SOURCE_BASE_DIR}/wxWidgets-2.8.10/lib/vc_lib)
  SET(wxWidgets_ROOT_DIR ${SOURCE_BASE_DIR}/wxWidgets-2.8.10)
ENDIF(WIN32)

##
## External libraries
##
FIND_PACKAGE(TIFF REQUIRED)
INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
FIND_PACKAGE(JPEG REQUIRED)
INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})
FIND_PACKAGE(PNG REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})

##
## XML library
##
FIND_PACKAGE(LibXml2 REQUIRED)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})
INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

##
## Panotools
##

FIND_PACKAGE(PANO13 REQUIRED)
INCLUDE_DIRECTORIES(${PANO13_INCLUDE_DIR})

#BUILD SETUP
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

IF(UNIX)
  ADD_DEFINITIONS(-Wall)
  ADD_DEFINITIONS(-DHAS_PANO13)
ENDIF(UNIX)

IF(APPLE)
  SET(HAVE_MALLOC 1)
ENDIF(APPLE)

# Setup MSVC
IF (MSVC)
  # Stop MSVC8 from bitching about the C library
  ADD_DEFINITIONS(/D_CRT_SECURE_NO_DEPRECATE)
  # Stop MSVC9 from bitching about possibly invalid STL usage
  ADD_DEFINITIONS(/D_SCL_SECURE_NO_WARNINGS)
  # Stop MSVC9 from bitching about POSIX names without underscores
  ADD_DEFINITIONS(/D_CRT_NONSTDC_NO_DEPRECATE)
  # compile everything for the static C runtime
  FOREACH(TYPE C CXX)
	FOREACH(CFG "_DEBUG" "_MINSIZEREL" "_RELEASE" "_RELWITHDEBINFO")
      SET(NEW_FLAGS "${CMAKE_${TYPE}_FLAGS${CFG}}")
	  STRING(REPLACE "/MD" "/MT" NEW_FLAGS "${NEW_FLAGS}")
	  SET(CMAKE_${TYPE}_FLAGS${CFG} "${NEW_FLAGS}" CACHE STRING
				"CMAKE_${TYPE}_FLAGS${CFG} (overwritten to ensure static build)" FORCE)
	ENDFOREACH(CFG)
  ENDFOREACH(TYPE)
  
  # yes we DO have pano13
  ADD_DEFINITIONS( /DHAS_PANO13 )

ENDIF(MSVC)

# create config.h file
configure_file(config.h.in.cmake ${CMAKE_BINARY_DIR}/config.h)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

##
## Finally, rules to build the source code!
##

SET(LIBSIFT_SOURCES LoweDetector.c RANSAC.c GaussianConvolution.c
     ScaleSpace.c KeypointXML.c MatchKeys.c KDTree.c BondBall.c
     AreaFilter.c ImageMatchModel.c Transform.c DisplayImage.c ImageMap.c
     HashTable.c ArrayList.c Random.c SimpleMatrix.c Utils.c)

ADD_LIBRARY(libsift STATIC ${LIBSIFT_SOURCES})

set(all_libs libsift ${JPEG_LIBRARIES} ${TIFF_LIBRARIES}
    ${PNG_LIBRARIES} ${ZLIB_LIBRARIES} ${PANO13_LIBRARIES} ${LIBXML2_LIBRARIES}) 

add_executable(generatekeys GenerateKeys.c)
TARGET_LINK_LIBRARIES(generatekeys ${all_libs})

add_executable(autopano AutoPano.c)
TARGET_LINK_LIBRARIES(autopano ${all_libs})

# Nowozin's code has to be compiled as C++ with MSVC
# This used to be global, but now APSCpp has real C files
if ( MSVC )
  SET_TARGET_PROPERTIES( libsift generatekeys autopano 
              PROPERTIES COMPILE_FLAGS /TP 
                        )
endif (MSVC )

install(TARGETS generatekeys autopano DESTINATION bin)

# the code and cmake script for autopano-sift-c are in subdirectory APSCpp
add_subdirectory( APSCpp )

# scripts to run the 2-part version under hugin
# (historical interest only)
if (WIN32)
  install(PROGRAMS autopano-c-complete.vbs DESTINATION bin)
else (WIN32)
  install(PROGRAMS autopano-c-complete.sh DESTINATION bin)
endif (WIN32)


# documentation
FILE(GLOB MAN_PAGES_1 doc/*.1)
FILE(GLOB MAN_PAGES_7 doc/*.7)
install(FILES ${MAN_PAGES_1} DESTINATION share/man/man1) 
install(FILES ${MAN_PAGES_7} DESTINATION share/man/man7) 

## Test exectuables.

if (BUILD_TESTS)
  add_executable(testArray TestArray.c)
  TARGET_LINK_LIBRARIES(testArray ${all_libs})
  add_executable(testRandom TestRandom.c)
  TARGET_LINK_LIBRARIES(testRandom ${all_libs})
#add_executable(testSimpleMatrix TestSimpleMatrix.c)
#TARGET_LINK_LIBRARIES(testSimpleMatrix ${all_libs})
#add_executable(testTransform TestTransform.c)
#TARGET_LINK_LIBRARIES(testTransform ${all_libs})
#add_executable(testRansac TestRansac.c)
#TARGET_LINK_LIBRARIES(testRansac ${all_libs})
#add_executable(testAreaFilter TestAreaFilter.c)
#TARGET_LINK_LIBRARIES(testAreaFilter ${all_libs})
  add_executable(testKDTree TestKDTree.c)
  TARGET_LINK_LIBRARIES(testKDTree ${all_libs})
endif(BUILD_TESTS)
	

SET(CPACK_PACKAGE_VERSION_MAJOR "${V_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${V_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${V_PATCH}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${V_MAJOR}.${V_MINOR}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "autopano-sift-C-${V_MAJOR}.${V_MINOR}.${V_PATCH}")
SET(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
SET(CPACK_SET_DESTDIR "ON")
# Find the revision number and use it as the release in rpm-package-build.
# This way we may omit the otherwise needed "--force" parameter when
# installing from that rpm package.
FIND_PROGRAM(_svnversion svnversion)
EXECUTE_PROCESS(COMMAND ${_svnversion} WORKING_DIRECTORY "${CMAKE_HOME_DIRECTORY}" OUTPUT_VARIABLE CPACK_RPM_PACKAGE_RELEASE OUTPUT_STRIP_TRAILING_WHITESPACE)
SET(CPACK_PACKAGE_CONTACT "hugin-ptx@googlegroups.com")
SET(CPACK_SOURCE_IGNORE_FILES
"/autopano$"
"/generatekeys$"
"/autopano-sift-c$"
"/Makefile$"
"/_CPack_Packages/"
"/CMakeFiles/"
"/CMakeCache.txt$"
"/cmake_install.cmake$"
"/CMakeOutput.log$"
"/CPackConfig.cmake$"
"/CPackSourceConfig.cmake$"
"\\\\.dir/"
"\\\\.tar\\\\.gz$"
"\\\\.tar\\\\.bz2$"
"\\\\.tar\\\\.Z$"
"\\\\.svn/"
"\\\\.cvsignore$"
"\\\\.swp$"
"~$"
"\\\\.#"
"/#"
"/build/"
"/CVS/"
"/\\\\.libs/"
"/\\\\.deps/"
"\\\\.o$"
"\\\\.a$"
"\\\\.lo$"
"\\\\.so$"
"\\\\.so\\\\.0$" 
"\\\\.so\\\\.0\\\\.0$" 
"\\\\.la$"
"Makefile\\\\.in$"
 )
INCLUDE(CPack)
