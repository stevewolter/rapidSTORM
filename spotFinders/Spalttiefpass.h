#ifndef DSTORM_SPALTTIEFPASS_H
#define DSTORM_SPALTTIEFPASS_H

#include <dStorm/data-c++/Vector.h>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/SpotFinder.h>
#include "averageSmooth.h"

#include <simparm/Object.hh>
#include <simparm/Structure.hh>

namespace cimg_library {
    template <typename Pixel> class CImg;
}

namespace dStorm {
namespace spotFinders {
    class Spalttiefpass : public engine::SpotFinder {
        struct _Config : public simparm::Object {
            void registerNamedEntries() {}
            _Config() : simparm::Object("Average", "Smooth by average") {}
        };
      public:
        typedef simparm::Structure<_Config> Config;
        typedef engine::SpotFinderBuilder<Spalttiefpass> Factory;

        Spalttiefpass (const Config&, const engine::Config &conf,
            int imw, int imh) 
        : SpotFinder(conf, imw, imh)
        {}

        void smooth( const engine::Image &in ) {
            smoothByAverage( in, *smoothed, msx, msy );
        }
    };
}
}
#endif
