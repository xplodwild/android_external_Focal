This directory performs simple tests in PTmender for all different
types of stitches on 2 8-bit images.

* JPG
* TIFF_m
* TIFF_mask
* PSD
* PSD_nomask

For TIFF_m there are several tests:

* c:NONE
* c:LZW
* c:DEFLATE

And then the same combination with Cropped TIFFs

* r:CROP c:NONE
* r:CROP c:LZW
* r:CROP c:DEFLATE


To run the tests just type:

  make 

If you want to verify the output files again just type:

  make verify



