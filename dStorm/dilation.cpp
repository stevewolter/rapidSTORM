#include <CImg.h>
#include <CImgBuffer/Image.h>
#include <dStorm/Image.h>
#include <limits>

using namespace std;

#include <iostream>

namespace dStorm {

/** Algorithm from Gil & Werman, IEEE Transactions on Pattern Analysis
 *  and Machine Learning, Vol 15, No. 5. This algorithm is highly
 *  optimised and best understood with a piece of paper. */
static void dilate_line(const SmoothedPixel *src, const int step,
        const int w, const int p, SmoothedPixel *trg) throw()
{
    /* Maximum value in reverse scan */
    SmoothedPixel R[p-1];
    /* Maximum value in forward scan */
    SmoothedPixel S[p-1];
    /* Result for this line. Actually, buffering the last window of width
     * (p-1) would suffice, but this solution is easier.*/
    SmoothedPixel res_line[w];

    const SmoothedPixel *minP = src,
                        *maxP = src + w * step;
    for (int x0 = 0; x0 < w; x0 += p-1) {
        int w0 = x0 - p/2;
        const SmoothedPixel *Rj, *Sj;
        SmoothedPixel Rc, Sc;

        Rj = src + (w0 + (p-1)) * step;
        Sj = Rj - step;
        Rc = 0;
        Sc = 0;
        for (int j = 0; j < p-1; j++) {
            Rj -= step; Sj += step;
            if (Rj >= minP)
                Rc = max<SmoothedPixel>(*Rj, Rc);
            if (Sj < maxP)
                Sc = max<SmoothedPixel>(*Sj, Sc);
            R[j] = Rc;
            S[j] = Sc;
        }

        SmoothedPixel *T = res_line + x0;
        int rb = min<int>(p-1, w-x0);
        for (int j = 0; j < rb; j++) {
            *T = max<SmoothedPixel>(R[(p-2)-j], S[j]);
            T++;
        }
    }

    for (int i = 0; i < w; i++) {
        *trg = res_line[i];
        trg += step;
    }
}

/** Same as dilation, but modified for different neutral element. */
static void erode_line(const SmoothedPixel *src, const int step,
        const int w, const int p, SmoothedPixel *trg) throw()
{
    SmoothedPixel R[p-1], S[p-1], res_line[w];

    const SmoothedPixel *minP = src,
                        *maxP = src + w * step;
    SmoothedPixel maxEl = numeric_limits<SmoothedPixel>::max();
    for (int x0 = 0; x0 < w; x0 += p-1) {
        int w0 = x0 - p/2;
        const SmoothedPixel *Rj, *Sj;
        SmoothedPixel Rc, Sc;

        Rj = src + (w0 + (p-1)) * step;
        Sj = Rj - step;
        Rc = maxEl;
        Sc = maxEl;
        for (int j = 0; j < p-1; j++) {
            Rj -= step; Sj += step;
            if (Rj >= minP)
                Rc = min<SmoothedPixel>(*Rj, Rc);
            if (Sj < maxP)
                Sc = min<SmoothedPixel>(*Sj, Sc);
            R[j] = Rc;
            S[j] = Sc;
        }

        SmoothedPixel *T = res_line + x0;
        int rb = min<int>(p-1, w-x0);
        for (int j = 0; j < rb; j++) {
            *T = min<SmoothedPixel>(R[(p-2)-j], S[j]);
            T++;
        }
    }

    for (int i = 0; i < w; i++) {
        *trg = res_line[i];
        trg += step;
    }
}

void rectangular_dilation(const SmoothedImage &i, SmoothedImage &t, 
                       const int mrx, const int mry,
                       const int borderX, const int borderY)
    throw()
{
    for (int x = borderX; x < int(i.width)-borderX; x++)
        dilate_line(i.ptr(x, borderY), i.width, i.height-2*borderY, 
            2*mry+1, t.ptr(x, borderY));
    for (int y = borderY; y < int(i.height)-borderY; y++)
        dilate_line(t.ptr(borderX, y), 1, i.width-2*borderX, 2*mrx+1,
                 t.ptr(borderX, y));
}

void rectangular_erosion(const SmoothedImage &i, SmoothedImage &t, 
                       const int mrx, const int mry,
                       const int borderX, const int borderY)
    throw()
{
    for (int x = borderX; x < int(i.width)-borderX; x++)
        erode_line(i.ptr(x, borderY), i.width, i.height-2*borderY, 2*mry+1,
                 t.ptr(x, borderY));
    for (int y = borderY; y < int(i.height)-borderY; y++)
        erode_line(t.ptr(borderX, y), 1, i.width-2*borderX, 2*mrx+1,
                 t.ptr(borderX, y));
}
}
