void go(char** argv, int input_args) {
	int blend_wh;
	int i;
  int pitch;
	my_timer timer;

  if (!g_nooutput) {
	  if (g_jpegquality!=-1) {
		  fopen_s(&g_jpeg,g_output_filename,"wb");
		  if (!g_jpeg) die("couldn't open output file");
	  } else {
		  if (!g_bigtiff) g_tiff=TIFFOpen(g_output_filename,"w"); else g_tiff=TIFFOpen(g_output_filename,"w8");
		  if (!g_tiff) die("couldn't open output file");
	  }
  }

	timer.set();
  load_images(argv,input_args);

	if (g_numimages==0) die("no valid input files");

	timer.report("load");
// calculate number of levels

	if (!g_wideblend) {
		blend_wh=0;
		for (i=0; i<g_numimages; i++) blend_wh+=g_images[i].width+g_images[i].height;
		blend_wh=(int)(blend_wh*(0.5/g_numimages));
	} else blend_wh=min(g_workwidth,g_workheight);

  g_levels=0;
  while (blend_wh>4) { // was >4 but that caused banding
    blend_wh=(blend_wh+1)>>1;
    g_levels++;
  }
	g_levels=min(g_max_levels,g_levels);
	g_levels=max(0,g_levels-g_sub_levels);

	output(1,"%dx%d, %dbpp, %d levels\n",g_workwidth,g_workheight,g_workbpp,g_levels);

  pitch=(g_workwidth+7)&(~7);

	g_line0=_aligned_malloc(pitch*sizeof(int),16);
	g_line1=_aligned_malloc(pitch*sizeof(int),16);
	g_line2=_aligned_malloc(pitch*sizeof(int),16);

	if (g_numimages==1 && g_images[0].xpos==0 && g_images[0].ypos==0) {
		output(1,"Only one image at 0,0; pseudo-wrapping mode assumed\n");
		g_pseudowrap=true;
		pseudowrap_split();
	}

// dimension mask structs for all images
	for (i=0; i<g_numimages; i++) {
		g_images[i].masks=(float**)malloc(g_levels*sizeof(float*));
	}

// calculate seams
	timer.set();
	if (g_pseudowrap) {
		pseudowrap_seam();
	} else {
  	seam();
	}
	timer.report("seaming");

  if (!g_nooutput) {

// calculate mask pyramids
  	mask_pyramids();

// blend
    timer.set();
	  blend();
    timer.report("blend");

	  if (g_pseudowrap) pseudowrap_unsplit();

	  output(1,"writing %s...\n",g_output_filename);
	  timer.set();

  	if (g_jpegquality!=-1) jpeg_out(); else tiff_out();
  	timer.report("write");
  }

	//	ppm_out(out_channels);
}
