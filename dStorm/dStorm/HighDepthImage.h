#ifndef DSTORM_HIGHDEPTHIMAGE_H
#define DSTORM_HIGHDEPTHIMAGE_H

#include <dStorm/Transmission.h>
#include <CImg.h>
#include <dStorm/ViewportImage.h>
#include <dStorm/Histogram.h>
#include <iostream>

namespace dStorm {

    class HighDepthImage {
      public:
        HighDepthImage(int depth, float rediscThreshold) throw()
            : depth(depth), discretFactor(0), thres(rediscThreshold) {}

        void setSize(int width, int height) throw() { 
            image.resize(width, height, 1, 1); 
            clear();
            if (target) target->setBaseImageSize(width, height);
        }

        inline void updatePixel(int x, int y, float val) throw() {
            if (val > currentMax) {
                currentMax = val;
                dirty = ( (currentMax / discretMax) > thres );
            }

            if (dirty) return;
            SmoothedPixel oldVal, newVal;
            oldVal = image(x,y);
            newVal = (SmoothedPixel)(discretFactor*val);
            newVal = std::min<SmoothedPixel>(depth-1, newVal);

            if (oldVal != newVal) {
                image(x,y) = newVal;
                if (histogram) histogram->changeValue( oldVal, newVal );
                if (target) 
                    target->update( x, y, newVal );
            }
        }

        void clean(const cimg_library::CImg<float>& src) throw();
        void clear() throw();

        void setListener(NormalizedHistogram *h) throw() {histogram=h;}
        void setListener(ViewportImage *i) throw() {target=i;}

        const SmoothedImage& getImage() const throw() { return image; }

      private:
        SmoothedImage image;
        int depth;
        float discretFactor, discretMax, currentMax, thres;
        bool dirty;

        NormalizedHistogram *histogram;
        ViewportImage *target;
    };
}

#endif
