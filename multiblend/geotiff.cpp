// some defintions for geotiff

#define TIFFTAG_GEOPIXELSCALE   33550
#define TIFFTAG_GEOTIEPOINTS    33922
#define TIFFTAG_GEOTRANSMATRIX  34264  
#define TIFFTAG_GEOKEYDIRECTORY 34735
#define TIFFTAG_GEODOUBLEPARAMS 34736
#define TIFFTAG_GEOASCIIPARAMS  34736

#define TIFFTAG_GDAL_NODATA     42113

static const TIFFFieldInfo xtiffFieldInfo[] = {
  /* XXX Insert Your tags here */
  { TIFFTAG_GEOPIXELSCALE, -1,-1, TIFF_DOUBLE,FIELD_CUSTOM,
    TRUE,TRUE,(char*)"GeoPixelScale" },
  { TIFFTAG_GEOTRANSMATRIX, -1,-1, TIFF_DOUBLE,FIELD_CUSTOM,
    TRUE,TRUE,(char*)"GeoTransformationMatrix" },
  { TIFFTAG_GEOTIEPOINTS, -1,-1, TIFF_DOUBLE,FIELD_CUSTOM,
    TRUE,TRUE,(char*)"GeoTiePoints" },
  { TIFFTAG_GEOKEYDIRECTORY, -1,-1, TIFF_SHORT,FIELD_CUSTOM,
    TRUE,TRUE,(char*)"GeoKeyDirectory" },
  { TIFFTAG_GEODOUBLEPARAMS, -1,-1, TIFF_DOUBLE,FIELD_CUSTOM,
    TRUE,TRUE,(char*)"GeoDoubleParams" },
  { TIFFTAG_GEOASCIIPARAMS, -1,-1, TIFF_ASCII,FIELD_CUSTOM,
    TRUE,FALSE,(char*)"GeoASCIIParams" },
  { TIFFTAG_GDAL_NODATA, -1,-1, TIFF_ASCII,FIELD_CUSTOM,
    TRUE,FALSE,(char*)"GDALNoDataValue" }
};

void geotiff_register(TIFF * tif) {
  TIFFMergeFieldInfo(tif, xtiffFieldInfo, sizeof(xtiffFieldInfo)/sizeof(xtiffFieldInfo[0]));
}

/** Read geotiff tags from an image. Only accept images with correct geocoding.

    Returns  1 if reading was successfull, 0 if it failed.
*/
int geotiff_read(TIFF * tiff, GeoTIFFInfo * info) {
  unsigned short nCount = 0;
  double *geo_scale;
  // clear geotiff info
  memset(info,0,sizeof(GeoTIFFInfo));
  geotiff_register(tiff);

  if (!TIFFGetField(tiff, TIFFTAG_GEOPIXELSCALE, &nCount, &geo_scale) && nCount >= 2 ) {
    return 0;
  }
  info->XCellRes = geo_scale[0];
  info->YCellRes = geo_scale[1];
  double *tiepoints;
  if (!TIFFGetField(tiff, TIFFTAG_GEOTIEPOINTS, &nCount, &tiepoints) && nCount >= 6 ) {
    return 0;
  }
  info->XGeoRef = tiepoints[3] - tiepoints[0]*(geo_scale[0]);
  info->YGeoRef = tiepoints[4] - tiepoints[1]*(geo_scale[1]);
  // TODO: check if tiepoints refer to center of upper left pixel or upper left edge of upper left pixel
  char * nodata;
  if( TIFFGetField(tiff, TIFFTAG_GDAL_NODATA, &nCount, &nodata ) ) {
    info->nodata = atoi(nodata);
  }
  // TODO: read coordinate system definitions...
  return 1;
}

/** Write geotiff tags to an image */
int geotiff_write(TIFF * tiff, GeoTIFFInfo * info) {
  geotiff_register(tiff);
  double scale[3];
  scale[0]=info->XCellRes;
  scale[1]=info->YCellRes;
  scale[2]=0.0;
  TIFFSetField(tiff, TIFFTAG_GEOPIXELSCALE, 3, scale);
  double tiepoint[6];
  tiepoint[0] = tiepoint[1] = tiepoint[2] = tiepoint[5] = 0;
  tiepoint[3] = info->XGeoRef;
  tiepoint[4] = info->YGeoRef;
  TIFFSetField(tiff, TIFFTAG_GEOTIEPOINTS, 6, tiepoint);
  char nodata[50];
  SNPRINTF(nodata,50,"%d",info->nodata);
  TIFFSetField(tiff, TIFFTAG_GDAL_NODATA, nodata);
  return 1;
}
