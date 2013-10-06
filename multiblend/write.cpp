void jpeg_out() {
	int x,y;
	int xp;
	int p=0;
	int i,j;
	int m;
	int mincount;
	int* maskcount=(int*)malloc(g_numimages*sizeof(int));
	int* masklimit=(int*)malloc(g_numimages*sizeof(int));
	int* mask=(int*)malloc(g_numimages*sizeof(int));
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
	JSAMPROW row=(JSAMPROW)malloc(g_workwidth*3);
	uint32 temp;

	cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo,g_jpeg);

	cinfo.image_width=g_workwidth;
	cinfo.image_height=g_workheight;
	cinfo.input_components=3;
	cinfo.in_color_space=JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo,g_jpegquality,true);
	jpeg_start_compress(&cinfo,true);

	if (g_nomask) {
		for (y=0; y<g_workheight; y++) {
			xp=0;
			for (x=0; x<g_workwidth; x++) {
				((uint8*)row)[xp++]=((uint8*)g_out_channels[0].data)[p];
				((uint8*)row)[xp++]=((uint8*)g_out_channels[1].data)[p];
				((uint8*)row)[xp++]=((uint8*)g_out_channels[2].data)[p];
				p++;
			}
  		jpeg_write_scanlines(&cinfo,&row,1);
		}
	} else {
		for (y=0; y<g_workheight; y++) {
			for (i=0; i<g_numimages; i++) {
				mask[i]=MASKOFF;
				if (y>=g_images[i].ypos && y<g_images[i].ypos+g_images[i].height) {
					maskcount[i]=g_images[i].xpos;
					masklimit[i]=g_images[i].xpos+g_images[i].width;
					g_images[i].binary_mask.pointer=&g_images[i].binary_mask.data[g_images[i].binary_mask.rows[y-g_images[i].ypos]];
				} else {
					maskcount[i]=g_workwidth;
					masklimit[i]=g_workwidth;
				}
			}

			x=0;
			xp=0;
			while (x<g_workwidth) {
				m=MASKOFF;
				mincount=g_workwidth-x;
				for (i=0; i<g_numimages; i++) {
					if (maskcount[i]==0) {
						if (x<masklimit[i]) {
							NEXTiMASK(i);
						} else {
							mask[i]=MASKOFF;
							maskcount[i]=mincount;
						}
					}

					if (maskcount[i]<mincount) mincount=maskcount[i];
					if (mask[i]!=MASKOFF) m=MASKON;
				}

				if (m==MASKON) {
					for (j=0; j<mincount; j++) {
						((uint8*)row)[xp++]=((uint8*)g_out_channels[0].data)[p];
						((uint8*)row)[xp++]=((uint8*)g_out_channels[1].data)[p];
						((uint8*)row)[xp++]=((uint8*)g_out_channels[2].data)[p];
						p++;
					}
				} else {
					memset(&((uint8*)row)[xp],0,mincount*3);

					xp+=mincount*3;
					p+=mincount;
				}

				for (i=0; i<g_numimages; i++) maskcount[i]-=mincount;
				x+=mincount;
			}

			jpeg_write_scanlines(&cinfo,&row,1);
		}
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
  fclose(g_jpeg);
}

void tiff_out() {
	int i,j;
	int m;
	int mincount;
	int* maskcount=(int*)malloc(g_numimages*sizeof(int));
	int* masklimit=(int*)malloc(g_numimages*sizeof(int));
	int* mask=(int*)malloc(g_numimages*sizeof(int));
	int rowsperstrip=64;
	int p=0;
	int strips;
	int remaining;
	int strip_p;
	int x,y=0,s;
	int stripy;
	int rows;
  uint16 out[1];
	uint32 temp;
	int mul;

	mul=3;
	if (!g_nomask) mul=4;
	if (g_workbpp==16) mul=mul<<1;
	mul=mul*g_workwidth;

	for (i=0; i<g_numimages; i++) g_images[i].binary_mask.pointer=g_images[i].binary_mask.data;

	void* strip=malloc((rowsperstrip*g_workwidth)<<(g_workbpp>>2));

	TIFFSetField(g_tiff, TIFFTAG_IMAGEWIDTH, g_workwidth);
	TIFFSetField(g_tiff, TIFFTAG_IMAGELENGTH, g_workheight);
	TIFFSetField(g_tiff, TIFFTAG_COMPRESSION, g_compression);
	TIFFSetField(g_tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(g_tiff, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
	TIFFSetField(g_tiff, TIFFTAG_BITSPERSAMPLE, g_workbpp);
	if (g_nomask) {
		TIFFSetField(g_tiff, TIFFTAG_SAMPLESPERPIXEL, 3);
	} else {
		TIFFSetField(g_tiff, TIFFTAG_SAMPLESPERPIXEL, 4);
		out[0] = EXTRASAMPLE_UNASSALPHA;
    TIFFSetField(g_tiff, TIFFTAG_EXTRASAMPLES, 1, &out);
	}
  TIFFSetField(g_tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  if (g_xres!=-1) { TIFFSetField(g_tiff, TIFFTAG_XRESOLUTION, g_xres); TIFFSetField(g_tiff, TIFFTAG_XPOSITION, (float)(g_min_left/g_xres)); }
  if (g_yres!=-1) { TIFFSetField(g_tiff, TIFFTAG_YRESOLUTION, g_yres); TIFFSetField(g_tiff, TIFFTAG_YPOSITION, (float)(g_min_top/g_yres)); }


	if (g_images[0].geotiff.XCellRes) {
		// if we got a georeferenced input, store the geotags in the output
		GeoTIFFInfo info(g_images[0].geotiff);
		info.XGeoRef = g_min_left * g_images[0].geotiff.XCellRes;
		info.YGeoRef = -g_min_top * g_images[0].geotiff.YCellRes;
		output(1,"Output georef: UL: %f %f, pixel size: %f %f\n",info.XGeoRef, info.YGeoRef, info.XCellRes, info.YCellRes);
		geotiff_write(g_tiff, &info);
	}

  strips=(int)((g_workheight+rowsperstrip-1)/rowsperstrip);
	remaining=g_workheight;

	for (s=0; s<strips; s++) {
		rows=min(remaining,rowsperstrip);
    strip_p=0;
		for (stripy=0; stripy<rows; stripy++) {
			if (g_nomask) {
				if (g_workbpp==8) {
					for (x=0; x<g_workwidth; x++) {
						((uint8*)strip)[strip_p++]=((uint8*)g_out_channels[0].data)[p];
						((uint8*)strip)[strip_p++]=((uint8*)g_out_channels[1].data)[p];
						((uint8*)strip)[strip_p++]=((uint8*)g_out_channels[2].data)[p];
						p++;
					}
				} else {
					for (x=0; x<g_workwidth; x++) {
						((uint16*)strip)[strip_p++]=((uint16*)g_out_channels[0].data)[p];
						((uint16*)strip)[strip_p++]=((uint16*)g_out_channels[1].data)[p];
						((uint16*)strip)[strip_p++]=((uint16*)g_out_channels[2].data)[p];
						p++;
					}
				}
			} else {
				for (i=0; i<g_numimages; i++) {
					mask[i]=MASKOFF;
					if (y>=g_images[i].ypos && y<g_images[i].ypos+g_images[i].height) {
						maskcount[i]=g_images[i].xpos;
						masklimit[i]=g_images[i].xpos+g_images[i].width;
						g_images[i].binary_mask.pointer=&g_images[i].binary_mask.data[g_images[i].binary_mask.rows[y-g_images[i].ypos]];
					} else {
						maskcount[i]=g_workwidth;
						masklimit[i]=g_workwidth;
					}
				}

				x=0;
			  while (x<g_workwidth) {
					m=MASKOFF;
					mincount=g_workwidth-x;
					for (i=0; i<g_numimages; i++) {
						if (maskcount[i]==0) {
							if (x<masklimit[i]) {
								NEXTiMASK(i);
							} else {
								mask[i]=MASKOFF;
								maskcount[i]=mincount;
							}
						}

						if (maskcount[i]<mincount) mincount=maskcount[i];
						if (mask[i]!=MASKOFF) m=MASKON;
					}

					if (m==MASKON) {
						if (g_workbpp==8) {
							for (j=0; j<mincount; j++) {
								((uint8*)strip)[strip_p++]=((uint8*)g_out_channels[0].data)[p];
								((uint8*)strip)[strip_p++]=((uint8*)g_out_channels[1].data)[p];
								((uint8*)strip)[strip_p++]=((uint8*)g_out_channels[2].data)[p];
								((uint8*)strip)[strip_p++]=0xff;
								p++;
							}
						} else {
							for (j=0; j<mincount; j++) {
								((uint16*)strip)[strip_p++]=((uint16*)g_out_channels[0].data)[p];
								((uint16*)strip)[strip_p++]=((uint16*)g_out_channels[1].data)[p];
								((uint16*)strip)[strip_p++]=((uint16*)g_out_channels[2].data)[p];
								if (!g_nomask) ((uint16*)strip)[strip_p++]=0xffff;
								p++;
							}
						}
					} else {
						if (g_workbpp==8) {
							memset(&((uint8*)strip)[strip_p],0,mincount<<2);
						} else {
							memset(&((uint16*)strip)[strip_p],0,mincount<<3);
						}
						strip_p+=mincount<<2;
						p+=mincount;
					}
/*
				if (m==MASKON || g_nomask) {
					if (g_workbpp==8) {
						for (j=0; j<mincount; j++) {
							((uint8*)strip)[strip_p++]=((uint8**)g_out_channels)[0][p];
							((uint8*)strip)[strip_p++]=((uint8**)g_out_channels)[1][p];
							((uint8*)strip)[strip_p++]=((uint8**)g_out_channels)[2][p];
							if (!g_nomask) ((uint8*)strip)[strip_p++]=0xff;
							p++;
						}
					} else {
						for (j=0; j<mincount; j++) {
							((uint16*)strip)[strip_p++]=((uint16**)g_out_channels)[0][p];
							((uint16*)strip)[strip_p++]=((uint16**)g_out_channels)[1][p];
							((uint16*)strip)[strip_p++]=((uint16**)g_out_channels)[2][p];
							if (!g_nomask) ((uint16*)strip)[strip_p++]=0xffff;
							p++;
						}
					}
				} else {
					if (g_workbpp==8) {
						if (g_nomask) blank=mincount*3; else blank=mincount<<2;
						memset(&((uint8*)strip)[strip_p],0,blank);
						strip_p+=mincount<<2;
					} else {
						if (g_nomask) blank=mincount*6; else blank=mincount<<3;
						memset(&((uint16*)strip)[strip_p],0,blank);
						strip_p+=mincount<<2;
					}
					p+=mincount;
				}
*/

					for (i=0; i<g_numimages; i++) maskcount[i]-=mincount;
					x+=mincount;
				}
			}
			y++;
		}
    TIFFWriteEncodedStrip(g_tiff,s,strip,rows*mul);

		remaining-=rows;
	}

	TIFFClose(g_tiff);
}
