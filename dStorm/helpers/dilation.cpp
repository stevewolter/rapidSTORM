#include <CImg.h>
#include <dStorm/engine/Image.h>
#include <limits>

using namespace std;
using namespace dStorm::engine;

#include <iostream>

namespace dStorm {

/** Algorithm from Gil & Werman, IEEE Transactions on Pattern Analysis
 *  and Machine Learning, Vol 15, No. 5. This algorithm is highly
 *  optimised and best understood with a piece of paper. */
template <typename PixelType>
static void dilate_line(const PixelType *src, const int step,
        const int w, const int p, PixelType *trg)
{
    /* Maximum value in reverse scan */
    PixelType R[p-1];
    /* Maximum value in forward scan */
    PixelType S[p-1];
    /* Result for this line. Actually, buffering the last window of width
     * (p-1) would suffice, but this solution is easier.*/
    PixelType res_line[w];

    const PixelType *minP = src,
                        *maxP = src + w * step;
    for (int x0 = 0; x0 < w; x0 += p-1) {
        int w0 = x0 - p/2;
        const PixelType *Rj, *Sj;
        PixelType Rc, Sc;

        Rj = src + (w0 + (p-1)) * step;
        Sj = Rj - step;
        Rc = 0;
        Sc = 0;
        for (int j = 0; j < p-1; j++) {
            Rj -= step; Sj += step;
            if (Rj >= minP)
                Rc = max<PixelType>(*Rj, Rc);
            if (Sj < maxP)
                Sc = max<PixelType>(*Sj, Sc);
            R[j] = Rc;
            S[j] = Sc;
        }

        PixelType *T = res_line + x0;
        int rb = min<int>(p-1, w-x0);
        for (int j = 0; j < rb; j++) {
            *T = max<PixelType>(R[(p-2)-j], S[j]);
            T++;
        }
    }

    for (int i = 0; i < w; i++) {
        *trg = res_line[i];
        trg += step;
    }
}

/** Same as dilation, but modified for different neutral element. */
template <typename PixelType>
static void erode_line(const PixelType *src, const int step,
        const int w, const int p, PixelType *trg)
{
    PixelType R[p-1], S[p-1], res_line[w];

    const PixelType *minP = src,
                        *maxP = src + w * step;
    PixelType maxEl = numeric_limits<PixelType>::max();
    for (int x0 = 0; x0 < w; x0 += p-1) {
        int w0 = x0 - p/2;
        const PixelType *Rj, *Sj;
        PixelType Rc, Sc;

        Rj = src + (w0 + (p-1)) * step;
        Sj = Rj - step;
        Rc = maxEl;
        Sc = maxEl;
        for (int j = 0; j < p-1; j++) {
            Rj -= step; Sj += step;
            if (Rj >= minP)
                Rc = min<PixelType>(*Rj, Rc);
            if (Sj < maxP)
                Sc = min<PixelType>(*Sj, Sc);
            R[j] = Rc;
            S[j] = Sc;
        }

        PixelType *T = res_line + x0;
        int rb = min<int>(p-1, w-x0);
        for (int j = 0; j < rb; j++) {
            *T = min<PixelType>(R[(p-2)-j], S[j]);
            T++;
        }
    }

    for (int i = 0; i < w; i++) {
        *trg = res_line[i];
        trg += step;
    }
}

template <typename PixelType>
void rectangular_dilation(const cimg_library::CImg<PixelType> &i,
                       cimg_library::CImg<PixelType> &t, 
                       const int mrx, const int mry,
                       const int borderX, const int borderY)

{
    const cimg_library::CImg<PixelType>* xs = &i, *ys;
    cimg_library::CImg<PixelType>* xr = &t, *yr = &t;
    ys = (mry > 1) ? &t : &i;

    if ( mry > 1 ) {
        for (int x = borderX; x < int(i.width)-borderX; x++) {
            dilate_line(xs->ptr(x, borderY), i.width, i.height-2*borderY, 
                2*mry+1, xr->ptr(x, borderY));
        }
    }

    if ( mrx > 1 ) {
        for (int y = borderY; y < int(i.height)-borderY; y++)
            dilate_line(ys->ptr(borderX, y), 1, i.width-2*borderX, 2*mrx+1,
                    yr->ptr(borderX, y));
    }
}

template <typename PixelType>
void rectangular_erosion(const cimg_library::CImg<PixelType> &i, 
                       cimg_library::CImg<PixelType> &t, 
                       const int mrx, const int mry,
                       const int borderX, const int borderY)

{
    const cimg_library::CImg<PixelType>* xs = &i, *ys;
    cimg_library::CImg<PixelType>* xr = &t, *yr = &t;
    ys = (mry > 1) ? &t : &i;

    if ( mry > 1 ) {
        for (int x = borderX; x < int(i.width)-borderX; x++) {
            erode_line(xs->ptr(x, borderY), i.width, i.height-2*borderY, 
                2*mry+1, xr->ptr(x, borderY));
        }
    }

    if ( mrx > 1 ) {
        for (int y = borderY; y < int(i.height)-borderY; y++)
            erode_line(ys->ptr(borderX, y), 1, i.width-2*borderX, 2*mrx+1,
                    yr->ptr(borderX, y));
    }
}

template void rectangular_dilation<bool>(
        const cimg_library::CImg<bool> &i,
        cimg_library::CImg<bool> &t,
        const int mrx, const int mry,
        const int borderX, const int borderY);

template void rectangular_erosion<bool>(
        const cimg_library::CImg<bool> &i,
        cimg_library::CImg<bool> &t,
        const int mrx, const int mry,
        const int borderX, const int borderY);

template void rectangular_dilation<SmoothedPixel>(
        const cimg_library::CImg<SmoothedPixel> &i,
        cimg_library::CImg<SmoothedPixel> &t,
        const int mrx, const int mry,
        const int borderX, const int borderY);

template void rectangular_erosion<SmoothedPixel>(
        const cimg_library::CImg<SmoothedPixel> &i,
        cimg_library::CImg<SmoothedPixel> &t,
        const int mrx, const int mry,
        const int borderX, const int borderY);

}
