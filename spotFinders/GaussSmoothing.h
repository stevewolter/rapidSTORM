#ifndef DSTORM_GAUSSSMOOTHING_H
#define DSTORM_GAUSSSMOOTHING_H

#include <dStorm/data-c++/Vector.h>
#include <dStorm/engine/SpotFinder.h>
#include <simparm/Object.hh>
#include <simparm/Structure.hh>

namespace dStorm {
namespace spotFinders {

    class GaussSmoother : public engine::spot_finder::Base {
        struct _Config : public simparm::Object {
            void registerNamedEntries() {}
            _Config() : simparm::Object("Gaussian", 
                "Smooth with gaussian kernel") {}
        };
      public:
        typedef simparm::Structure<_Config> Config;
        typedef engine::spot_finder::Builder<GaussSmoother> Factory;

        GaussSmoother (const Config&, const engine::spot_finder::Job&);

        void smooth( const engine::Image2D &in );

      protected:
        data_cpp::Vector<int> xkern, ykern;
    };

}
}
#endif
