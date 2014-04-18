#include "engine/CandidateTree.h"
#include "engine/Image.h"
#include <iostream>
#include <cassert>

#define USE_ANEUBECK_NMS

namespace dStorm {
namespace engine {

using std::min;
using std::max;

template <typename PixelType>
CandidateTree<PixelType>::~CandidateTree() {}

template <typename PixelType>
void CandidateTree<PixelType>::insert( const Element e, PixelType& largestReject ) {
    typedef typename std::vector< Candidate<PixelType> >::iterator Iter;
    std::pair<Iter,Iter> places = 
        std::equal_range( elements.begin(), elements.end(), e, 
            typename Candidate<PixelType>::decreasing_strength() );
    for ( Iter i = places.first; i != places.second; ++i ) {
        if ( i->spot().closer_than( e.spot(), msx, msy ) ) {
            i->merge( e );
            return;
        }
    }
    elements.insert( places.first, e );
    if ( elements.size() > limit_ ) {
        elements.pop_back();
        largestReject = elements.back().strength();
    }
}

template <typename PixelType>
void CandidateTree<PixelType>::fill(const Input &i) 
#ifdef USE_ANEUBECK_NMS
{
   elements.clear();
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

            /* Original aneubeck code. */
            const int left = max(cx - w, 0), top = max(cy - h, 0);
            const int right = min(cx + w, lastValidX),
                      bottom = min(cy + h, lastValidY);
            for (int yy = top; yy <= bottom; yy++)
                for (int xx = left; xx <= right; xx++) {
                    assert( xx <= W-1 && yy <= H-1 );
                    if( I[cx+W*cy] < I[xx+W*yy] ) goto failed;
                }
            
            insert( Candidate<PixelType>(I[cx+W*cy], Spot(cx, cy)), largestReject );
            failed: ;
            no--;
         }
      }
}
#else   /* USE_ANEUBECK_NMS */
{
    clear();

    rectangular_dilation(input, buffer, msx, msy, msx, msy);

    for (int y = 2*msy; y < int(input.height-2*msy); y++)
      for (int x = 2*msx; x < int(input.height-2*msx); x++)
        if (input(x,y) == buffer(x,y)) {
            new(add()) Candidate<PixelType>(buffer(x,y), Spot(x, y));
            commit();
        }

}
#endif

template <typename PixelType>
void CandidateTree<PixelType>::fillMax(const Input &i) 
{
   elements.clear();
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
        
        insert( Candidate<PixelType>(I[cx+W*cy], Spot(cx, cy)), largestReject );
        failed: ;
      }
}

template class CandidateTree<SmoothedPixel>;
template class CandidateTree<float>;

}
}
