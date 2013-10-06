#define PY(i,l) g_images[i].pyramid[l]
#define ACCURACY 5 // don't change this

void hshrink(struct_level* upper, struct_level* lower) {
	int x,y;
	int tmp1,tmp2;
	size_t up=0;
	int* tmp=(int*)g_temp;
  int x_extra0=(upper->x0>>1)-lower->x0;
  int xlim=(upper->x1>>1)-lower->x0; // xpos on lower when we need to wrap last pixel
	int ushift=upper->pitch-upper->w;

	for (y=0; y<upper->h; y++) {
    x=0;
		if (g_workbpp==8) {
      tmp1=((short*)upper->data)[up++];
      tmp2=((short*)upper->data)[up++];
		} else {
      tmp1=((int*)upper->data)[up++];
      tmp2=((int*)upper->data)[up++];
		}
		tmp1+=tmp1<<1;
		tmp1=tmp1+tmp2;
    while (x<=x_extra0) {
      tmp[x++]=tmp1; // was +tmp2
    }

		if (g_workbpp==8) {
			while (x<xlim) {
				tmp1=tmp2+(((short*)upper->data)[up++]<<1);
				tmp2=((short*)upper->data)[up++];
				tmp[x++]=(tmp1+tmp2);
			}
      tmp1=((short*)upper->data)[up++];
		} else {
			while (x<xlim) {
				tmp1=tmp2+(((int*)upper->data)[up++]<<1);
				tmp2=((int*)upper->data)[up++];
				tmp[x++]=(tmp1+tmp2);
			}
      tmp1=((int*)upper->data)[up++];
		}

    tmp1+=tmp1<<1;
		tmp1+=tmp2;

    while (x<lower->pitch) {
      tmp[x++]=tmp1;
    }

		up+=ushift;
    tmp+=lower->pitch;
  }

//	if (g_workbpp==8) {
//		for (x=0; x<lower->pitch; x++) {
//	}
}

void vshrink(struct_level* upper, struct_level* lower) {
	int i;
	int x,y;
	size_t lp=0;
	int* tmp=(int*)g_temp;
  int y_extra0=(upper->y0>>1)-lower->y0;
  int ylim=(upper->y1>>1)-lower->y0; // ypos on lower when we need to duplicate last rows
//	int lshift=lower->pitch-lower->w;
	int lps;
	__m128i eight;
	__m128i sse_tmp1;
	__m128i sse_tmp2;
	__m128i* a;
	__m128i* b;
	__m128i* c;

	for (i=0; i<4; i++) ((int*)&eight)[i]=8;

// setup/top line/copies of top line
	if (g_workbpp==8) {
		for (x=0; x<lower->pitch; x++) ((short*)lower->data)[lp++]=(tmp[x]+(tmp[x]<<1)+tmp[x+lower->pitch]+8)>>4;
		for (y=1; y<=y_extra0; y++) { memcpy(&((short*)lower->data)[lp],lower->data,lower->pitch<<1); lp+=lower->pitch; }
  } else {
    for (x=0; x<lower->pitch; x++) ((int*)lower->data)[lp++]=(tmp[x]+(tmp[x]<<1)+tmp[x+lower->pitch]+8)>>4;
		for (y=1; y<=y_extra0; y++) { memcpy(&((int*)lower->data)[lp],lower->data,lower->pitch<<2); lp+=lower->pitch; }
	}

// middle lines

	/*
		if (g_workbpp==8) {
			for (x=0; x<lps; x++) {
				sse_tmp1=_mm_slli_epi16(*b++,1); // b*2
				sse_tmp2=_mm_add_epi16(*a++,*c++); // a+c
				sse_tmp1=_mm_add_epi16(sse_tmp1,sse_tmp2); // a+b*2+c
				sse_tmp1=_mm_add_epi16(sse_tmp1,eight);
				((__m128i*)lower->data)[lp++]=_mm_srai_epi16(sse_tmp1,4);
			}
			*/

	lps=lower->pitch>>2;

	a=(__m128i*)(tmp+lower->pitch);
	b=a+lps; // was (lower->pitch>>2);
	c=b+lps;

// move to __m128i offsets
	if (g_workbpp==8) {
//		lp=lp>>3; // 8 shorts per __m128i

		for (; y<ylim; y++) {
//			tmp+=lower->pitch<<1; // point to middle of 3, keep it moving so it's at the right place later
			for (x=0; x<lps; x++) {
				sse_tmp1=_mm_slli_epi32(*b++,1); // b*2
				sse_tmp2=_mm_add_epi32(*a++,*c++); // a+c
				sse_tmp1=_mm_add_epi32(sse_tmp1,sse_tmp2); // a+b*2+c
				sse_tmp1=_mm_add_epi32(sse_tmp1,eight);
				((__m128i*)g_line0)[x]=_mm_srai_epi32(sse_tmp1,4);
			}
			for (x=0; x<lower->pitch; x++) ((short*)lower->data)[lp++]=((int*)g_line0)[x];
			a=b;
			b=c;
			c+=lps;
		}
		tmp=(int*)(a-lps);
//		lp=lp>>3;
	} else {
		lp=lp>>2;

		for (; y<ylim; y++) {
			tmp+=lower->pitch<<1; // point to middle line of 3
			for (x=0; x<lps; x++) {
				sse_tmp1=_mm_slli_epi32(*b++,1); // b*2
				sse_tmp2=_mm_add_epi32(*a++,*c++); // a+c
				sse_tmp1=_mm_add_epi32(sse_tmp1,sse_tmp2); // a+b*2+c
				sse_tmp1=_mm_add_epi32(sse_tmp1,eight);
				((__m128i*)lower->data)[lp++]=_mm_srai_epi32(sse_tmp1,4);
			}
	// b now points to where c started, c points to the line beyond c
			a=b;
			b=c;
			c+=lps;
		}
		tmp=(int*)(a-lps);
// move back to short/int offsets
		lp=lp<<2;
	}

// bottom line
	tmp+=lower->pitch<<1;

	if (g_workbpp==8) for (x=0; x<lower->pitch; x++) ((short*)lower->data)[lp++]=(tmp[x-lower->pitch]+tmp[x]+(tmp[x]<<1)+8)>>4;
	else            	for (x=0; x<lower->pitch; x++) ((int*)lower->data)[lp++]=(tmp[x-lower->pitch]+tmp[x]+(tmp[x]<<1)+8)>>4;
	y++;


// copies of bottom line
	if (g_workbpp==8) {
//  	tmp=&((short*)lower->data)[lp-lower->pitch];
		for (; y<lower->h; y++) { memcpy(&((short*)lower->data)[lp],&((short*)lower->data)[lp-lower->pitch],lower->pitch<<1); lp+=lower->pitch; }
	} else {
//  	tmp=&((int*)lower->data)[lp-lower->pitch];
		for (; y<lower->h; y++) { memcpy(&((int*)lower->data)[lp],&((int*)lower->data)[lp-lower->pitch],lower->pitch<<2); lp+=lower->pitch; }
	}
}

__inline void inflate_line_short(short *input, short *output, int w) {
  int x=0,ix=0,p,n;

  p=input[ix++];
  output[x++]=p;

  while (x<w-1) {
    n=input[ix++];
    output[x++]=(p+n+1)>>1;
    output[x++]=n;
    p=n;
  }

  while (x<w) {
    output[x]=output[x-1];
		x++;
  }
}

__inline void inflate_line_int(int *input, int *output, int w) {
  int x=0,ix=0,p,n;

  p=input[ix++];
  output[x++]=p;

  while (x<w-1) {
    n=input[ix++];
    output[x++]=(p+n+1)>>1;
    output[x++]=n;
    p=n;
  }

  while (x<w) {
    output[x]=output[x-1];
		x++;
  }
}

void hps(struct_level* upper, struct_level *lower) {
	int i;
  int x,y;
  int x_extra0=(upper->x0>>1)-lower->x0;
  int y_extra0=(upper->y0>>1)-lower->y0;
  int ylim=(upper->h+1)>>1;
	int sse_pitch;
	int lp;
	__m128i sse_mix;
	__m128i one;
  void* swap;

	if (g_workbpp==8) {
		sse_pitch=upper->pitch>>3;
		for (i=0; i<8; i++) ((short*)&one)[i]=1;
	} else {
		sse_pitch=upper->pitch>>2;
		for (i=0; i<4; i++) ((int*)&one)[i]=1;
	}

	lp=y_extra0*lower->pitch+x_extra0;
  __m128i* upper_p=((__m128i*)upper->data);

	if (g_workbpp==8) inflate_line_short(&((short*)lower->data)[lp],(short*)g_line0,upper->pitch);
  else	            inflate_line_int(&((int*)lower->data)[lp],(int*)g_line0,upper->pitch);
  lp+=lower->pitch;

	if (g_workbpp==8) for (x=0; x<sse_pitch; x++) upper_p[x]=_mm_sub_epi16(upper_p[x],((__m128i*)g_line0)[x]);
	else            	for (x=0; x<sse_pitch; x++) upper_p[x]=_mm_sub_epi32(upper_p[x],((__m128i*)g_line0)[x]);
	upper_p+=sse_pitch;

  for (y=1; y<ylim; y++) {
		if (g_workbpp==8) {
			inflate_line_short(&((short*)lower->data)[lp],(short*)g_line1,upper->pitch);
			for (x=0; x<sse_pitch; x++) {
				sse_mix=_mm_add_epi16(((__m128i*)g_line0)[x],((__m128i*)g_line1)[x]);
				sse_mix=_mm_add_epi16(sse_mix,one);
				sse_mix=_mm_srai_epi16(sse_mix,1);
				upper_p[x]=_mm_sub_epi16(upper_p[x],sse_mix);
			}
			lp+=lower->pitch;
		} else {
			inflate_line_int(&((int*)lower->data)[lp],(int*)g_line1,upper->pitch);
			for (x=0; x<sse_pitch; x++) {
				sse_mix=_mm_add_epi32(((__m128i*)g_line0)[x],((__m128i*)g_line1)[x]);
				sse_mix=_mm_add_epi32(sse_mix,one);
				sse_mix=_mm_srai_epi32(sse_mix,1);
				upper_p[x]=_mm_sub_epi32(upper_p[x],sse_mix);
			}
			lp+=lower->pitch;
		}
		upper_p+=sse_pitch;

		if (g_workbpp==8) for (x=0; x<sse_pitch; x++) upper_p[x]=_mm_sub_epi16(upper_p[x],((__m128i*)g_line1)[x]);
		else          		for (x=0; x<sse_pitch; x++) upper_p[x]=_mm_sub_epi32(upper_p[x],((__m128i*)g_line1)[x]);
  	upper_p+=sse_pitch;

		swap=g_line0;
    g_line0=g_line1;
    g_line1=swap;
  }
}

void shrink_hps(struct_level* upper, struct_level* lower) {
  hshrink(upper,lower);
	vshrink(upper,lower);
	hps(upper,lower);
}

void copy_channel(int i, int c) {
	int x,y;
	struct_level* top=&PY(i,0);
	void* pixels;
	int a=0;
	int ip=0;
	int op=0;
	int x_extra0=g_images[i].xpos-top->x0;
	int y_extra0=g_images[i].ypos-top->y0;
//	int x_extra1=top->x1-(g_images[i].xpos+g_images[i].width-1);
	int y_extra1=top->y1-(g_images[i].ypos+g_images[i].height-1);
	int xlim=g_images[i].width+x_extra0;
//	int bpp=g_images[i].bpp;
	int ipt;
	int mode;

	if (g_bgr) c=2-c;

	mode=(g_workbpp==16)<<1|(g_images[i].bpp==16);

//	if (rand()&1) {
//		output(0, "swaptest %d %d\n",i,c);
//		g_images[i].channels[c].swap();
//	}

	if (g_images[i].channels[c].swapped) g_images[i].channels[c].unswap();

	pixels=g_images[i].channels[c].data;

	for (y=0; y<top->h-y_extra1; y++) {
		switch(mode) {
  		case 0:
				a=((uint8*)pixels)[ip++]<<ACCURACY;
				for (x=0; x<=x_extra0; x++) ((short*)top->data)[op++]=a;
  			ipt=ip+xlim-x;
				while (ip<ipt) {
					a=((uint8*)pixels)[ip++]<<ACCURACY;
					((short*)top->data)[op++]=a;
				}
				for (x=xlim; x<top->pitch; x++) ((short*)top->data)[op++]=a;
				break;
			case 1:
				a=((uint16*)pixels)[ip++]>>(8-ACCURACY);
				for (x=0; x<=x_extra0; x++) ((short*)top->data)[op++]=a;
  			ipt=ip+xlim-x;
				while (ip<ipt) {
					a=((uint16*)pixels)[ip++]>>(8-ACCURACY);
					((short*)top->data)[op++]=a;
				}
				for (x=xlim; x<top->pitch; x++) ((short*)top->data)[op++]=a;
				break;
			case 2:
				a=((uint8*)pixels)[ip++];
				a=a<<16|a<<8;
				for (x=0; x<=x_extra0; x++) ((int*)top->data)[op++]=a;
  			ipt=ip+xlim-x;
				while (ip<ipt) {
					a=((uint8*)pixels)[ip++];
					a=a<<16|a<<8;
					((int*)top->data)[op++]=a;
				}
				for (x=xlim; x<top->pitch; x++) ((int*)top->data)[op++]=a;
				break;
			case 3:
				a=((uint16*)pixels)[ip++]<<8;
				for (x=0; x<=x_extra0; x++) ((int*)top->data)[op++]=a;
  			ipt=ip+xlim-x;
				while (ip<ipt) {
					a=((uint16*)pixels)[ip++]<<8;
					((int*)top->data)[op++]=a;
				}
				for (x=xlim; x<top->pitch; x++) ((int*)top->data)[op++]=a;
				break;
		}
//continue;
/*		if (bpp==8) {
			ipt=ip+xlim-x;
//			for (; x<xlim; x++) {
			while (ip<ipt) {
				a=((uint8*)pixels)[ip++]<<16;
				((int*)top->data)[op++]=a;
			}
			x=xlim;
		} else {
			ipt=ip+xlim-x;
//			for (; x<xlim; x++) {
			while (ip<ipt) {
				a=((uint16*)pixels)[ip++]<<8;
				((int*)top->data)[op++]=a;
			}
		} */

//		for (; x<top->pitch; x++) ((int*)top->data)[op++]=a;

		if (y==0) {
			switch(mode&2) {
  			case 0:
					for (; y<y_extra0; y++) {
						memcpy(&((short*)top->data)[op],&((short*)top->data)[op-top->pitch],top->pitch<<1);
						op+=top->pitch;
					}
					break;
				case 2:
					for (; y<y_extra0; y++) {
						memcpy(&((int*)top->data)[op],&((int*)top->data)[op-top->pitch],top->pitch<<2);
						op+=top->pitch;
					}
					break;
			}
		}
	}

	switch(mode&2) {
  	case 0:
			for (; y<top->h; y++) {
				memcpy(&((short*)top->data)[op],&((short*)top->data)[op-top->pitch],top->pitch<<1);
				op+=top->pitch;
			}
			break;
  	case 2:
			for (; y<top->h; y++) {
				memcpy(&((int*)top->data)[op],&((int*)top->data)[op-top->pitch],top->pitch<<2);
				op+=top->pitch;
			}
			break;
	}

	free(pixels);
	g_images[i].channels[c].swapped=true;
}

#define NEXT_MASK { \
	pixel.f=*mask++; \
 	if (pixel.i<0) { \
  	count=-pixel.i; \
  	pixel.f=*mask++; \
	} else count=1; \
}

void mask_into_output(struct_level* input, float* mask, struct_level* output, bool first) {
	int x,y;
	void* input_line;
	int count;
	int limcount;
  int x_extra,y_extra;
	void* out_p=((void*)output->data);
	int xlim=min(output->w,input->x0+input->w);
	int ylim=min(output->h,input->y0+input->h);
	int bpp_shift=g_workbpp>>3;
  intfloat pixel;

	if (first) memset(output->data,0,(output->pitch*output->h)<<bpp_shift);

	input_line=input->data;

	x=0;
	x_extra=input->x0;
	if (x_extra<0) {
		x_extra=0;
		x-=input->x0;
	}
//	input_line+=x-x_extra;
	input_line=(void*)&((char*)input_line)[(x-x_extra)<<bpp_shift];

	y_extra=input->y0;
	if (y_extra<0) {
		input_line=(void*)&((char*)input_line)[(-y_extra*input->pitch)<<bpp_shift];
		y_extra=0;
	}

// advance mask pointer to first active line
  x=output->w*y_extra;
	while (x>0) {
		NEXT_MASK;
		x-=count;
	}

// advance output pointer to first active line
//	out_p+=output->pitch*y_extra;
	out_p=(void*)&((char*)out_p)[(output->pitch*y_extra)<<bpp_shift];

	for (y=y_extra; y<ylim; y++) {
// advance mask pointer to correct x position
		x=0;
		while (x<x_extra) {
			NEXT_MASK;
			x+=count;
		}

		count=x-x_extra;
		x-=count;

// mask in active pixels
		while (x<xlim) {
			if (count==0) NEXT_MASK;
			if (pixel.f==0) {
				x+=count;
				count=0;
			} else
			if (pixel.f==1) {
				limcount=xlim-x;
				if (limcount>count) limcount=count;
				if (g_workbpp==8)
  				memcpy(&((short*)out_p)[x],&((short*)input_line)[x],limcount<<bpp_shift);
				else
  				memcpy(&((int*)out_p)[x],&((int*)input_line)[x],limcount<<bpp_shift);
//				} else {
//					memcpy(&out_p[x],&input_line[x],count<<bpp_shift);
//				}
				x+=count;
				count=0;
			} else {
				if (g_workbpp==8)
  				((short*)out_p)[x]+=(int)(((short*)input_line)[x++]*pixel.f+0.5);
				else
  				((int*)out_p)[x]+=(int)(((int*)input_line)[x++]*pixel.f+0.5);
				count--;
			}
		}

// advance mask pointer to next line
		x+=count;
		while (x<output->w) {
			NEXT_MASK;
      x+=count;
		}

		out_p=(void*)&((char*)out_p)[output->pitch<<bpp_shift];
		input_line=(void*)&((char*)input_line)[input->pitch<<bpp_shift];
	}
}

void collapse(struct_level* lower, struct_level* upper) {
	int i;
  int x,y;
//  int x_extra0=(upper->x0>>1)-lower->x0;
//  int y_extra0=(upper->y0>>1)-lower->y0;
//  int ylim=(upper->h+1)>>1;
	int sse_pitch;
	int lp;
	__m128i sse_mix;
	__m128i one;
  void* swap;
	int y_extra=lower->h*2-upper->w-1;

	if (g_workbpp==8) {
		sse_pitch=upper->pitch>>3;
		for (i=0; i<8; i++) ((short*)&one)[i]=1;
	} else {
		sse_pitch=upper->pitch>>2;
		for (i=0; i<4; i++) ((int*)&one)[i]=1;
	}

	lp=0; //y_extra0*lower->pitch+x_extra0;
  __m128i* upper_p=((__m128i*)upper->data);

	if (g_workbpp==8)	inflate_line_short(&((short*)lower->data)[lp],(short*)g_line0,upper->pitch);
	else            	inflate_line_int(&((int*)lower->data)[lp],(int*)g_line0,upper->pitch);
  lp+=lower->pitch;

	if (g_workbpp==8)	for (x=0; x<sse_pitch; x++) upper_p[x]=_mm_add_epi16(upper_p[x],((__m128i*)g_line0)[x]);
	else            	for (x=0; x<sse_pitch; x++) upper_p[x]=_mm_add_epi32(upper_p[x],((__m128i*)g_line0)[x]);
	upper_p+=sse_pitch;

  for (y=1; y<lower->h; y++) {
		if (g_workbpp==8) {
			inflate_line_short(&((short*)lower->data)[lp],(short*)g_line1,upper->pitch);
			lp+=lower->pitch;
			for (x=0; x<sse_pitch; x++) {
				sse_mix=_mm_add_epi16(((__m128i*)g_line0)[x],((__m128i*)g_line1)[x]);
				sse_mix=_mm_add_epi16(sse_mix,one);
				sse_mix=_mm_srai_epi16(sse_mix,1);
				upper_p[x]=_mm_add_epi16(upper_p[x],sse_mix);
			}
		} else {
			inflate_line_int(&((int*)lower->data)[lp],(int*)g_line1,upper->pitch);
			lp+=lower->pitch;
			for (x=0; x<sse_pitch; x++) {
				sse_mix=_mm_add_epi32(((__m128i*)g_line0)[x],((__m128i*)g_line1)[x]);
				sse_mix=_mm_add_epi32(sse_mix,one);
				sse_mix=_mm_srai_epi32(sse_mix,1);
				upper_p[x]=_mm_add_epi32(upper_p[x],sse_mix);
			}
		}
		upper_p+=sse_pitch;

		if (y==lower->h && y_extra) break;

		if (g_workbpp==8) for (x=0; x<sse_pitch; x++) upper_p[x]=_mm_add_epi16(upper_p[x],((__m128i*)g_line1)[x]);
		else              for (x=0; x<sse_pitch; x++) upper_p[x]=_mm_add_epi32(upper_p[x],((__m128i*)g_line1)[x]);
  	upper_p+=sse_pitch;

		swap=g_line0;
    g_line0=g_line1;
    g_line1=swap;
  }
}

void dither(struct_level* top, void* channel) {
	int i;
	int x,y;
	int p=0;
	int q;
	int dith_off=0;
//	int dith_x=0;
	int dp=0;

	if (g_workbpp==8) {
		if (RAND_MAX==32767) for (i=0; i<1024; i++) g_dither[i]=rand()>>(15-ACCURACY);
    else for (i=0; i<1024; i++) g_dither[i]=rand()>>(31-ACCURACY);

		for (y=0; y<g_workheight; y++) {
			dith_off-=32;
			if (dith_off<0) dith_off=992;
			for (x=0; x<g_workwidth; x++) {
				q=(((short*)top->data)[dp+x]+g_dither[dith_off+(x&31)])>>ACCURACY;
				if (q<0) q=0; else if (q>255) q=0xff;
        ((uint8*)channel)[p++]=q;
			}

			dp+=top->pitch;
		}
	} else {
    if (RAND_MAX==32767) for (i=0; i<1024; i++) g_dither[i]=rand()>>7;
    else for (i=0; i<1024; i++) g_dither[i]=rand()>>23;

		for (y=0; y<g_workheight; y++) {
			dith_off-=32;
			if (dith_off<0) dith_off=992;
			for (x=0; x<g_workwidth; x++) {
				q=(((int*)top->data)[dp+x]+g_dither[dith_off+(x&31)])>>8;
				if (q<0) q=0; else if (q>0xffff) q=0xffff;
        ((uint16*)channel)[p++]=q;
			}

			dp+=top->pitch;
		}
	}
}

void blend() {
	int i;
	int l;
	int c;
	int temp;
//	int x;
	size_t mem_out;
	size_t mem_image;
	size_t mem_image_max=0;
	size_t mem_temp=0;
	size_t mem_temp_max=0;
	size_t msize;
//	int rsize;
	void* out_pyramid;
	void* image_pyramid;
	my_timer timer;
	int pitch_plus;
	int size_of;

	output(1,"blending...\n");

  if (g_workbpp==8) pitch_plus=7; else pitch_plus=3;

// dimension pyramid structs
	msize=g_levels*sizeof(struct_level);
	g_output_pyramid=(struct_level*)malloc(msize);

	for (i=0; i<g_numimages; i++) {
		mem_image=0;
		g_images[i].pyramid=(struct_level*)malloc(msize);

//		PY(i,0).w=PY(i,0).x1+1-PY(i,0).x0;
//		PY(i,0).h=PY(i,0).y1+1-PY(i,0).y0;

		for (l=0; l<g_levels; l++) {
			PY(i,l).offset=mem_image;

			if (l==0) {
				PY(i,l).x0=(g_images[i].xpos-1)&~1;
				PY(i,l).y0=(g_images[i].ypos-1)&~1;
				PY(i,l).x1=(g_images[i].xpos+g_images[i].width+1)&~1;
				PY(i,l).y1=(g_images[i].ypos+g_images[i].height+1)&~1;
//				if (PY(i,l).x0<0) PY(i,l).x0=0;
//				if (PY(i,l).y0<0) PY(i,l).y0=0;
//				if (PY(i,l).x1>g_workwidth-1) PY(i,l).x1=g_workwidth-1;
//				if (PY(i,l).y1>g_workheight-1) PY(i,l).y1=g_workheight-1;
			} else {
				PY(i,l).x0=((PY(i,l-1).x0>>1)-1)&~1;
				PY(i,l).y0=((PY(i,l-1).y0>>1)-1)&~1;
				PY(i,l).x1=((PY(i,l-1).x1>>1)+2)&~1;
				PY(i,l).y1=((PY(i,l-1).y1>>1)+2)&~1;
			}

//			PY(i,l).x0=((PY(i,l).x0+0x10000000)&~3)-0x10000000;
//			PY(i,l).y0=((PY(i,l).y0+0x10000000)&~3)-0x10000000;

			PY(i,l).w=PY(i,l).x1+1-PY(i,l).x0;
			PY(i,l).h=PY(i,l).y1+1-PY(i,l).y0;
			PY(i,l).pitch=(PY(i,l).w+pitch_plus)&(~pitch_plus);

		  mem_image+=PY(i,l).pitch*PY(i,l).h;
		}

		if (g_levels>1)
      mem_temp=PY(i,0).h*PY(i,1).pitch;
		else
			mem_temp=0;
		if (mem_temp>mem_temp_max) mem_temp_max=mem_temp;

		if (mem_image>mem_image_max) mem_image_max=mem_image;
	}

	mem_out=0;
	for (l=0; l<g_levels; l++) {
		g_output_pyramid[l].offset=mem_out;

		if (l==0) {
			g_output_pyramid[l].x0=0;
			g_output_pyramid[l].y0=0;
			g_output_pyramid[l].w=g_workwidth;
			g_output_pyramid[l].h=g_workheight;
		} else {
			g_output_pyramid[l].x0=0;
			g_output_pyramid[l].y0=0;
			g_output_pyramid[l].w=(g_output_pyramid[l-1].w+2)>>1;
			g_output_pyramid[l].h=(g_output_pyramid[l-1].h+2)>>1;
		}

		g_output_pyramid[l].x1=g_output_pyramid[l].w-1;
		g_output_pyramid[l].y1=g_output_pyramid[l].h-1;
		g_output_pyramid[l].pitch=(g_output_pyramid[l].w+pitch_plus)&~pitch_plus;
		mem_out+=g_output_pyramid[l].pitch*g_output_pyramid[l].h;
	}

	if (g_workbpp==8) size_of=sizeof(short); else size_of=sizeof(int);

	image_pyramid=swap_aligned_malloc(mem_image_max*size_of,16);
	if (!image_pyramid) die("Couldn't allocate memory for image pyramid!");

	out_pyramid=swap_aligned_malloc(mem_out*size_of,16);
	if (!out_pyramid) die("Couldn't allocate memory for output pyramid!");

	if (mem_temp>0) {
  	g_temp=swap_aligned_malloc(mem_temp_max*sizeof(int),16);
	  if (!g_temp) die("Couldn't allocate enough temporary memory!");
	}

	for (i=0; i<g_numimages; i++) {
		for (l=0; l<g_levels; l++) {
			if (g_workbpp==8)
  			PY(i,l).data=&((short*)image_pyramid)[PY(i,l).offset];
			else
  			PY(i,l).data=&((int*)image_pyramid)[PY(i,l).offset];
		}
	}

	for (l=0; l<g_levels; l++) {
		if (g_workbpp==8)
  		g_output_pyramid[l].data=&((short*)out_pyramid)[g_output_pyramid[l].offset];
		else
  		g_output_pyramid[l].data=&((int*)out_pyramid)[g_output_pyramid[l].offset];
	}

	g_dither=(int*)_aligned_malloc(1024<<2,16);

// iterate over channels/images, create pyramids, copy/add to output
	double copy_time=0;
	double shrink_time=0;
	double collapse_time=0;
	double dither_time=0;
	double mio_time=0;

//  g_frugality=1;

	g_out_channels=new channel[g_numchannels];

	for (c=0; c<g_numchannels; c++) {
  	for (i=0; i<g_numimages; i++) {
			timer.set();
			copy_channel(i,c); // also frees channel when finished
			copy_time+=timer.read();

			timer.set();
			for (l=0; l<g_levels-1; l++) shrink_hps(&PY(i,l),&PY(i,l+1));
/*uLongf src_size=(int)PY(i,1).data-(int)PY(i,0).data;
uLongf dest_size=src_size;

int temp=compress2((uint8*)g_output_pyramid[0].data,&dest_size,(uint8*)PY(i,0).data,src_size,Z_BEST_COMPRESSION);
output(0, "%d,%d\n",temp,Z_OK);
output(0, "Compressed from %d to %d\n",src_size,dest_size);*/
      shrink_time+=timer.read();

			timer.set();
			for (l=0; l<g_levels; l++) mask_into_output(&PY(i,l),g_images[i].masks[l],&g_output_pyramid[l],i==0);
			mio_time+=timer.read();
		}

    timer.set();
		for (l=g_levels-1; l>0; l--) collapse(&g_output_pyramid[l],&g_output_pyramid[l-1]);
		collapse_time+=timer.read();

		if (i==g_numimages-1 && c==g_numchannels-1) {
			_aligned_free(image_pyramid);
			_aligned_free(g_temp);
		}
		
		temp=(g_workwidth*g_workheight)<<(g_workbpp>>4);
		g_out_channels[c].data=swap_aligned_malloc(temp,0);
		if (!g_out_channels[c].data) die("not enough memory for output channel!");

		timer.set();
		dither(&g_output_pyramid[0],g_out_channels[c].data);
		dither_time+=timer.read();
	}

  if (g_timing) {
		output(0, "\n");
		report_time("copy",copy_time);
		report_time("shrink",shrink_time);
		report_time("merge",mio_time);
		report_time("collapse",collapse_time);
		report_time("dither",dither_time);
		output(0,"\n");
	}

	_aligned_free(out_pyramid);
//	_aligned_free(image_pyramid);
//	_aligned_free(g_temp);

	free(g_output_pyramid);
	for (i=0; i<g_numimages; i++) free(g_images[i].pyramid);
}
