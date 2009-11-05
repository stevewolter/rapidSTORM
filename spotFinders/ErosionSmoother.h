#ifndef DSTORM_EROSIONSMOOTHER_H
#define DSTORM_EROSIONSMOOTHER_H

#include <dStorm/data-c++/Vector.h>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/SpotFinder.h>
#include <simparm/Structure.hh>
#include <CImg.h>

namespace cimg_library {
    template <typename Pixel> class CImg;
}

namespace dStorm {
    class ErosionSmoother : public SpotFinder {
      private:
        cimg_library::CImg<bool> mask;
        struct _Config : public simparm::Object {
            void registerNamedEntries() {}
            _Config() : simparm::Object("Erosion", "Erode image") {}
        };
      public:
        typedef simparm::Structure<_Config> Config;
        typedef SpotFinderBuilder<ErosionSmoother> Factory;

        ErosionSmoother (const Config&, const dStorm::Config &conf,
                         int imw, int imh) 
        : SpotFinder(conf, imw, imh),
          mask(msx+(1-msx%2), msy+(1-msy%2))
        {
            mask.fill(true);
        }
        ~ErosionSmoother() {}

        void smooth( const Image &in ) {
            *smoothed = in;
            smoothed->erode<bool>(mask);
        }
    };
}
#endif
