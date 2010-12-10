#include "FillholeSmoother.h"

#include "Reconstruction.hh"
#define RECONSTRUCTION_IMPL_TYPE unsigned short
#include "Reconstruction.cc"
#undef RECONSTRUCTION_IMPL_TYPE
#define RECONSTRUCTION_IMPL_TYPE unsigned int
#include "Reconstruction.cc"
#undef RECONSTRUCTION_IMPL_TYPE

#include "dilation.h"

using namespace cimg_library;

namespace dStorm {

/** Produce an image that is \c v throughout but for the border of
 *  width \c border, which is from \c source. */
template <typename T> 
    void fill_border(CImg<T>& res, int border, const CImg<T>& src) throw()
{
    for (int i = 0; i < border; i++) {
        cimg_forX( res, x ) {
            res(i, x) = src(i, x);
            res(res.height-1-i,x) = src(src.height-1-i, x);
        }
        cimg_forY( res, y ) {
            res(y, i) = src(y, i);
            res(y, res.width-1-i) = src(y, res.width-1-i);
        }
    }
}

template <typename T>
    CImg<T> get_invert(const CImg<T>& src) throw()
{
    CImg<T> res(src.width, src.height, src.depth, src.dim);
    unsigned long sz = src.size();
    T max = numeric_limits<T>::max();
    for (unsigned long i = 0; i < sz; i++)
        res.data[i] = max - src.data[i];
    return res;
}

template <typename T>
    CImg<T>& invert(CImg<T>& src) throw()
{
    unsigned long sz = src.size();
    T max = numeric_limits<T>::max();
    for (unsigned long i = 0; i < sz; i++)
        src.data[i] = max - src.data[i];
    return src;
}

template <typename T, typename U>
    void copyImage(CImg<T>& dst, const CImg<U>& src) throw()
{
    T* t = dst.ptr();
    for (unsigned int i = 0; i < src.size(); i++)
        t[i] = src.ptr()[i];
}


void
FillholeSmoother::prepare( const Image &image ) throw() {
        SmoothedImage &inv_image = *buffer[0],
                      &inv_fillhole_mask = *buffer[1],
                      &recon = *buffer[2], &bg = *buffer[0];
        /* Remember that reconstruction by erosion is equivalent
         * to the invert of reconstruction by dilation using the
         * inverts of the mask and marker image. */
        copyImage(inv_image, image);
        invert(inv_image);

        inv_fillhole_mask.fill(0);
        fill_border(inv_fillhole_mask, 1, inv_image);

        /* This is effectively the fillhole transformation on image to
         * recon. */
        ReconstructionByDilationII<SmoothedPixel> 
            (inv_fillhole_mask, inv_image, recon);
        invert<SmoothedPixel>(recon);

        /* Would be faster, but border can't be accounted for. */
        rectangular_erosion( recon, bg, rms2/2, rms2/2, 0, 0 );
        rectangular_dilation( bg, bg, rms2/2, rms2/2, 0, 0 );

        rectangular_erosion( recon, recon, rms1/2, rms1/2, 0, 0 );

        unsigned int sz = recon.size();
        for (unsigned int i = 0; i < sz; i++)
            smoothed->data[i] = recon.data[i];
}

}
