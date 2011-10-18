#include "CandidateTree.h"
#include "Image.h"

using namespace std;

#define USE_ANEUBECK_NMS

#ifndef USE_ANEUBECK_NMS
#define DATA_CPP_MERGINGTREE_INLINE_COMPARISON
#include <dStorm/spotFinders/dilation.h>
#endif

#include <dStorm/data-c++/impl/MergingTree.cpp>

using namespace dStorm::engine;

namespace data_cpp {
    template class MergingTree<Candidate<SmoothedPixel> >;
    template class MergingTree<Candidate<float> >;
}

namespace dStorm {
namespace engine {

#include <iostream>
#include <cassert>

template <typename PixelType>
CandidateTree<PixelType>::~CandidateTree() {}

template <typename PixelType>
int CandidateTree<PixelType>::compare(
    const Candidate<PixelType> &a, const Candidate<PixelType> &b
) 
{
    if ( a.first < b.first )
        return 1;
    else if ( a.first > b.first )
        return -1;
    else {
        int dx = a.second.x() - b.second.x();
        if ( dx < -msx )
            return -1;
        else if ( dx > msx )
            return 1;
        else {
            int dy = a.second.y() - b.second.y();
            if ( dy < -msy )
                return -1;
            else if ( dy > msy )
                return 1;
            else
                return 0;
        }
    }
}

template <typename PixelType>
void CandidateTree<PixelType>::fill(const Input &i) 
#ifdef USE_ANEUBECK_NMS
{
   this->clear();
   /* LargestReject is a dynamic threshold based on the ordering relation
    * of maximums. If any given maximum strength is rejected, all maxima
    * of same or lower strength should be rejected; thus, it is safe to
    * ignore any maxima of this height. */
   PixelType largestReject = 0;

   const int W = i.width().value(), H = i.height().value();
   const int w = msx, h = msy;
   const PixelType* I = i.ptr();
   int cxs[(w+1)*(h+1)];
   int cys[(w+1)*(h+1)];
   const int lastValidX = W - (bx+1), lastValidY = H - (by+1);

    /* aneubeck's algorithm as documented on 
        * http://www.vision.ee.ethz.ch/~aneubeck/ */
    for( int y = by; y <= lastValidY; y += h+1 ) 
      for( int x = bx; x <= lastValidX; x += w+1 ){
        int no = -1;
        PixelType curMax = largestReject;

        const int window_right = min(x + w, lastValidX);
        const int window_bottom = min(y + h, lastValidY);
        for( int dx = x; dx <= window_right; dx++ )
            for( int dy = y; dy <= window_bottom; dy++ ) {
              assert( dx <= W-1 && dy <= H-1 );
              if( I[dx+W*dy] >= curMax ) {
                if (I[dx+W*dy] > curMax) no = -1;
                no++;
                cxs[no] = dx, cys[no] = dy;
                curMax = I[dx+W*dy];
              }
            }

         while (no >= 0) {
            const int cx = cxs[no], cy = cys[no];
            int commit_result;

#if 0
            /* Alternative code honoring the fact that the
             * local window was already checked. */

            for (int yy = cy-h; yy <= cy+h;yy++) {
                bool hole = (yy >= y && yy <= y+w);
                int xx = (hole && cx == x+w) ? cx+1 : cx - w;
                while (xx <= cx + w) {
                    if( I[cx+W*cy] < I[xx+W*yy] ) goto failed;
                    xx += (hole && xx == x) ? w+1 : 1;
                }
            }
#else
            /* Original aneubeck code. */
            const int left = max(cx - w, 0), top = max(cy - h, 0);
            const int right = min(cx + w, lastValidX),
                      bottom = min(cy + h, lastValidY);
            for (int yy = top; yy <= bottom; yy++)
                for (int xx = left; xx <= right; xx++) {
                    assert( xx <= W-1 && yy <= H-1 );
                    if( I[cx+W*cy] < I[xx+W*yy] ) goto failed;
                }
#endif
            
            new(this->add())
                Candidate<PixelType>(I[cx+W*cy], Spot(cx, cy));
            commit_result = this->commit();
            if (commit_result == -1) {
                largestReject = 
                    max<PixelType>(largestReject, I[cx+W*cy]);
            }
            failed: ;
            no--;
         }
      }
}
#else   /* USE_ANEUBECK_NMS */
{
    clear();

    rectangular_dilation(input, buffer, msx, msy, msx, msy);

#if 1
    for (int y = 2*msy; y < int(input.height-2*msy); y++)
      for (int x = 2*msx; x < int(input.height-2*msx); x++)
        if (input(x,y) == buffer(x,y)) {
            new(add()) Candidate<PixelType>(buffer(x,y), Spot(x, y));
            commit();
        }
#else
    /* Alternative code pre-merging maximums. */
    PixelType *lastLine = NULL, *line = NULL;
    const PixelType *lastSLine = NULL, *Sline = NULL;
    for (int y = 2*msy; y < int(input.height-2*msy); y++) {
        lastLine = line;
        lastSLine = Sline;
        line = buffer.ptr(0, y);
        Sline = input.ptr(0, y);

        line[2*msx-1] = line[input.width-2*msx] = 0;
        for (int x = 2*msx; x < int(input.width-2*msx); x++) {
            if (line[x] != Sline[x]) { line[x] = 0; continue; }

            int label;
            label = line[x-1];
            if (lastLine)
                label |= lastLine[x-1] | lastLine[x] | lastLine[x+1];

            if (label == 0) {
                new(add()) Candidate<PixelType>(line[x], Spot(x, y));
                label = commit() + 2;
            } else if (label == 1) {
                /* Candidate was discarded. No action necessary. */
            } else {
                Candidate<PixelType> &m = get(label-2);
                m.second.add(x, y);
            }

            line[x] = label;
        }
    }
#endif

}
#endif

template <typename PixelType>
void CandidateTree<PixelType>::fillMax(const Input &i) 
{
   this->clear();
   /* LargestReject is a dynamic threshold based on the ordering relation
    * of maximums. If any given maximum strength is rejected, all maxima
    * of same or lower strength should be rejected; thus, it is safe to
    * ignore any maxima of this height. */
   PixelType largestReject = 0;

   const int W = i.width().value(), H = i.height().value();
   const int w = msx, h = msy;
   const PixelType* I = i.ptr();
   const int lastValidX = W - (w+w+1), lastValidY = H - (h+h+1);

    /* aneubeck's algorithm as documented on 
        * http://www.vision.ee.ethz.ch/~aneubeck/ */
    for( int y = h+h; y <= lastValidY; y += h+1 ) 
      for( int x = w+w; x <= lastValidX; x += w+1 ){
        int cx = -1, cy = -1;
        PixelType curMax = largestReject;

        for( int dx = 0; dx <= w; dx++ )
            for( int dy = 0; dy <= h; dy++ )
            if( I[(x+dx)+W*(y+dy)] > curMax ) {
                cx = x+dx, cy = y+dy;
                curMax = I[(x+dx)+W*(y+dy)];
            } else if ( I[(x+dx)+W*(y+dy)] == curMax ) {
                if (cx == -1) {
                    cx = x+dx, cy = y+dy;
                } else
                    cx = -1;
            }

        int commit_result;
        if (cx < 0 || cx > lastValidX || cy > lastValidY)
            goto failed;

        for (int yy = cy-h; yy <= cy+h; yy++) {
            bool hole = (yy >= y && yy <= y+w);
            int xx = (hole && cx == x+w) ? cx+1 : cx - w;
            while (xx <= cx + w) {
                if( I[cx+W*cy] < I[xx+W*yy] ) goto failed;
                xx += (hole && xx == x) ? w+1 : 1;
            }
        }
        
        new(this->add()) Candidate<PixelType>(I[cx+W*cy], Spot(cx, cy));
        commit_result = this->commit();
        if (commit_result < 0) {
            largestReject = 
                max<PixelType>(largestReject, I[cx+W*cy]);
        }
        failed: ;
      }
}

template class CandidateTree<SmoothedPixel>;
template class CandidateTree<float>;

}
}
