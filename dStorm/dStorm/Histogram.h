#ifndef DSTORM_HIGHRESOLUTION_H
#define DSTORM_HIGHRESOLUTION_H

#include <dStorm/Transmission.h>
#include <dStorm/Image.h>
#include <CImg.h>
#include <data-c++/Vector.h>
#include <cassert>

namespace dStorm {
    class NormalizedHistogram {
      public:
        typedef SmoothedPixel In;
        typedef uint8_t Out;

        NormalizedHistogram(int depth, bool invert) throw()
            : histogram(depth, 0), transition(depth, 255), depth(depth),
              dirtyHistogramValues(0), histogramSum(0),
              resultDirty(false), power(0), invert(invert) {}

        void clear(int pixelCount) throw() {
            for (unsigned int i = 0; i < depth; i++) {
                histogram[i] = 0;
                transition[i] = (invert) ? 255 : 0;
            }
            histogram[0] = pixelCount; 
        }

        void compute(const cimg_library::CImg<In>& from) throw() {
            for (unsigned int i = 0; i < depth; i++)
                histogram[i] = 0;
            for (unsigned int i = 0; i < from.size(); i++) {
                assert(from[i] < depth);
                histogram[from[i]]++;
            }
            dirtyHistogramValues = histogramSum = from.size();
        }

        void changeValue(In oldValue, In newValue)
            throw()
        {
            assert(oldValue < depth && newValue < depth);
            histogram[oldValue]--;
            histogram[newValue]++;
            dirtyHistogramValues++;
        }

        inline Out convert(In input) throw()
            { return transition[input]; }

        void cleanHistogram() throw() {
            if (dirtyHistogramValues >= 100) normalizeHistogram();
        }

        bool isDirty() const throw() { return resultDirty; }
        void cleanTransition() throw() { resultDirty = false; }

        void setHistogramPower(float newPower) throw()
            { power = newPower; 
              dirtyHistogramValues = 100; }

      private:
        data_cpp::Vector<int> histogram;
        data_cpp::Vector<Out> transition;
        unsigned int depth, dirtyHistogramValues, histogramSum;
        bool resultDirty;
        float power;
        bool invert;

        void normalizeHistogram() throw();
    };
}

#endif
