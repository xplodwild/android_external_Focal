#ifndef WIN32
void fopen_s(FILE** f,const char* filename, const char* mode);
#endif

void die(const char* error, ...);

void* g_line0;
void* g_line1;
void* g_line2;
void* g_linet;

void* g_temp;

int* g_dither;

struct struct_indexed {
  int size;
  uint32 p;
  uint32* pointer;
  uint32* data;
  uint32* rows;
  uint32* w;
  uint32* h;
};

struct struct_level {
  void* data;
	size_t offset;
  int x0,y0; // inclusive minimum coordinates within this extended level
  int x1,y1; // inclusive maximum coordinates within this extended level
  int w,h;
	int pitch;
};

struct GeoTIFFInfo {
  double XGeoRef, YGeoRef;
  double XCellRes, YCellRes;
  double projection[16];
  int    nodata;
};

class channel {
public:
	bool swapped;
	void* data;
	size_t bytes;
	char swapfile[FILENAME_MAX];

	channel() {
		data=NULL;
		swapped=false;
	}

	bool swap() {
		FILE* out;
		char tmp[FILENAME_MAX];

		if (swapped) return false;

#ifdef WIN32
		GetTempPath(MAX_PATH,tmp);
		GetTempFileName(tmp,"multiblend",0,swapfile);
#endif

		fopen_s(&out,swapfile,"wb");

		if (!out) die("Couldn't open swapfile for write");

		if (fwrite(data,bytes,1,out)!=bytes) {
			fclose(out);
			remove(swapfile);
			return false;
		}

		fclose(out);
		free(data);

		data=NULL;
		swapped=true;
		return true;
	}

	bool unswap() {
		FILE* in;
		
		if (!swapped) return false;

		fopen_s(&in,swapfile,"rb");
		if (!in) die("Couldn't open swapfile for read");

		if (!(data=malloc(bytes))) die("Couldn't re-allocate space for channel");

    fread(data,bytes,1,in);
		fclose(in);

		remove(swapfile);

		swapped=false;
		return true;
	}
};

struct struct_image {
  char filename[256];
  int bpp;
  int width,height;
  int xpos,ypos;
  channel* channels;
  struct_indexed binary_mask;
  struct_level* pyramid;
	float** masks;
	bool seampresent;
  GeoTIFFInfo geotiff;
};

int g_numthreads;

int g_numimages;
int g_workwidth=0;
int g_workheight=0;
int g_workbpp=0;
int g_workbpp_cmd=0;
int g_min_top=0;
int g_min_left=0;
double g_xres=-1;
double g_yres=-1;
int g_levels;
int g_max_levels=1000000;
int g_sub_levels=0;
int g_verbosity=1;
bool g_wideblend=false;
bool g_seamwarning=false;
bool g_reverse=false;
bool g_pseudowrap=false;
bool g_swap=false;
char* g_output_filename;
char* g_seamload_filename;
png_color* g_palette;
char* g_seamsave_filename;
char* g_xor_filename;
int g_numchannels=3;
channel* g_out_channels;
TIFF* g_tiff;
FILE* g_jpeg;
int g_compression=-1;
int g_jpegquality=-1;
uint32* g_seams;
bool g_timing=false;
bool g_savemasks=false;
bool g_nooutput=false;

struct_level* g_output_pyramid;

bool g_crop=true;
bool g_debug=false;
bool g_nomask=false;
bool g_bigtiff=false;
bool g_bgr=false;

uint32* g_edt;

struct_image* g_images;
