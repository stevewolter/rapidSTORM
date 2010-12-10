#ifndef DSTORM_VIEWPORT_H
#define DSTORM_VIEWPORT_H

#include <CImg.h>
#include <dStorm/Histogram.h>
#include <iostream>
#include <cassert>
#include <cc++/thread.h>
#include <limits>

using namespace std;

namespace dStorm {
    class ViewportImage {
      public:
        ViewportImage(int dimNumber, int filledDimension, bool inv)     
            throw() 
        : histogram(4096, inv), x0(-1), x1(-2), y0(-1), y1(-2),
          needRecomputation(false), dimNumber(dimNumber),
          filledDimension(filledDimension), invert(inv)
        {
        }

        void setBaseImageSize(int w, int h) throw() { 
            baseWidth = w; baseHeight = h; 
            x1 = std::min<int>(w-1,x1);
            y1 = std::min<int>(h-1,y1);
            x0 = std::min<int>(x0,x1);
            y0 = std::min<int>(y0,y1);
            viewport.resize(width(), height(), 1, dimNumber);
            PROGRESS("Setting viewport to " << x0 <<"-"<< x1 << "-" << y0<< "-" << y1);
            needRecomputation = true;
            viewport.fill( (invert) 
                ? std::numeric_limits<unsigned char>::max() 
                : std::numeric_limits<unsigned char>::min() );
        }

        void setViewport(int x0, int x1, int y0, int y1) throw() {
            PROGRESS("Setting viewport to " << x0 <<"-"<< x1 << "-" << y0<< "-" << y1);
            this->x0 = x0; this->x1 = x1; this->y0 = y0; this->y1 = y1; 
            changesInViewport = width() * height();
            viewport.resize(width(), height(), 1, dimNumber);
            viewport.fill( (invert) 
                ? std::numeric_limits<unsigned char>::max() 
                : std::numeric_limits<unsigned char>::min() );
            needRecomputation = true;
        }

        inline void setPixel(int x, int y, SmoothedPixel val) throw() {
            unsigned char &oldVal = viewport(x-x0, y-y0, 0, 
                                                filledDimension),
                            newVal = histogram.convert(val);
            if (oldVal != newVal) {
                oldVal = newVal;
                changesInViewport++;
            }
        }
        void update(int x, int y, SmoothedPixel val) throw() {
            if (x0 <= x && x <= x1 && y0 <= y && y <= y1) {
                setPixel(x,y,val);
            }
        }

        void recompute(const SmoothedImage& from)
            throw()
        {
            if (x0>x1 || y0>y1) return;
            assert( x0 >= 0 && y0 >= 0 );
            assert( x1-x0 < (int)viewport.width && 
                    y1-y0 < (int)viewport.height);
            assert( x1 < (int)from.width && x1 < (int)from.height );
            for (int x = x0; x <= x1; x++)
              for (int y = y0; y <= y1; y++)
                setPixel(x,y, from(x,y) );
            needRecomputation = false;
        }

        bool needsRecompute() const throw() { return needRecomputation; }
        bool isDirty(int threshold) const throw()
            { return changesInViewport >= threshold; }
        const cimg_library::CImg<unsigned char>& redisplay() throw()
            { changesInViewport = 0; return viewport; }

        unsigned int leftBorder() const throw() { return x0; }
        unsigned int rightBorder() const throw() { return x1; }
        unsigned int topBorder() const throw() { return y0; }
        unsigned int bottomBorder() const throw() { return y1; }
        unsigned int width() const throw() { return x1-x0+1; }
        unsigned int height() const throw() { return y1-y0+1; }

        void shift(float xpages, float ypages) throw() {
            int nx0 = int(width() * xpages)+x0,
                ny0 = int(height() * ypages)+y0;
            nx0 = std::max<int>(0, 
                    std::min<int>( baseWidth-width(), nx0 ));
            ny0 = std::max<int>(0, 
                    std::min<int>( baseHeight-height(), ny0 ));

            if (x0 != nx0 || y0 != ny0 ) {
                x1 = x1+ (nx0-x0);
                x0 = nx0;
                y1 = y1+ (ny0-y0);
                y0 = ny0;
                changesInViewport = width() * height();
            PROGRESS("Setting viewport to " << x0 <<"-"<< x1 << "-" << y0<< "-" << y1);
            }
        }

        const NormalizedHistogram& getHistogram() const throw()
            { return histogram; }
        NormalizedHistogram& getHistogram() throw()
            { return histogram; }

        inline int getNumberOfDimensions() const throw()
            { return dimNumber; }
        inline int getFilledDimension() const throw()
            { return filledDimension; }
        
      private:
        cimg_library::CImg<unsigned char> viewport;
        NormalizedHistogram histogram;
        int x0, x1, y0, y1;
        int baseWidth, baseHeight;
        int changesInViewport;
        bool needRecomputation;

        int dimNumber, filledDimension;
        bool invert;
    };
};

#endif
