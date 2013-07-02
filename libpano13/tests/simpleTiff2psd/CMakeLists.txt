# This file is part of libpano13, the hugin build environment.
# Licence details can be found in the file gpl.txt.
#
# Copyright (c) 2009 Kornel Benko, <Kornel.Benko@berlin.de>
#

SET (TIFFc0 ../simpleStitch/reference/tiff_m0000.tif)
SET (TIFFc1 ../simpleStitch/reference/tiff_m0001.tif)

SET (TIFFu0 ../simpleStitch/reference/tiff_m_uncropped0000.tif)
SET (TIFFu1 ../simpleStitch/reference/tiff_m_uncropped0001.tif)

SET (TIFF16 ../simpleTiff16/reference/tiff_m0000.tif)

project(test3)


add_test(simpleTiff2psd-1-layer ${PERL_EXECUTABLE} "${TOP_SRC_DIR}/tests/panoAutomatePSDtest.pl" simpleStitch_crop_1_layer.psd ${TIFFc0})
add_test(simpleTiff2psd-2-layers ${PERL_EXECUTABLE} "${TOP_SRC_DIR}/tests/panoAutomatePSDtest.pl" simpleStitch_crop_2_layer.psd ${TIFFc0} ${TIFFc1})

add_test(simpleTiff2psd-1-layer-uncropped ${PERL_EXECUTABLE} "${TOP_SRC_DIR}/tests/panoAutomatePSDtest.pl" simpleStitch_uncrop_1_layer.psd ${TIFFu0})
add_test(simpleTiff2psd-2-layers-uncropped ${PERL_EXECUTABLE} "${TOP_SRC_DIR}/tests/panoAutomatePSDtest.pl" simpleStitch_uncrop_2_layer.psd ${TIFFu0} ${TIFFu1})

add_test(simpleTiff16-16bit-1-layer ${PERL_EXECUTABLE} "${TOP_SRC_DIR}/tests/panoAutomatePSDtest.pl" 
         simpleTiff16-16-_uncrop_1_layer.psd ${TIFF16})

add_test(simpleTiff16-16bit-2-layers ${PERL_EXECUTABLE} "${TOP_SRC_DIR}/tests/panoAutomatePSDtest.pl" 
         simpleTiff16-16-_uncrop_2_layer.psd ${TIFF16} ${TIFF16})
