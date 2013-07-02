#define HAVE_MKSTEMP 1

#include <iostream>
#include "vigra/stdimage.hxx"
#include "vigra/stdcachedfileimage.hxx"
#include "vigra/imageinfo.hxx"
#include "vigra/impex.hxx"
#include "vigra/impexalpha.hxx"
#include "vigra/initimage.hxx"
#include "vigra/resizeimage.hxx"

using namespace std;
using namespace vigra;

bool GimpAssociatedAlphaHack = true;

int main(void) {
    CachedFileImageDirector::v().setAllocation(1500 << 20);
    CachedFileImageDirector::v().setBlockSize(2LL << 20);

    USRGBCFImage uscf(40000, 30000);
    //BRGBCFImage uscf(40000, 25000);
    CachedFileImageDirector::v().printStats("uscf", 0, &uscf);
    CachedFileImageDirector::v().printStats();
    CachedFileImageDirector::v().resetCacheMisses();

    typedef BCFImage::PixelType AlphaPixelType;
    BCFImage a(40000, 30000);
    //BCFImage a(40000, 25000);
    CachedFileImageDirector::v().printStats("uscf", 1, &uscf);
    CachedFileImageDirector::v().printStats("a", 1, &a);
    CachedFileImageDirector::v().printStats();
    CachedFileImageDirector::v().resetCacheMisses();

    ImageImportInfo inputImage("gigapixel_out.tif");
    importImageAlpha(inputImage, destImage(uscf), destImage(a));
    CachedFileImageDirector::v().printStats("uscf", 2, &uscf);
    CachedFileImageDirector::v().printStats("a", 2, &a);
    CachedFileImageDirector::v().printStats();
    CachedFileImageDirector::v().resetCacheMisses();

    {
    FindBoundingRectangle unionRect;
    inspectImageIf(srcIterRange(Diff2D(), Diff2D() + a.size()),
            srcImage(a), unionRect);
        cout << "readback bounding box: valid="
	     << unionRect.valid
	     << " ("
             << unionRect.upperLeft.x
             << ", "
             << unionRect.upperLeft.y
             << ") -> ("
             << unionRect.lowerRight.x
             << ", "
             << unionRect.lowerRight.y
             << ")" << endl;
    }
    CachedFileImageDirector::v().printStats("uscf", 3, &uscf);
    CachedFileImageDirector::v().printStats("a", 3, &a);
    CachedFileImageDirector::v().printStats();
    CachedFileImageDirector::v().resetCacheMisses();

    int stride = 40;
    USRGBCFImage small(uscf.width() / stride, uscf.height() / stride);
    //BRGBCFImage small(uscf.width() / stride, uscf.height() / stride);
    CachedFileImageDirector::v().printStats("uscf", 5, &uscf);
    CachedFileImageDirector::v().printStats("a", 5, &a);
    CachedFileImageDirector::v().printStats("small", 5, &small);
    CachedFileImageDirector::v().printStats();
    CachedFileImageDirector::v().resetCacheMisses();

    {
	    typedef USRGBCFImage::Accessor USAccessor;
	    typedef USRGBCFImage::traverser USTraverser;
	    //typedef BRGBCFImage::Accessor USAccessor;
	    //typedef BRGBCFImage::traverser USTraverser;
	    USAccessor sa = uscf.accessor();
	    USAccessor da = small.accessor();
	    USTraverser sy = uscf.upperLeft();
	    USTraverser send = uscf.lowerRight();
	    USTraverser dy = small.upperLeft();
	    for (; sy.y != send.y; sy.y+=stride, ++dy.y) {
		USTraverser sx = sy;
		USTraverser dx = dy;
		for (; sx.x != send.x; sx.x+=stride, ++dx.x) {
		    da.set(sa(sx), dx);
		}
	    }
    }
    CachedFileImageDirector::v().printStats("uscf", 6, &uscf);
    CachedFileImageDirector::v().printStats("a", 6, &a);
    CachedFileImageDirector::v().printStats("small", 6, &small);
    CachedFileImageDirector::v().printStats();
    CachedFileImageDirector::v().resetCacheMisses();

    BCFImage smalla(a.width() / stride, a.height() / stride);
    CachedFileImageDirector::v().printStats("uscf", 7, &uscf);
    CachedFileImageDirector::v().printStats("a", 7, &a);
    CachedFileImageDirector::v().printStats("small", 7, &small);
    CachedFileImageDirector::v().printStats("smalla", 7, &smalla);
    CachedFileImageDirector::v().printStats();
    CachedFileImageDirector::v().resetCacheMisses();

    {
	    typedef BCFImage::traverser BTraverser;
	    typedef BCFImage::Accessor BAccessor;
	    BAccessor sa = a.accessor();
	    BAccessor da = smalla.accessor();
	    BTraverser sy = a.upperLeft();
	    BTraverser send = a.lowerRight();
	    BTraverser dy = smalla.upperLeft();
	    for (; sy.y != send.y; sy.y+=stride, ++dy.y) {
		BTraverser sx = sy;
		BTraverser dx = dy;
		for (; sx.x != send.x; sx.x+=stride, ++dx.x) {
		    da.set(sa(sx), dx);
		}
	    }
    }

    CachedFileImageDirector::v().printStats("uscf", 8, &uscf);
    CachedFileImageDirector::v().printStats("a", 8, &a);
    CachedFileImageDirector::v().printStats("small", 8, &small);
    CachedFileImageDirector::v().printStats("smalla", 8, &smalla);
    CachedFileImageDirector::v().printStats();
    CachedFileImageDirector::v().resetCacheMisses();

    //ImageExportInfo smallOutputInfo("readback_small.tif");
    ImageExportInfo smallOutputInfo("gigapixel_out_small.tif");
    exportImageAlpha(srcImageRange(small), srcImage(smalla), smallOutputInfo);
    CachedFileImageDirector::v().printStats("uscf", 9, &uscf);
    CachedFileImageDirector::v().printStats("a", 9, &a);
    CachedFileImageDirector::v().printStats("small", 9, &small);
    CachedFileImageDirector::v().printStats("smalla", 9, &smalla);
    CachedFileImageDirector::v().printStats();
    CachedFileImageDirector::v().resetCacheMisses();
return 0;
    //ImageExportInfo readbackAlpha("readback_smallalpha.tif");
    //exportImage(srcImageRange(smalla), readbackAlpha);

    {
    FindBoundingRectangle unionRect;
    inspectImageIf(srcIterRange(Diff2D(), Diff2D() + a.size()),
            srcImage(a), unionRect);
        cout << "assembled bounding box: valid=" << unionRect.valid << " ("
             << unionRect.upperLeft.x
             << ", "
             << unionRect.upperLeft.y
             << ") -> ("
             << unionRect.lowerRight.x
             << ", "
             << unionRect.lowerRight.y
             << ")" << endl;
    }

    CachedFileImageDirector::v().printStats("uscf", 10, &uscf);
    CachedFileImageDirector::v().printStats("a", 10, &a);
    CachedFileImageDirector::v().printStats("small", 10, &small);
    CachedFileImageDirector::v().printStats("smalla", 10, &smalla);
    CachedFileImageDirector::v().printStats();
    CachedFileImageDirector::v().resetCacheMisses();

    transformImage(srcImageRange(a), destImage(a),
            Threshold<AlphaPixelType, AlphaPixelType>(
		    NumericTraits<AlphaPixelType>::max() / 2,
		    NumericTraits<AlphaPixelType>::max(),
		    NumericTraits<AlphaPixelType>::zero(),
		    NumericTraits<AlphaPixelType>::max()
	    )
    );

    {
    FindBoundingRectangle unionRect;
    inspectImageIf(srcIterRange(Diff2D(), Diff2D() + a.size()),
            srcImage(a), unionRect);
        cout << "post-transform bounding box: valid=" << unionRect.valid << " ("
             << unionRect.upperLeft.x
             << ", "
             << unionRect.upperLeft.y
             << ") -> ("
             << unionRect.lowerRight.x
             << ", "
             << unionRect.lowerRight.y
             << ")" << endl;
    }

    CachedFileImageDirector::v().printStats("uscf", 11, &uscf);
    CachedFileImageDirector::v().printStats("a", 11, &a);
    CachedFileImageDirector::v().printStats("small", 11, &small);
    CachedFileImageDirector::v().printStats("smalla", 11, &smalla);
    CachedFileImageDirector::v().printStats();
    CachedFileImageDirector::v().resetCacheMisses();

    return 0;
}
