#ifdef HAVE_LIBMAGICK__
#define cimg_use_magick
#endif

namespace cimg_library {
    template <typename PixelType> class CImg;
};

namespace CImgBuffer {
    template <typename ObjectType> class Source;
    template <typename ObjectType> class Buffer;
};

namespace dStorm {
    /** Input pixel type for STORM engine. */
    typedef unsigned short StormPixel;
    /** Output pixel type for STORM engine. */
    typedef unsigned int SmoothedPixel;
    /** Input image type for STORM engine. */
    typedef cimg_library::CImg<StormPixel> Image;
    /** Output image type for STORM engine. */
    typedef cimg_library::CImg<SmoothedPixel> SmoothedImage;
}
