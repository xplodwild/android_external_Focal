#ifndef STRIDE_HXX
#define STRIDE_HXX


#include <vigra/imageiterator.hxx>


namespace vigra_ext {

vigra::Diff2D
stridedSize(int xstride, int ystride, vigra::Diff2D size)
{
    if (size.x % xstride != 0) {
        size.x += xstride - size.x % xstride;
    }
    if (size.y % ystride != 0) {
        size.y += ystride - size.y % ystride;
    }

    size.x /= xstride;
    size.y /= ystride;

    return size;
}


template <class Iterator>
int
iteratorWidth(Iterator imageIterator)
{
    Iterator nextLine(imageIterator);
    nextLine.y += 1;
    return nextLine[0] - imageIterator[0];
}


template <typename PixelType, typename ImgAccessor, typename ImgIterator>
vigra::triple<vigra::StridedImageIterator<PixelType>,
              vigra::StridedImageIterator<PixelType>,
              ImgAccessor>
stride(int xstride, int ystride,
       vigra::triple<vigra::BasicImageIterator<PixelType, ImgIterator>,
                     vigra::BasicImageIterator<PixelType, ImgIterator>,
                     ImgAccessor> image)
{
    typedef vigra::StridedImageIterator<PixelType> SII;

    const vigra::Diff2D size = stridedSize(xstride, ystride,
                                    image.second - image.first);
    const SII base = SII(image.first[0],
                         iteratorWidth(image.first),
                         xstride, ystride);

    return vigra::make_triple(base, base + size, image.third);
}


template <typename PixelType, typename ImgAccessor, typename ImgIterator>
std::pair<vigra::StridedImageIterator<PixelType>, ImgAccessor>
stride(int xstride, int ystride,
       std::pair<vigra::BasicImageIterator<PixelType, ImgIterator>, ImgAccessor> image)
{
    typedef vigra::StridedImageIterator<PixelType> SII;

    return std::make_pair(SII(image.first[0],
                              iteratorWidth(image.first),
                              xstride, ystride),
                          image.second);
}


template <typename PixelType, typename ImgAccessor, typename ImgIterator>
vigra::triple<vigra::ConstStridedImageIterator<PixelType>,
              vigra::ConstStridedImageIterator<PixelType>,
              ImgAccessor>
stride(int xstride, int ystride,
       vigra::triple<vigra::ConstBasicImageIterator<PixelType, ImgIterator>,
                     vigra::ConstBasicImageIterator<PixelType, ImgIterator>,
                     ImgAccessor> image)
{
    typedef vigra::ConstStridedImageIterator<PixelType> CSII;

    const vigra::Diff2D size = stridedSize(xstride, ystride,
                                    image.second - image.first);
    const CSII base = CSII(image.first[0],
                           iteratorWidth(image.first),
                           xstride, ystride);

    return vigra::make_triple(base, base + size, image.third);
}


template <typename PixelType, typename ImgAccessor, typename ImgIterator>
std::pair<vigra::ConstStridedImageIterator<PixelType>, ImgAccessor>
stride(int xstride, int ystride,
       std::pair<vigra::ConstBasicImageIterator<PixelType, ImgIterator>, ImgAccessor> image)
{
    typedef vigra::ConstStridedImageIterator<PixelType> CSII;

    return std::make_pair(CSII(image.first[0],
                               iteratorWidth(image.first),
                               xstride, ystride),
                          image.second);
}


} // namespace vigra_ext


#endif // STRIDE_HXX
