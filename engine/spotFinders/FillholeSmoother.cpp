#include "spotFinders/FillholeSmoother.h"

#include "spotFinders/Reconstruction.cc"
#include "engine/dilation.h"
#include "FillholeSmoother.h"
#include <CImg.h>

using namespace cimg_library;

namespace dStorm {

/** Produce an image that is \c v throughout but for the border of
 *  width \c border, which is from \c source. */
template <typename T> 
    void fill_border(CImg<T>& res, int border, const CImg<T>& src)
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
    CImg<T> get_invert(const CImg<T>& src)
{
    CImg<T> res(src.width, src.height, src.depth, src.dim);
    unsigned long sz = src.size();
    T max = std::numeric_limits<T>::max();
    for (unsigned long i = 0; i < sz; i++)
        res.data[i] = max - src.data[i];
    return res;
}

template <typename T>
    CImg<T>& invert(CImg<T>& src)
{
    unsigned long sz = src.size();
    T max = numeric_limits<T>::max();
    for (unsigned long i = 0; i < sz; i++)
        src.data[i] = max - src.data[i];
    return src;
}

template <typename T, typename U>
    void copyImage(CImg<T>& dst, const CImg<U>& src)
{
    T* t = dst.ptr();
    for (unsigned int i = 0; i < src.size(); i++)
        t[i] = src.ptr()[i];
}


void
#ifdef D3_INTERNAL_VERSION
FillholeSmoother::smooth( const Image &image ) {
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
#else
FillholeSmoother::smooth( const Image & ) {
#endif
}

FillholeSmoother::FillholeSmoother(const Config &conf, int imw, int imh)

: SpotFinder(conf, imw, imh),
    rms1(conf.reconstruction_mask_1()),
    rms2(conf.reconstruction_mask_2())
{
    for (int i = 0; i < 3; i++)
        buffer[i].reset(new SmoothedImage(imw, imh));
}

}
