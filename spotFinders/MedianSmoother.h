#ifndef DSTORM_MEDIANSMOOTHER_H
#define DSTORM_MEDIANSMOOTHER_H

#include <dStorm/data-c++/Vector.h>
#include <dStorm/engine/SpotFinder.h>
#include <simparm/Object.hh>
#include <simparm/Structure.hh>

namespace dStorm {
namespace spotFinders {
    class MedianSmoother : public engine::spot_finder::Base {
      private:
        typedef engine::Image2D Image;
        typedef engine::SmoothedImage SmoothedImage;

        void naiveMedian(const Image &in, SmoothedImage& out, 
                         int mw, int mh);
        
        void (*ahmad)(const Image &in, SmoothedImage& out, int mw, int mh);
        void chooseAhmad(int msx, int msy);

        struct _Config : public simparm::Object {
            void registerNamedEntries() {}
            _Config() : simparm::Object("Median", "Smooth by median") {}
        };
      public:
        typedef simparm::Structure<_Config> Config;
        typedef engine::spot_finder::Builder<MedianSmoother> Factory;

        MedianSmoother (const Config&, 
            const engine::spot_finder::Job &job)
            : Base(job)
            { chooseAhmad(msx, msy); }
        MedianSmoother* clone() const { return new MedianSmoother(*this); }

        void smooth( const Image &in ) {
            ahmad( in, smoothed, 2*msx+1, 2*msy+1 );
        }
    };
}
}
#endif
