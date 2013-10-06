#define NEXTiMASK(i) { temp=*g_images[i].binary_mask.pointer++; maskcount[i]=temp&0x7fffffff; mask[i]=~temp&0x80000000; }
#define PREViMASK(i) { temp=*--g_images[i].binary_mask.pointer; maskcount[i]=temp&0x7fffffff; mask[i]=~temp&0x80000000; }

#define MASKON 0
#define MASKOFF 0x80000000

void seam_png(int mode, const char* filename) {
  int x;
  int y;
  int count,i;
	int* maskcount=(int*)malloc(g_numimages*sizeof(int));
	int* masklimit=(int*)malloc(g_numimages*sizeof(int));
	int* mask=(int*)malloc(0x100*sizeof(int));
  int mincount;
  int xorcount;
  int xorimage;
  int stop;
  uint32 temp;
  uint32* seam_p;
  png_structp png_ptr;
  png_infop info_ptr;
  uint8* random;
  FILE* f;

  if (!g_palette) {
    random=(uint8*)malloc(256);
    g_palette=(png_color*)malloc(256*sizeof(png_color));
    for (i=0; i<256; i++) random[i]=rand()&0xff;
    for (i=0; i<65536; i++) { x=rand()&0xff; y=rand()&0xff; temp=random[x]; random[x]=random[y]; random[y]=temp; }
    for (i=0; i<255; i++) { g_palette[i].red=min(255,(random[i]&7)*32+32); g_palette[i].green=min(255,((random[i]&56)>>1)*7+64); g_palette[i].blue=min(255,(random[i]&192)+64); }
    memset(&g_palette[255],0,sizeof(png_color));
  }

  switch (mode) {
    case 0 : output(1,"saving xor map...\n"); break;
    case 1 : output(1,"saving seams...\n"); break;
  }

  fopen_s(&f,filename, "wb");
  if (!f) {
    output(0,"WARNING: couldn't save seam file\n");
    return;
  }

  png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    output(0,"WARNING: PNG create failed\n");
    return;
  }

  info_ptr=png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
    return;
  }

  png_init_io(png_ptr, f);

  png_set_IHDR(png_ptr,info_ptr,g_workwidth,g_workheight,8,PNG_COLOR_TYPE_PALETTE,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
  png_set_PLTE(png_ptr,info_ptr,g_palette,256);

  png_write_info(png_ptr, info_ptr);

  if (mode==0) {
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
		  while (x<g_workwidth) {
			  mincount=g_workwidth-x;
			  xorcount=0;
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
          if (mask[i]!=MASKOFF) {
					  xorcount++;
  					xorimage=i;
				  }
			  }

        stop=x+mincount;
        
        if (xorcount!=1) xorimage=255;

        while (x<stop) {
          ((uint8*)g_line0)[x++]=xorimage;
        }

        for (i=0; i<g_numimages; i++) maskcount[i]-=mincount;
      }

      png_write_row(png_ptr, (uint8*)g_line0);
    }
  }

  if (mode==1) {
    seam_p=g_seams;

    for (y=0; y<g_workheight; y++) {
      x=0;
      while (x<g_workwidth) {
        i=*seam_p&0xff;
        count=*seam_p++>>8;
        memset(&((uint8*)g_line0)[x],i,count);
        x+=count;
      }

      png_write_row(png_ptr, (uint8*)g_line0);
    }
  }

  fclose(f);
}

void load_seams() {
  int x,y;
  int pd,pc;
  int size;
  int a,b;
  int count=1;
  int p=0;
  png_uint_32 pw,ph;
  uint8 sig[8];
  png_structp png_ptr;
  png_infop info_ptr;
  FILE* f;

  fopen_s(&f,g_seamload_filename,"rb");
  if (!f) die("Couldn't open seam file!");

  fread(sig, 1, 8, f);
  //if (!png_check_sig(sig,8)) die("Bad PNG signature!");
  
  png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
  if (!png_ptr) die("PNG problem");
  info_ptr=png_create_info_struct(png_ptr);
  if (!info_ptr) die("PNG problem");

  png_init_io(png_ptr,f);
  png_set_sig_bytes(png_ptr,8);
  png_read_info(png_ptr,info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &pw, &ph, &pd, &pc,NULL,NULL,NULL);

  if (pw!=g_workwidth || ph!=g_workheight) die("PNG dimensions don't match workspace!");
  if (pd!=8 || pc!=PNG_COLOR_TYPE_PALETTE) die("Incorrect seam PNG format!");

	size=(g_numimages*g_workwidth)<<2;
	g_seams=(uint32*)malloc(size*sizeof(uint32));

	for (y=0; y<g_workheight; y++) {
    png_read_row(png_ptr,(png_bytep)g_line0,NULL);
    a=((uint8*)g_line0)[0]&0xff;
		x=1;
		while (x<g_workwidth) {
      b=((uint8*)g_line0)[x++]&0xff;
			if (b!=a) {
				g_seams[p++]=count<<8|a;
        g_images[a].seampresent=true;
				count=1;
				a=b;
			} else {
				count++;
			}
		}
		g_seams[p++]=count<<8|a;
		count=1;

		if ((p+((g_numimages*g_workwidth)<<1))>size) {
			size+=(g_numimages*g_workwidth)<<2;
			g_seams=(uint32*)realloc(g_seams,size*sizeof(uint32));
		}
	}

	g_seams=(uint32*)realloc(g_seams,p*sizeof(uint32));
}

#define EDT_MAX 0xfffffbff
#define VALMASKED(x) (x|mask[x&0xff])

void rightdownxy() {
	int i;
	int x;
	int y;
	int xorcount;
	int mincount=0;
	int stop;
	uint32 temp;
	int* maskcount=(int*)malloc(g_numimages*sizeof(int));
	int* masklimit=(int*)malloc(g_numimages*sizeof(int));
	int* mask=(int*)malloc(0x100*sizeof(int));
	bool lastpixel=false;
	uint32* line;
	uint32 bestval,testval;
	uint32 a,b,c,d;

	mask[255]=MASKOFF;

	y=0;
	while (y<g_workheight) {
	  line=&g_edt[y*g_workwidth];

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
			mincount=g_workwidth-x;
			xorcount=0;
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
        if (mask[i]!=MASKOFF) {
					xorcount++;
//					xorimage=i;
				}
			}

			stop=x+mincount;

			if (xorcount==1) x=stop;
			else {
// if we're on the top line:
				if (y==0) {
					if (x==0) x=1;
					while (x<stop) {
						bestval=line[x];

						testval=VALMASKED(line[x-1])+(3<<8); // changed to -1
						if (testval<bestval) bestval=testval;

						if (bestval&MASKOFF && xorcount!=0) {
							for (i=0; i<g_numimages; i++) {
								if (mask[i]==MASKON) {
									g_seamwarning=true;
									bestval=MASKOFF|i;
									if (!g_reverse) break;
								}
							}
						}

						line[x++]=bestval;
					}
				} else {
// if we're not on the top line
					if (x==0) {
						testval=VALMASKED(line[-g_workwidth])+(3<<8);
						bestval=min(line[x],testval);

						testval=VALMASKED(line[-g_workwidth+1])+(4<<8);
						if (testval<bestval) bestval=testval;

						if (bestval&MASKOFF && xorcount!=0) {
							for (i=0; i<g_numimages; i++) {
								if (mask[i]==MASKON) {
									g_seamwarning=true;
									bestval=MASKOFF|i;
									if (!g_reverse) break;
								}
							}
						}

						line[x++]=bestval;
					}

					if (stop==g_workwidth) {
						stop=g_workwidth-1;
						lastpixel=true;
					}

/* abc
   dx  */
					if (x<stop) {
						a=VALMASKED(line[-g_workwidth+x-1])+(4<<8);
						b=VALMASKED(line[-g_workwidth+x])+(3<<8);
						d=VALMASKED(line[x-1])+(3<<8);

						while (x<stop) { // main bit
							temp=line[-g_workwidth+x+1];
							c=VALMASKED(temp)+(4<<8);

							bestval=line[x];
							if (a<bestval) bestval=a;
							if (b<bestval) bestval=b;
							if (c<bestval) bestval=c;
							if (d<bestval) bestval=d;
	
							if (bestval&MASKOFF && xorcount!=0) {
								for (i=0; i<g_numimages; i++) {
									if (mask[i]==MASKON) {
										g_seamwarning=true;
										bestval=MASKOFF|i;
  									if (!g_reverse) break;
									}
								}
							}
						
							line[x++]=bestval;

							a=b+(1<<8);
							b=c-(1<<8);
							d=bestval+(3<<8);
						}
					}

					if (lastpixel) {
						testval=VALMASKED(line[-g_workwidth+x])+(3<<8);
						bestval=min(line[x],testval);

						testval=VALMASKED(line[-g_workwidth+x-1])+(4<<8);
						if (testval<bestval) bestval=testval;

						testval=VALMASKED(line[x-1])+(3<<8);
						if (testval<bestval) bestval=testval;

						if (bestval&MASKOFF && xorcount!=0) {
							for (i=0; i<g_numimages; i++) {
								if (mask[i]==MASKON) {
									g_seamwarning=true;
									bestval=MASKOFF|i;
									break;
									if (!g_reverse) break;
								}
							}
						}

						line[x++]=bestval;

						lastpixel=false;
					}
				}
			}

			for (i=0; i<g_numimages; i++) maskcount[i]-=mincount;
		}
		y++;
	}
}

void leftupxy() {
	int i;
	int x,y;
	int xorcount;
	int xorimage;
	int mincount=0;
	int stop;
	uint32 temp;
	int* maskcount=(int*)malloc(g_numimages*sizeof(int));
	int* masklimit=(int*)malloc(g_numimages*sizeof(int));
	int* mask=(int*)malloc(0x100*sizeof(int));
	bool lastpixel=false;
	uint32* line;
	uint32 bestval,testval;
	uint32 a,b,c,d;

	mask[255]=MASKOFF;

	y=g_workheight-1;
	while (y>=0) {
	  line=&g_edt[y*g_workwidth];

		for (i=0; i<g_numimages; i++) {
			mask[i]=MASKOFF;
      if (y>=g_images[i].ypos && y<g_images[i].ypos+g_images[i].height) {
				maskcount[i]=g_workwidth-(g_images[i].xpos+g_images[i].width);
				masklimit[i]=g_images[i].xpos;
				g_images[i].binary_mask.pointer=&g_images[i].binary_mask.data[g_images[i].binary_mask.rows[y-g_images[i].ypos+1]]; // point to END of line
			} else {
				maskcount[i]=g_workwidth;
				masklimit[i]=g_workwidth;
			}
		}

		x=g_workwidth-1;
		while (x>=0) {
			mincount=(x+1);
			xorcount=0;
			for (i=0; i<g_numimages; i++) {
				if (maskcount[i]==0) {
					if (x>=masklimit[i]) {
						PREViMASK(i);
					} else {
						mask[i]=MASKOFF;
						maskcount[i]=mincount;
					}
				}

				if (maskcount[i]<mincount) mincount=maskcount[i];
        if (mask[i]!=MASKOFF) {
					xorcount++;
					xorimage=i;
				}
			}

			stop=x-mincount;

			if (xorcount==1) {
				g_images[xorimage].seampresent=true;
				while (x>stop) line[x--]=xorimage;
			} else {
// if we're on the bottom line:
				if (y==g_workheight-1) {
					if (x==g_workwidth-1) {
						while (x>stop) line[x--]=EDT_MAX;
					} else
					while (x>stop) {
						testval=VALMASKED(line[x+1])+(3<<8);
						line[x--]=min(testval,EDT_MAX);
					}
				} else {
// if we're not on the bottom line
					if (x==g_workwidth-1) {
						testval=VALMASKED(line[+g_workwidth+x])+(3<<8);
						bestval=min(EDT_MAX,testval);

						testval=VALMASKED(line[+g_workwidth+x-1])+(4<<8);
						if (testval<bestval) bestval=testval;

						line[x--]=bestval;
					}

					if (stop==-1) {
						stop=0;
						lastpixel=true;
					}

/*  xd
   abc */
					if (x>stop) {
						b=VALMASKED(line[+g_workwidth+x])+(3<<8);
						c=VALMASKED(line[+g_workwidth+x+1])+(4<<8);
						d=VALMASKED(line[x+1])+(3<<8);
					}

					while (x>stop) { // main bit
						temp=line[+g_workwidth+x-1];
						a=VALMASKED(temp)+(4<<8);

						bestval=EDT_MAX;
						if (a<bestval) bestval=a;
						if (b<bestval) bestval=b;
						if (c<bestval) bestval=c;
						if (d<bestval) bestval=d;

						line[x--]=bestval;

						c=b+(1<<8);
						b=a-(1<<8);
						d=bestval+(3<<8);
					}

					if (lastpixel) {
						testval=VALMASKED(line[+g_workwidth+x])+(3<<8);
						bestval=min(EDT_MAX,testval);

						testval=VALMASKED(line[+g_workwidth+x+1])+(4<<8);
						bestval=min(bestval,testval);

						testval=VALMASKED(line[x+1])+(3<<8);
						bestval=min(bestval,testval);

						line[x--]=bestval;

						lastpixel=false;
					}
				}
			}

			for (i=0; i<g_numimages; i++) {
				maskcount[i]-=mincount;
			}
		}
		y--;
	}
}

void make_seams() {
	int x,y;
	int p=0;
	int size;
	int count=1;
	int a,b;
	uint32* line;

	size=(g_numimages*g_workheight)<<2; // was g_workwidth<<3
	g_seams=(uint32*)malloc(size*sizeof(uint32));

	for (y=0; y<g_workheight; y++) {
		line=&g_edt[y*g_workwidth];
    a=line[0]&0xff;
		x=1;
		while (x<g_workwidth) {
      b=line[x++]&0xff;
			if (b!=a) {
				g_seams[p++]=count<<8|a;
				count=1;
				a=b;
			} else {
				count++;
			}
		}
		g_seams[p++]=count<<8|a;
		count=1;

//		if ((p+(g_workwidth<<3))>size) {
//			size+=g_workwidth<<4;
		if ((p+((g_numimages*g_workheight)<<1))>size) {
			size+=(g_numimages*g_workheight)<<2;
			g_seams=(uint32*)realloc(g_seams,size*sizeof(uint32));
		}
	}

	g_seams=(uint32*)realloc(g_seams,p*sizeof(uint32));
}

void seam() {
	int i;

	output(1,"seaming...\n");
	for (i=0; i<g_numimages; i++) g_images[i].seampresent=false;

  if (!g_seamload_filename) {
  	g_edt=(uint32*)swap_aligned_malloc(g_workwidth*g_workheight*sizeof(uint32),0); // if malloc fails fall back on dtcomp

  	if (!g_edt) die("not enough memory to create seams");

	  leftupxy();
	  rightdownxy();

    if (g_xor_filename) seam_png(0,g_xor_filename);

  	make_seams();

    if (g_seamsave_filename) seam_png(1,g_seamsave_filename);

	  for (i=0; i<g_numimages; i++) {
		  if (!g_images[i].seampresent) {
			output(0, "WARNING: some images completely overlapped\n");
		    break;
      }
	  }
	  if (g_seamwarning) output(0, "WARNING: some image areas have been arbitrarily assigned\n");
  } else {
    load_seams();

    for (i=0; i<g_numimages; i++) {
      if (!g_images[i].seampresent) {
    	  output(0, "WARNING: some images not present in seam bitmap\n");
        break;
      }
    }
  }

	_aligned_free(g_edt);
}
