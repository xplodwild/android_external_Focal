# Makefile for libgsl
 
######################################################
###                   libgsl.a                      ##
######################################################

# Normally, libgsl is a bunch of separate libraries for
# each component. We simplify this for Nemesis and build
# what we need for enblend directly here in one lib.
 
LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

libgsl_la_SOURCES = \
	version.c \
	block/init.c \
	block/file.c \
	block/block.c \
	rng/borosh13.c \
	rng/cmrg.c \
	rng/coveyou.c \
	rng/default.c \
	rng/file.c \
	rng/fishman2x.c \
	rng/fishman18.c \
	rng/fishman20.c \
	rng/gfsr4.c \
	rng/inline.c \
	rng/knuthran.c \
	rng/knuthran2.c \
	rng/knuthran2002.c \
	rng/lecuyer21.c \
	rng/minstd.c \
	rng/mrg.c \
	rng/mt.c \
	rng/r250.c \
	rng/ran0.c \
	rng/ran1.c \
	rng/ran2.c \
	rng/ran3.c \
	rng/rand.c \
	rng/rand48.c \
	rng/random.c \
	rng/randu.c \
	rng/ranf.c \
	rng/ranlux.c \
	rng/ranlxd.c \
	rng/ranlxs.c \
	rng/ranmar.c \
	rng/rng.c \
	rng/schrage.c \
	rng/slatec.c \
	rng/taus.c \
	rng/taus113.c \
	rng/transputer.c \
	rng/tt.c \
	rng/types.c \
	rng/uni.c \
	rng/uni32.c \
	rng/vax.c \
	rng/waterman14.c \
	rng/zuf.c \
	multimin/conjugate_fr.c \
	multimin/conjugate_pr.c \
	multimin/convergence.c \
	multimin/diff.c \
	multimin/fdfminimizer.c \
	multimin/fminimizer.c \
	multimin/simplex.c \
	multimin/simplex2.c \
	multimin/steepest_descent.c \
	multimin/vector_bfgs.c \
	matrix/init.c \
	matrix/matrix.c \
	matrix/file.c \
	matrix/rowcol.c \
	matrix/swap.c \
	matrix/copy.c \
	matrix/minmax.c \
	matrix/prop.c \
	matrix/oper.c \
	matrix/getset.c \
	matrix/view.c \
	matrix/submatrix.c \
	err/error.c \
	err/stream.c \
	err/message.c \
	err/strerror.c \
	blas/blas.c \
	vector/init.c \
	vector/file.c \
	vector/vector.c \
	vector/copy.c \
	vector/swap.c \
	vector/prop.c \
	vector/minmax.c \
	vector/oper.c \
	vector/reim.c \
	vector/subvector.c \
	vector/view.c \
	cblas/sasum.c \
	cblas/saxpy.c \
	cblas/scasum.c \
	cblas/scnrm2.c \
	cblas/scopy.c \
	cblas/sdot.c \
	cblas/sdsdot.c \
	cblas/sgbmv.c \
	cblas/sgemm.c \
	cblas/sgemv.c \
	cblas/sger.c \
	cblas/snrm2.c \
	cblas/srot.c \
	cblas/srotg.c \
	cblas/srotm.c \
	cblas/srotmg.c \
	cblas/ssbmv.c \
	cblas/sscal.c \
	cblas/sspmv.c \
	cblas/sspr.c \
	cblas/sspr2.c \
	cblas/sswap.c \
	cblas/ssymm.c \
	cblas/ssymv.c \
	cblas/ssyr.c \
	cblas/ssyr2.c \
	cblas/ssyr2k.c \
	cblas/ssyrk.c \
	cblas/stbmv.c \
	cblas/stbsv.c \
	cblas/stpmv.c \
	cblas/stpsv.c \
	cblas/strmm.c \
	cblas/strmv.c \
	cblas/strsm.c \
	cblas/strsv.c \
	cblas/dasum.c \
	cblas/daxpy.c \
	cblas/dcopy.c \
	cblas/ddot.c \
	cblas/dgbmv.c \
	cblas/dgemm.c \
	cblas/dgemv.c \
	cblas/dger.c \
	cblas/dnrm2.c \
	cblas/drot.c \
	cblas/drotg.c \
	cblas/drotm.c \
	cblas/drotmg.c \
	cblas/dsbmv.c \
	cblas/dscal.c \
	cblas/dsdot.c \
	cblas/dspmv.c \
	cblas/dspr.c \
	cblas/dspr2.c \
	cblas/dswap.c \
	cblas/dsymm.c \
	cblas/dsymv.c \
	cblas/dsyr.c \
	cblas/dsyr2.c \
	cblas/dsyr2k.c \
	cblas/dsyrk.c \
	cblas/dtbmv.c \
	cblas/dtbsv.c \
	cblas/dtpmv.c \
	cblas/dtpsv.c \
	cblas/dtrmm.c \
	cblas/dtrmv.c \
	cblas/dtrsm.c \
	cblas/dtrsv.c \
	cblas/dzasum.c \
	cblas/dznrm2.c \
	cblas/caxpy.c \
	cblas/ccopy.c \
	cblas/cdotc_sub.c \
	cblas/cdotu_sub.c \
	cblas/cgbmv.c \
	cblas/cgemm.c \
	cblas/cgemv.c \
	cblas/cgerc.c \
	cblas/cgeru.c \
	cblas/chbmv.c \
	cblas/chemm.c \
	cblas/chemv.c \
	cblas/cher.c \
	cblas/cher2.c \
	cblas/cher2k.c \
	cblas/cherk.c \
	cblas/chpmv.c \
	cblas/chpr.c \
	cblas/chpr2.c \
	cblas/cscal.c \
	cblas/csscal.c \
	cblas/cswap.c \
	cblas/csymm.c \
	cblas/csyr2k.c \
	cblas/csyrk.c \
	cblas/ctbmv.c \
	cblas/ctbsv.c \
	cblas/ctpmv.c \
	cblas/ctpsv.c \
	cblas/ctrmm.c \
	cblas/ctrmv.c \
	cblas/ctrsm.c \
	cblas/ctrsv.c \
	cblas/zaxpy.c \
	cblas/zcopy.c \
	cblas/zdotc_sub.c \
	cblas/zdotu_sub.c \
	cblas/zdscal.c \
	cblas/zgbmv.c \
	cblas/zgemm.c \
	cblas/zgemv.c \
	cblas/zgerc.c \
	cblas/zgeru.c \
	cblas/zhbmv.c \
	cblas/zhemm.c \
	cblas/zhemv.c \
	cblas/zher.c \
	cblas/zher2.c \
	cblas/zher2k.c \
	cblas/zherk.c \
	cblas/zhpmv.c \
	cblas/zhpr.c \
	cblas/zhpr2.c \
	cblas/zscal.c \
	cblas/zswap.c \
	cblas/zsymm.c \
	cblas/zsyr2k.c \
	cblas/zsyrk.c \
	cblas/ztbmv.c \
	cblas/ztbsv.c \
	cblas/ztpmv.c \
	cblas/ztpsv.c \
	cblas/ztrmm.c \
	cblas/ztrmv.c \
	cblas/ztrsm.c \
	cblas/ztrsv.c \
	cblas/icamax.c \
	cblas/idamax.c \
	cblas/isamax.c \
	cblas/izamax.c \
	cblas/xerbla.c \
	min/fsolver.c \
	min/golden.c \
	min/brent.c \
	min/convergence.c \
	min/bracketing.c \
	sys/minmax.c \
	sys/prec.c \
	sys/hypot.c \
	sys/log1p.c \
	sys/expm1.c \
	sys/coerce.c \
	sys/invhyp.c \
	sys/pow_int.c \
	sys/infnan.c \
	sys/fdiv.c \
	sys/fcmp.c \
	sys/ldfrexp.c

LOCAL_SRC_FILES:= $(libgsl_la_SOURCES)

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/gsl

LOCAL_LDLIBS := -lz
LOCAL_CFLAGS := -DAVOID_TABLES -O3 -fstrict-aliasing -fprefetch-loop-arrays \
	-DANDROID_TILE_BASED_DECODE -DENABLE_ANDROID_NULL_CONVERT -D__Ansi__

LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := libgsl

include $(BUILD_STATIC_LIBRARY)
