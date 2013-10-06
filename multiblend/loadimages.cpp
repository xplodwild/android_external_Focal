//#define EXTRAS yes
#ifdef EXTRAS
void ppm_out(uint8* r, uint8* g, uint8* b, char *filename, int w, int h) {
  int p=0;
  int x,y;
  uint8* line=(uint8*)malloc(3*w);
  FILE *f;

  fopen_s(&f,filename,"wb");
  fprintf(f,"P6 %d %d 255 ",w,h);

  for (y=0; y<h; y++) {
    for (x=0; x<w; x++) {
      line[x*3]=r[p];
      line[x*3+1]=g[p];
      line[x*3+2]=b[p];
      p++;
    }
    fwrite(line,3,w,f);
  }
  fclose(f);
}

void pgm_out(uint32* bitmap, char *filename, int w, int h) {
  int p=0;
  int q;
  int x,y;
  uint8* line=(uint8*)malloc(w);
  FILE *f;
  
  fopen_s(&f,filename,"wb");
  fprintf(f,"P5 %d %d 255 ",w,h);

  for (y=0; y<h; y++) {
    for (x=0; x<w; x++) {
      if (bitmap[p]>512) q=255; else q=bitmap[p]&0xff;
      line[x]=q;
      p++;
    }
    fwrite(line,1,w,f);
  }

  fclose(f);
}

void ascii_output(uint8* bitmap, int w, int h, int channel) {
  int x,y;
  int row_offset,offset;
  int pixel,c;
  const char* alphabet=" .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
  double z=strlen(alphabet)*(1.0/256);

  for (y=0; y<22; y++) {
    row_offset=(int)(y*(1.0/22)*h)*w;
    for (x=0; x<80; x++) {
      offset=row_offset+(int)(x*(1.0/80)*w);
      pixel=bitmap[(offset)+channel]&0xff; // offset<<2 if we're looking at interleaved data
      c=(int)(pixel*z);
      putchar(alphabet[c]);
    }
  }
}

void ascii_output16(uint16* bitmap, int w, int h) {
  int x,y;
  int row_offset,offset;
  int pixel,c;
  const char* alphabet=" .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
  double z=strlen(alphabet)*(1.0/256);

  for (y=0; y<22; y++) {
    row_offset=(int)(y*(1.0/22)*h)*w;
    for (x=0; x<80; x++) {
      offset=row_offset+(int)(x*(1.0/80)*w);
      pixel=bitmap[offset]>>8; // offset<<2 if we're looking at interleaved data
      c=(int)(pixel*z);
      putchar(alphabet[c]);
    }
  }
}
#endif

void trim8(void* bitmap, uint32 w, uint32 h, int bpp, int* top, int* left, int* bottom, int* right) {
  size_t p;
  int x,y;
  uint32* b=(uint32*)bitmap;

// find first solid pixel
  x=0; y=0; p=0;
  while (p<(w*h)) {
    if (b[p++]>0x00ffffff) { // or maybe 0x00ffffff
      *top=y;
      *left=x;
      break;
    }
    x++; if (x==w) { x=0; y++; }
  }

// find last solid pixel
  x=w-1; y=h-1; p=w*h-1;
  while (p>=0) {
    if (b[p--]>0x00ffffff) {
      *bottom=y;
      *right=x;
      break;
    }
    x--; if (x<0) { x=w-1; y--; }
  }

/*	if (*left>*right) {
		p=*left;
		*left=*right;
		*right=p;
	} */

  b+=*top*w;
  for (y=*top; y<=*bottom; y++) {
    for (x=0; x<*left; x++) {
      if (b[x]>0x00ffffff) {
        *left=x;
        break;
      }
    }
    for (x=w-1; x>*right; x--) {
      if (b[x]>0x00ffffff) {
        *right=x;
        break;
      }
    }
    b+=w;
  }
}

void trim16(void* bitmap, uint32 w, uint32 h, int bpp, int* top, int* left, int* bottom, int* right) {
  size_t p;
  int x,y;
  uint16* b=(uint16*)bitmap;

// find first solid pixel
  x=0; y=0; p=3;
  while (p<(w*h)) {
    if (b[p]!=0x0000) { // or maybe 0x00ffffff
      *top=y;
      *left=x;
      break;
    }
		p+=4;
    x++; if (x==w) { x=0; y++; }
  }

// find last solid pixel
  x=w-1; y=h-1; p=w*h*4-1;
  while (p>=0) {
    if (b[p]!=0x0000) {
      *bottom=y;
      *right=x;
      break;
    }
		p-=4;
    x--; if (x<0) { x=w-1; y--; }
  }

	b+=*top*w*4;
  for (y=*top; y<=*bottom; y++) {
    for (x=0; x<*left; x++) {
      if (b[x*4+3]!=0x0000) {
        *left=x;
        break;
      }
    }
    for (x=w-1; x>*right; x--) {
      if (b[x*4+3]!=0x0000) {
        *right=x;
        break;
      }
    }
    b+=w*4;
  }
}

void trim(void* bitmap, int w, int h, int bpp, int* top, int* left, int* bottom, int* right) {
  if (bpp==8) trim8(bitmap,w,h,bpp,top,left,bottom,right); else trim16(bitmap,w,h,bpp,top,left,bottom,right);
}

void extract8(struct_image* image, void* bitmap, int uw, int uh) {
  int x,y;
	int c;
  size_t p,up;
  int mp=0;
  int masklast=-1,maskthis;
  int maskcount=0;
  size_t temp;
  uint32 pixel;

  image->binary_mask.rows=(uint32*)malloc((image->height+1)*sizeof(uint32));

  up=image->ypos*uw+image->xpos;

  temp=image->width*image->height*sizeof(uint8);
	image->channels=new channel[g_numchannels];
	for (c=0; c<g_numchannels; c++) {
		image->channels[c].bytes=temp;
		if ((image->channels[c].data=malloc(temp))==NULL) {
			die("couldn't malloc iamge channel (%d)",c);
		}
	}

  p=0;
  for (y=0; y<image->height; y++) {
    image->binary_mask.rows[y]=mp;

		pixel=((uint32*)bitmap)[up++];
    if (pixel>0xfeffffff) { // pixel is solid
      ((uint8*)image->channels[0].data)[p]=pixel&0xff;
      ((uint8*)image->channels[1].data)[p]=(pixel>>8)&0xff;
      ((uint8*)image->channels[2].data)[p]=(pixel>>16)&0xff;
      masklast=1;
    } else {
			masklast=0;
		}
    maskcount=1;
	  p++;

		for (x=1; x<image->width; x++) {
      pixel=((uint32*)bitmap)[up++];
      if (pixel>0xfeffffff) { // pixel is solid
				((uint8*)image->channels[0].data)[p]=pixel&0xff;
				((uint8*)image->channels[1].data)[p]=(pixel>>8)&0xff;
				((uint8*)image->channels[2].data)[p]=(pixel>>16)&0xff;
        maskthis=1;
      } else maskthis=0;
      if (maskthis!=masklast) {
        ((uint32*)bitmap)[mp++]=masklast<<31|maskcount;
        masklast=maskthis;
        maskcount=1;
      } else maskcount++;
      p++;
    }
    ((uint32*)bitmap)[mp++]=masklast<<31|maskcount;
    up+=uw-image->width;
  }
	image->binary_mask.rows[y]=mp;

  image->binary_mask.data=(uint32*)malloc(mp<<2);
  temp=mp;

	memcpy(image->binary_mask.data,bitmap,mp<<2);
}

void extract16(struct_image* image, void* bitmap, int uw, int uh) {
  int x,y;
	int c;
  size_t p,up;
  int mp=0;
  int masklast=-1,maskthis;
  int maskcount=0;
  size_t temp;
	int mask;

  image->binary_mask.rows=(uint32*)malloc((image->height+1)*sizeof(uint32));

  up=(image->ypos*uw+image->xpos)*4;

  temp=image->width*image->height*sizeof(uint16);
	image->channels=new channel[g_numchannels];
	for (c=0; c<g_numchannels; c++) {
		image->channels[c].bytes=temp;
		if ((image->channels[c].data=malloc(temp))==NULL) {
			die("couldn't malloc image channel (%d)",c);
		}
	}

  p=0;
  for (y=0; y<image->height; y++) {
    image->binary_mask.rows[y]=mp;

    mask=((uint16*)bitmap)[up+3];
    if (mask==0xffff) { // pixel is 100% opaque
			((uint16*)image->channels[0].data)[p]=((uint16*)bitmap)[up+2];
      ((uint16*)image->channels[1].data)[p]=((uint16*)bitmap)[up+1];
      ((uint16*)image->channels[2].data)[p]=((uint16*)bitmap)[up];
      masklast=1;
    } else {
			masklast=0;
		}
    maskcount=1;

		up+=4;
		p++;

		for (x=1; x<image->width; x++) {
      mask=((uint16*)bitmap)[up+3];
      if (mask==0xffff) { // pixel is 100% opaque
        ((uint16*)image->channels[0].data)[p]=((uint16*)bitmap)[up+2];
        ((uint16*)image->channels[1].data)[p]=((uint16*)bitmap)[up+1];
        ((uint16*)image->channels[2].data)[p]=((uint16*)bitmap)[up];
        maskthis=1;
      } else {
				maskthis=0;
			}
      if (maskthis!=masklast) {
        ((uint32*)bitmap)[mp++]=masklast<<31|maskcount;
        masklast=maskthis;
        maskcount=1;
      } else maskcount++;

			up+=4;
      p++;
    }
    ((uint32*)bitmap)[mp++]=masklast<<31|maskcount;
    up+=(uw-image->width)*4;
  }
	image->binary_mask.rows[y]=mp;

  image->binary_mask.data=(uint32*)malloc(mp<<2);
  temp=mp;

	memcpy(image->binary_mask.data,bitmap,mp<<2);
}

void extract(struct_image* image, void* bitmap, int uw, int uh) {
  if (image->bpp==8) extract8(image,bitmap,uw,uh); else extract16(image,bitmap,uw,uh);
}

#define NEXTMASK { mask=*mask_pointer++; maskcount=mask&0x7fffffff; mask=mask>>31; }
#define PREVMASK { mask=*--mask_pointer; maskcount=mask&0x7fffffff; mask=mask>>31; }

void inpaint8(struct_image* image, uint32* edt) {
  int x,y;
	int c;
  uint32* edt_p=edt;
  uint32* mask_pointer=image->binary_mask.data;
  int maskcount,mask;
  uint32 dist,temp_dist;
  int copy,temp_copy;
	uint8** chan_pointers=(uint8**)malloc(g_numchannels*sizeof(uint8*));
	int* p=(int*)malloc(g_numchannels*sizeof(int));
  bool lastpixel;

	for (c=0; c<g_numchannels; c++) chan_pointers[c]=(uint8*)image->channels[c].data;

// top-left to bottom-right
// first line, first block
  x=0;

  NEXTMASK;
  dist=(1-mask)<<31;
  for (; maskcount>0; maskcount--) edt_p[x++]=dist;

// first line, remaining blocks in first row
  while (x<image->width) {
    NEXTMASK;
    if (mask) {
      for (; maskcount>0; maskcount--) edt_p[x++]=0;
    } else { // mask if off, so previous mask must have been on
      dist=0;
			for (c=0; c<g_numchannels; c++) p[c]=chan_pointers[c][x-1];
      for (; maskcount>0; maskcount--) {
        dist+=2;
				for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=p[c];
        edt_p[x++]=dist;
      }
    }
  }

  for (y=image->height; y>1; y--) {
    lastpixel=false;
    edt_p+=image->width;
		for (c=0; c<g_numchannels; c++) chan_pointers[c]+=image->width;
    x=0;

    while (x<image->width) {
//			printf("%d",x);
      NEXTMASK;
      if (mask) {
        for (; maskcount>0; maskcount--) edt_p[x++]=0;
      } else {
        if (x==0) {
          copy=x-image->width+1;
          dist=edt_p[copy]+3;

          temp_copy=x-image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }
          
					for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
          edt_p[x++]=dist;
          maskcount--;
        }
        if (x+maskcount==image->width) {
          lastpixel=true;
          maskcount--;
        }

        for (; maskcount>0; maskcount--) {
          dist=edt_p[x-1]+2;
          copy=x-1;

          temp_copy=x-image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x-image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }
          
          temp_copy=x-image->width+1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (dist<0x10000000) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
          }
          edt_p[x++]=dist; // dist
        }
        if (lastpixel) {
          dist=edt_p[x-1]+2;
          copy=x-1;

          temp_copy=x-image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x-image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }
          
          if (dist<0x10000000) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
          }
          edt_p[x++]=dist;
        }
      }
    }
  }

// bottom-right to top-left
  // last line

  while (x>0) {
    PREVMASK;
    if (mask) {
      x-=maskcount;
    } else {
      if (x==image->width) {
        x--;
        maskcount--;
      }
			for (c=0; c<g_numchannels; c++) p[c]=chan_pointers[c][x];
      for (; maskcount>0; maskcount--) {
        dist=edt_p[x]+2;
        x--;
        if (dist<edt_p[x]) {
          edt_p[x]=dist;
					for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=p[c];
        } else {
    			for (c=0; c<g_numchannels; c++) p[c]=chan_pointers[c][x];
        }
      }
    }
  }

// remaining lines
  for (y=image->height; y>1; y--) {
    lastpixel=false;
    edt_p-=image->width;
		for (c=0; c<g_numchannels; c++) chan_pointers[c]-=image->width;
    x=image->width-1;

    while (x>=0) {
      PREVMASK;
      if (mask) {
        x-=maskcount;
      } else {
        if (x==image->width-1) {
          dist=edt_p[x];
          copy=0;

          temp_copy=x+image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (copy!=0) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
            edt_p[x--]=dist;
          } else x--;
          maskcount--;
        }
        if (x-maskcount==-1) {
          lastpixel=true;
          maskcount--;
        }
        for (; maskcount>0; maskcount--) {
          dist=edt_p[x];
          copy=0;

          temp_copy=x+1;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width+1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (copy!=0) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
            edt_p[x--]=dist;
          } else x--;
        }
        if (lastpixel) {
          dist=edt_p[x];
          copy=0;

          temp_copy=x+1;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width+1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (copy!=0) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
            edt_p[x--]=dist;
          } else x--;
        }
      }
    }
  }
}

void inpaint16(struct_image* image, uint32* edt) {
  int x,y;
	int c;
  uint32* edt_p=edt;
  uint32* mask_pointer=image->binary_mask.data;
  int maskcount,mask;
  uint32 dist,temp_dist;
  int copy,temp_copy;
	uint8** chan_pointers=(uint8**)malloc(g_numchannels*sizeof(uint8*));
	int* p=(int*)malloc(g_numchannels*sizeof(int));
  bool lastpixel;

	for (c=0; c<g_numchannels; c++) chan_pointers[c]=(uint8*)image->channels[c].data;

// top-left to bottom-right
// first line, first block
  x=0;
  
  NEXTMASK;
  dist=(1-mask)<<31;
  for (; maskcount>0; maskcount--) edt_p[x++]=dist;

// first line, remaining blocks in first row
  while (x<image->width) {
    NEXTMASK;
    if (mask) {
      for (; maskcount>0; maskcount--) edt_p[x++]=0;
    } else { // mask if off, so previous mask must have been on
      dist=0;
			for (c=0; c<g_numchannels; c++) p[c]=chan_pointers[c][x-1];
      for (; maskcount>0; maskcount--) {
        dist+=2;
				for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=p[c];
        edt_p[x++]=dist;
      }
    }
  }

  for (y=image->height; y>1; y--) {
    lastpixel=false;
    edt_p+=image->width;
		for (c=0; c<g_numchannels; c++) p[c]+=image->width;
    x=0;

    while (x<image->width) {
      NEXTMASK;
      if (mask) {
        for (; maskcount>0; maskcount--) edt_p[x++]=0;
      } else {
        if (x==0) {
          copy=x-image->width+1;
          dist=edt_p[copy]+3;

          temp_copy=x-image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }
          
					for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
          edt_p[x++]=dist;
          maskcount--;
        }
        if (x+maskcount==image->width) {
          lastpixel=true;
          maskcount--;
        }

        for (; maskcount>0; maskcount--) {
          dist=edt_p[x-1]+2;
          copy=x-1;

          temp_copy=x-image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x-image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }
          
          temp_copy=x-image->width+1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (dist<0x10000000) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
          }
          edt_p[x++]=dist; // dist
        }
        if (lastpixel) {
          dist=edt_p[x-1]+2;
          copy=x-1;

          temp_copy=x-image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x-image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }
          
          if (dist<0x10000000) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
          }
          edt_p[x++]=dist;
        }
      }
    }
  }

// bottom-right to top-left
  // last line

  while (x>0) {
    PREVMASK;
    if (mask) {
      x-=maskcount;
    } else {
      if (x==image->width) {
        x--;
        maskcount--;
      }
			for (c=0; c<g_numchannels; c++) p[c]=chan_pointers[c][x];
      for (; maskcount>0; maskcount--) {
        dist=edt_p[x]+2;
        x--;
        if (dist<edt_p[x]) {
          edt_p[x]=dist;
					for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=p[c];
        } else {
    			for (c=0; c<g_numchannels; c++) p[c]=chan_pointers[c][x];
        }
      }
    }
  }

// remaining lines
  for (y=image->height; y>1; y--) {
    lastpixel=false;
    edt_p-=image->width;
		for (c=0; c<g_numchannels; c++) chan_pointers[c]-=image->width;
    x=image->width-1;

    while (x>=0) {
      PREVMASK;
      if (mask) {
        x-=maskcount;
      } else {
        if (x==image->width-1) {
          dist=edt_p[x];
          copy=0;

          temp_copy=x+image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (copy!=0) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
            edt_p[x--]=dist;
          } else x--;
          maskcount--;
        }
        if (x-maskcount==-1) {
          lastpixel=true;
          maskcount--;
        }
        for (; maskcount>0; maskcount--) {
          dist=edt_p[x];
          copy=0;

          temp_copy=x+1;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width+1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (copy!=0) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
            edt_p[x--]=dist;
          } else x--;
        }
        if (lastpixel) {
          dist=edt_p[x];
          copy=0;

          temp_copy=x+1;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width+1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (copy!=0) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
            edt_p[x--]=dist;
          } else x--;
        }
      }
    }
  }
}

void inpaint(struct_image* image, uint32* edt) {
	if (image->bpp==8) inpaint8(image,edt); else inpaint16(image,edt);
}

bool load_image(struct_image *image) {
  uint32 rowsperstrip;
  uint32 strip;
  uint32 minstripsize;
  uint32 minstripcount=0;
  uint32 temp;
  uint32 first_strip,last_strip;
  uint32 tiff_width,tiff_height;
  int tiff_xoff=0,tiff_yoff=0;
  int top,left,bottom,right;
  int rowscut;
  float tiff_xpos,tiff_ypos;
  float tiff_xres,tiff_yres;

  uint16 bpp,spp;
  uint16 compression;

  output(1,"loading %s...\n",image->filename);
  TIFF* tiff = TIFFOpen(image->filename, "r");
  if (tiff==NULL) {
    die("couldn't open file");
  }

  if (!TIFFGetField(tiff, TIFFTAG_XPOSITION, &tiff_xpos)) tiff_xpos=-1;
  if (!TIFFGetField(tiff, TIFFTAG_YPOSITION, &tiff_ypos)) tiff_ypos=-1;
  if (!TIFFGetField(tiff, TIFFTAG_XRESOLUTION, &tiff_xres)) tiff_xres=-1;
  if (!TIFFGetField(tiff, TIFFTAG_YRESOLUTION, &tiff_yres)) tiff_yres=-1;
  TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &tiff_width);
  TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &tiff_height);
  TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &bpp);
  TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &spp);
  TIFFGetField(tiff, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
  TIFFGetField(tiff, TIFFTAG_COMPRESSION, &compression);

  if (bpp!=8 && bpp!=16) { output(1,"%dbpp not valid!\n",bpp); return false; }
  if (spp!=4) { output(1,"can't handle <>4spp!\n"); return false; }

// read geotiff offsets
  g_images[g_numimages].geotiff.XCellRes=0;
  if (tiff_xpos==-1 && tiff_ypos==-1) {
		// try to read geotiff tags
		if (geotiff_read(tiff, &(image->geotiff))) {
			tiff_xoff=(int)(image->geotiff.XGeoRef/g_images[0].geotiff.XCellRes);
			tiff_yoff=(int)(-image->geotiff.YGeoRef/g_images[0].geotiff.YCellRes);
		}
	} else {
		if (tiff_xpos!=-1 && tiff_xres>0) tiff_xoff=(int)(tiff_xpos*tiff_xres+0.5);
		if (tiff_ypos!=-1 && tiff_yres>0) tiff_yoff=(int)(tiff_ypos*tiff_yres+0.5);
		if (g_xres==-1 || g_yres==-1) {
			g_xres=tiff_xres;
			g_yres=tiff_xres;
		} else if (g_xres!=tiff_xres || g_yres!=tiff_yres) {
			output(0, "WARNING: Image resolution mismatch (%f %f/%f %f)!\n", g_xres, g_yres,tiff_xres, tiff_yres);
		}
	}

//  if (tiff_xpos!=-1 && tiff_xres>0) tiff_xoff=(int)(tiff_xpos*tiff_xres+0.5); else tiff_xoff=0;
//  if (tiff_ypos!=-1 && tiff_yres>0) tiff_yoff=(int)(tiff_ypos*tiff_yres+0.5); else tiff_yoff=0;

  first_strip=0;
  last_strip=TIFFNumberOfStrips(tiff)-1;

  if (compression!=1) {
    minstripsize=0xffffffff;
    for (strip=0; strip<TIFFNumberOfStrips(tiff)-1; strip++) { // don't look at the last strip; it may be short
      temp=(uint32)TIFFRawStripSize(tiff,strip);
      if (temp==minstripsize) minstripcount++;
      else if (temp<minstripsize) { minstripsize=temp; minstripcount=1; }
    }
    temp=(uint32)TIFFRawStripSize(tiff,strip);

    if (minstripcount>2) {

      first_strip=-1;
      for (strip=0; strip<TIFFNumberOfStrips(tiff)-1; strip++) {
        temp=(uint32)TIFFRawStripSize(tiff,strip);
        if (temp>minstripsize) {
          if (first_strip==-1) first_strip=strip;
          last_strip=strip;
        }
      }
			if (first_strip==-1) first_strip=0;
      if (last_strip==strip-1) last_strip=strip; // if the penultimate strip was non-empty, assume the final strip is too
    }
  } else output(1,"uncompressed image; smartcropping disabled\n");

	rowscut=first_strip*rowsperstrip;

// allocate enough space for strips first_strip-to-last_strip and read into memory
  void* untrimmed;
  uint32 untrimmed_height;

	if (last_strip==TIFFNumberOfStrips(tiff)-1) {
    untrimmed_height=tiff_height-rowsperstrip*first_strip;
  } else {
    untrimmed_height=(last_strip+1-first_strip)*rowsperstrip;
  }

	untrimmed=_TIFFmalloc((tiff_width*untrimmed_height)<<(bpp>>2));
  if (!untrimmed) die("Couldn't allocate enough memory to load image");
  char* pointer=(char*)untrimmed;

	for (strip=first_strip; strip<=last_strip; strip++) {
    TIFFReadEncodedStrip(tiff,strip,pointer,-1);
    pointer+=TIFFStripSize(tiff);
  }
  trim(untrimmed,tiff_width,untrimmed_height,bpp,&top,&left,&bottom,&right);

  image->xpos=left; image->ypos=top;
  image->width=right-left+1; image->height=bottom-top+1;
  image->bpp=bpp;

  extract(image,untrimmed,tiff_width,untrimmed_height);

	inpaint(image,(uint32*)untrimmed);

	_TIFFfree(untrimmed);

  image->ypos+=rowscut;

	image->xpos+=tiff_xoff; image->ypos+=tiff_yoff;

	g_workwidth=max(g_workwidth,(int)(tiff_xoff+tiff_width));
  g_workheight=max(g_workheight,(int)(tiff_yoff+tiff_height));

  TIFFClose(tiff);

  return true;
}

void tighten() {
  int i;
	int max_right=0,max_bottom=0;

  g_min_left=0x7fffffff;
  g_min_top=0x7fffffff;

  for (i=0; i<g_numimages; i++) {
    g_min_left=min(g_min_left,g_images[i].xpos);
    g_min_top=min(g_min_top,g_images[i].ypos);
  }

  for (i=0; i<g_numimages; i++) {
    g_images[i].xpos-=g_min_left;
    g_images[i].ypos-=g_min_top;
  }

	for (i=0; i<g_numimages; i++) {
		max_right=max(max_right,g_images[i].xpos+g_images[i].width);
		max_bottom=max(max_bottom,g_images[i].ypos+g_images[i].height);
	}

	g_workwidth=max_right;
	g_workheight=max_bottom;
}

void load_images(char** argv, int argc) {
  int i;
	int c8=0,c16=0;
  g_images=(struct_image*)malloc(argc*sizeof(struct_image));

  for (i=0; i<argc; i++) {
#ifdef WIN32
    strcpy_s(g_images[g_numimages].filename,256,argv[i]);
#else
    strncpy(g_images[g_numimages].filename,argv[i],256);
#endif

		if (load_image(&g_images[g_numimages])) {
			if (g_images[g_numimages].bpp==8) c8++; else c16++;
			g_numimages++;
		}
  }

	if (c8!=0 && c16!=0) die("8bpp and 16bpp images mixed - multiblend can't handle this!\n");

	if (g_numimages==0) die("no valid images loaded!");

	if (g_crop) tighten();

	if (g_workbpp_cmd!=0) {
		if (g_jpegquality!=-1) {
			output(0, "Warning: JPEG output; overriding to 8bpp\n");
			g_workbpp_cmd=8;
		}
		g_workbpp=g_workbpp_cmd;
	} else {
		if (c8==0) g_workbpp=16; else g_workbpp=8;
	}

	return;
}
