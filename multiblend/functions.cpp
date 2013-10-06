#include <iostream>
#include <setjmp.h>

#ifndef MIN
//#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
//#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
//#define MINMAX(X,Y,Z) (MAX(X,MIN(Y,Z)))
#endif

jmp_buf jmpbuf;

void output(int level, const char* fmt, ...) {
	char buffer[1024];
	va_list args;

	if (level<=g_verbosity) {
		va_start(args,fmt);
		vsprintf(buffer, fmt,args);
		va_end(args);

		std:cout << buffer << std::endl;
	}
}

#ifdef WIN32
class my_timer {
public:
	void set() {
		QueryPerformanceCounter(&t1);
	}

	double read() {
		QueryPerformanceCounter(&t2);
		return (double)(t2.QuadPart-t1.QuadPart)/frequency.QuadPart;
	}

	my_timer() {
		QueryPerformanceFrequency(&frequency);
	}

	void report(const char* name) {
		if (g_timing) output(0,"%s: %.3fs\n",name,this->read());
	}
private:
	LARGE_INTEGER t1;
	LARGE_INTEGER t2;
	LARGE_INTEGER frequency;
};
#else
class my_timer {
public:
	void set() { }
	double read() { return 0; }
  void report(const char* name) { if (g_timing) output(0,"Timing not available\n"); }
};
#endif

void report_time(const char* name, double time) {
  if (g_timing) output(0,"%s: %.3fs\n",name,time);
}

#ifndef WIN32
#define SNPRINTF snprintf
int _stricmp(const char* a, const char* b) {
	return strcasecmp(a,b);
}

void* _aligned_malloc(size_t size, int boundary) {
	return memalign(boundary,size);
}

void _aligned_free(void* a) {
	free(a);
}

void fopen_s(FILE** f,const char* filename, const char* mode) {
  *f=fopen(filename,mode);
}
#else
#define SNPRINTF _snprintf_s
#endif

void* swap_aligned_malloc(size_t bytes, int alignment) {
	int i,c;
	void* tmp=NULL;

	tmp=_aligned_malloc(bytes,alignment);

	if (!tmp && g_swap) {
		for (i=g_numimages-1; i>0; i--) {
			for (c=g_numchannels-1; c>=0; c--) {
				if (!g_images[i].channels[c].swapped) {
					g_images[i].channels[c].swap();
			  	tmp=_aligned_malloc(bytes,alignment);
			  	if (tmp) return tmp;
				}
			}
		}
	}

	return tmp;
}

void die(const char* fmt, ...) {
	char buffer[1024];

	va_list args;

	va_start(args,fmt);
		vsprintf(buffer, fmt, args);
		va_end(args);
	std:cout << buffer << std::endl;

	if (g_debug) {
		output(0,"\nPress Enter to end\n");
		getchar();
	}

	longjmp(jmpbuf, 1);
}
