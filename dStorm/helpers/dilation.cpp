#include <dStorm/Image.h>
#include <limits>

using namespace std;

#include <iostream>

namespace dStorm {

/** Algorithm from Gil & Werman, IEEE Transactions on Pattern Analysis
 *  and Machine Learning, Vol 15, No. 5. This algorithm is highly
 *  optimised and best understood with a piece of paper. */
template <typename PixelType, typename OutPixel>
static void dilate_line(const PixelType *src, const int step,
        const int w, const int p, OutPixel *trg)
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
template <typename PixelType, typename OutPixel>
static void erode_line(const PixelType *src, const int step,
        const int w, const int p, OutPixel *trg)
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

template <typename PixelType, typename OutPixel>
void rectangular_dilation(const Image<PixelType,2> &i,
                       Image<OutPixel,2> &t, 
                       const int mrx, const int mry,
                       const int borderX, const int borderY)

{
    const int width = i.width_in_pixels(), height = i.height_in_pixels();

    if ( mry > 1 ) {
        for (int x = borderX; x < int(width)-borderX; x++) {
            dilate_line(i.ptr(x, borderY), width, height-2*borderY, 
                2*mry+1, t.ptr(x, borderY));
        }
    }

    if ( mry > 1 && mrx > 1 ) {
        for (int y = borderY; y < int(height)-borderY; y++)
            dilate_line(t.ptr(borderX, y), 1, width-2*borderX, 2*mrx+1,
                    t.ptr(borderX, y));
    } else if ( mrx > 1 ) {
        for (int y = borderY; y < int(height)-borderY; y++)
            dilate_line(i.ptr(borderX, y), 1, width-2*borderX, 2*mrx+1,
                    t.ptr(borderX, y));
    }
}

template <typename PixelType, typename OutPixel>
void rectangular_erosion(const Image<PixelType,2> &i,
                       Image<OutPixel,2> &t, 
                       const int mrx, const int mry,
                       const int borderX, const int borderY)

{
    const int width = i.width_in_pixels(), height = i.height_in_pixels();

    if ( mry > 1 ) {
        for (int x = borderX; x < int(width)-borderX; x++) {
            erode_line(i.ptr(x, borderY), width, height-2*borderY, 
                2*mry+1, t.ptr(x, borderY));
        }
    }

    if ( mry > 1 && mrx > 1 ) {
        for (int y = borderY; y < int(height)-borderY; y++)
            erode_line(t.ptr(borderX, y), 1, width-2*borderX, 2*mrx+1,
                    t.ptr(borderX, y));
    } else if ( mrx > 1 ) {
        for (int y = borderY; y < int(height)-borderY; y++)
            erode_line(i.ptr(borderX, y), 1, width-2*borderX, 2*mrx+1,
                    t.ptr(borderX, y));
    }
}
}

#include <dStorm/engine/Image.h>

namespace dStorm {

using dStorm::engine::StormPixel;
using dStorm::engine::SmoothedPixel;

template void rectangular_dilation<bool>(
        const Image<bool,2> &i,
        Image<bool,2> &t,
        const int mrx, const int mry,
        const int borderX, const int borderY);

template void rectangular_erosion<bool>(
        const Image<bool,2> &i,
        Image<bool,2> &t,
        const int mrx, const int mry,
        const int borderX, const int borderY);

template void rectangular_dilation<StormPixel,SmoothedPixel>(
        const Image<StormPixel,2> &i,
        Image<SmoothedPixel,2> &t,
        const int mrx, const int mry,
        const int borderX, const int borderY);

template void rectangular_erosion<StormPixel,SmoothedPixel>(
        const Image<StormPixel,2> &i,
        Image<SmoothedPixel,2> &t,
        const int mrx, const int mry,
        const int borderX, const int borderY);

}
