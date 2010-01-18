#ifndef DSTORM_GAUSSSMOOTHING_H
#define DSTORM_GAUSSSMOOTHING_H

#include <dStorm/data-c++/Vector.h>
#include <dStorm/engine/SpotFinder.h>
#include <simparm/Object.hh>
#include <simparm/Structure.hh>

namespace dStorm {
namespace spotFinders {

    class GaussSmoother : public engine::SpotFinder {
        struct _Config : public simparm::Object {
            void registerNamedEntries() {}
            _Config() : simparm::Object("Gaussian", 
                "Smooth with gaussian kernel") {}
        };
      public:
        typedef simparm::Structure<_Config> Config;
        typedef engine::SpotFinderBuilder<GaussSmoother> Factory;

        GaussSmoother (const Config&, const engine::Config &, 
                       const engine::Traits::Size& size );

        void smooth( const engine::Image &in );

      protected:
        data_cpp::Vector<int> xkern, ykern;
    };

}
}
#endif
