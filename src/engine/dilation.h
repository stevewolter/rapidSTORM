#include <dStorm/engine/Image.h>

namespace dStorm {
    /** Perform a dilation on image \c i and write the result into
     *  \c t. The radius (half the width) for X and Y direction are
     *  given in \c mrx and \c mry, respectively. \c borderX and 
     *  \c borderY give, if greater than 0, the width of the borders
     *  around the image which need _not_ be dilated.
     *
     *  If the mask is too close to the border, it is shrinked as
     *  necessary. */
    template <typename PixelType>
    void rectangular_dilation(
                const cimg_library::CImg<PixelType> &input,
                cimg_library::CImg<PixelType> &output, 
                const int mrx, const int mry,
                const int borderX, const int borderY)
;

    /** Perform an erosion on image \c i and write the result into
     *  \c t. The radius (half the width) for X and Y direction are
     *  given in \c mrx and \c mry, respectively. \c borderX and 
     *  \c borderY give, if greater than 0, the width of the borders
     *  around the image which need _not_ be eroded.
     *
     *  If the mask is too close to the border, it is shrinked as
     *  necessary. */
    template <typename PixelType>
    void rectangular_erosion(
                const cimg_library::CImg<PixelType> &input,
                cimg_library::CImg<PixelType> &output, 
                const int mrx, const int mry,
                const int borderX, const int borderY)
;
}
