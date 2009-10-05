#ifndef DSTORM_HIGHRESOLUTION_H
#define DSTORM_HIGHRESOLUTION_H

#include <dStorm/Output.h>
#include <dStorm/Image.h>
#include <CImg.h>
#include <data-c++/Vector.h>
#include <cassert>
#include <outputs/HighDepthImage.h>

namespace dStorm {
}

namespace dStorm {
    template <
        typename UnnormalizedListener = DummyDiscretizationListener,
        typename NormalizedListener = DummyDiscretizationListener
    >
    class NormalizedHistogram {
      private:
        typedef HistogramPixel ValueNode;
      public:
        typedef SmoothedPixel In;
        typedef int Out;

        NormalizedHistogram(int in_depth, int out_depth)
            : histogram(in_depth, 0), 
              transition(in_depth, 0),
              value_lists(in_depth),
              in_depth(in_depth),
              out_depth(out_depth),
              dirtyHistogramValues(0), histogramSum(0),
              resultDirty(false), power(0) {}

        void setSize( int width, int height, int depth ) {
            assert( depth <= int(in_depth) );
            unnormalized->setSize( width, height, depth );

            pixelNumber = width * height;
            value_nodes = cimg_library::CImg<ValueNode>(width, height);
            cimg_forXY( value_nodes, x, y ) {
                value_nodes(x,y).x = x;
                value_nodes(x,y).y = y;
                value_nodes(x,y).clear();
            }

            normalized->setSize( width, height, out_depth );
        }

        void updatePixel(int x, int y, In oldValue, In newValue)
        {
            unnormalized->updatePixel(x, y, oldValue, newValue);

            assert(oldValue < in_depth && newValue < in_depth);
            histogram[oldValue]--;
            histogram[newValue]++;
            value_lists[newValue].push_back( value_nodes(x,y) );

            dirtyHistogramValues++;

            SmoothedPixel oldT = transition[oldValue],
                          newT = transition[newValue];
            if ( oldT != newT )
                normalized->updatePixel(x, y, oldT, newT);
        }

        void clear() {
            unnormalized->clear();
            for (unsigned int i = 0; i < in_depth; i++) {
                histogram[i] = 0;
                transition[i] = 0;
                value_lists[i].clear();
            }
            cimg_forXY( value_nodes, x, y )
                value_nodes(x,y).clear();
            histogram[0] = histogramSum = pixelNumber; 
            normalized->clear();
        }

        void clean() {
            unnormalized->clean();
            if (dirtyHistogramValues >= 100) normalizeHistogram();
            normalized->clean();
        }

        bool isDirty() const { return resultDirty; }
        void cleanTransition() { resultDirty = false; }

        void setHistogramPower(float newPower)
            { power = newPower; 
              dirtyHistogramValues = 100; }

        void setUnnormalizedListener(UnnormalizedListener *h) 
            { unnormalized.setListener(h); }
        void setNormalizedListener(NormalizedListener *h) 
            { normalized.setListener(h); }

      private:
        DiscretizationPublisher<UnnormalizedListener> unnormalized;
        DiscretizationPublisher<NormalizedListener> normalized;

        data_cpp::Vector<int> histogram;
        data_cpp::Vector<Out> transition;
        data_cpp::Vector<ValueNode> value_lists;
        cimg_library::CImg<ValueNode> value_nodes;

        unsigned int in_depth, out_depth,
                     dirtyHistogramValues, histogramSum,
                     pixelNumber;
        bool resultDirty;
        float power;

        void normalizeHistogram();
    };
}

#endif
