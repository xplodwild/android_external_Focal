void pseudowrap_split() {
	int c;
	int y;
  int split=g_workwidth>>1;
	int bpp=g_images[0].bpp>>3;

	g_images=(struct_image*)realloc(g_images,sizeof(struct_image)*2);

	g_images[1]=g_images[0];

	g_images[0].width=split;
	g_images[0].xpos=g_workwidth-split;
	g_images[1].xpos=0;
	g_images[1].width=g_workwidth-split;
	g_images[1].channels=new channel[3];

	for (c=0; c<g_numchannels; c++) g_images[1].channels[c].data=malloc(g_images[1].width*g_images[1].height*bpp);

  for (y=0; y<g_images[1].height; y++) {
		for (c=0; c<g_numchannels; c++) memcpy((uint8*)g_images[1].channels[c].data+(y*g_images[1].width)*bpp,(uint8*)g_images[0].channels[c].data+(y*g_workwidth+split)*bpp,g_images[1].width*bpp);

		if (y>0) for (c=0; c<g_numchannels; c++) memcpy((uint8*)g_images[0].channels[c].data+(y*g_images[0].width)*bpp,(uint8*)g_images[0].channels[c].data+(y*g_workwidth)*bpp,g_images[0].width*bpp);
	}
	
	for (c=0; c<g_numchannels; c++) g_images[0].channels[c].data=realloc(g_images[0].channels[c].data,g_images[0].width*g_images[0].height*bpp);
	g_numimages++;
	g_levels--;
}

void pseudowrap_seam() {
	int y;
	int p=0;
	int split=g_workwidth>>1;

	g_seams=(uint32*)malloc(g_workheight*2*sizeof(uint32));

  for (y=0; y<g_workheight; y++) {
		g_seams[p++]=(g_workwidth-split)<<8|1;
		g_seams[p++]=split<<8;
	}
}

void pseudowrap_unsplit() {
	int c,y;
	int split=g_workwidth>>1;
	int bpp=g_workbpp>>4;
	void* temp=malloc((g_workwidth-split)<<bpp); // larger end
	uint8* line;

	for (c=0; c<g_numchannels; c++) {
		for (y=0; y<g_workheight; y++) {
			line=(uint8*)(g_out_channels[c].data)+((g_workwidth*y)<<bpp);
			memcpy(temp,line+(split<<bpp),(g_workwidth-split)<<bpp);
      memcpy(line+((g_workwidth-split)<<bpp),line,split<<bpp);
			memcpy(line,temp,(g_workwidth-split)<<bpp);
		}
	}

	g_images[0].xpos=0;
	g_images[0].width=g_workwidth;

	g_numimages--;
}
