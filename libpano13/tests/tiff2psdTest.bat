Rem Create PSD files from tiff using PTtiff2PSD
%1 -o crop.psd simpleStitch\reference\tiff_m_cropped0000.tif simpleStitch\reference\tiff_m_cropped0001.tif
%1 -o crop.psb -B simpleStitch\reference\tiff_m_cropped0000.tif simpleStitch\reference\tiff_m_cropped0001.tif
%1 -o uncrop.psd simpleStitch\reference\tiff_m_uncropped0000.tif simpleStitch\reference\tiff_m_uncropped0001.tif
%1 -o uncrop.psb -B simpleStitch\reference\tiff_m_uncropped0000.tif simpleStitch\reference\tiff_m_uncropped0001.tif
rem  Only differences compared to the reference images should be the date created and 
rem    application used to create in the PICT record.
rem  Reference images were created with 32bit version of PTtiff2PSD
pause