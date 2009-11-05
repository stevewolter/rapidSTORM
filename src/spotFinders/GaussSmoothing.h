#ifndef DSTORM_GAUSSSMOOTHING_H
#define DSTORM_GAUSSSMOOTHING_H

#include <dStorm/data-c++/Vector.h>
#include <dStorm/engine/SpotFinder.h>
#include <simparm/Object.hh>
#include <simparm/Structure.hh>

namespace dStorm {
    class GaussSmoother : public SpotFinder {
        struct _Config : public simparm::Object {
            void registerNamedEntries() {}
            _Config() : simparm::Object("Gaussian", 
                "Smooth with gaussian kernel") {}
        };
      public:
        typedef simparm::Structure<_Config> Config;
        typedef SpotFinderBuilder<GaussSmoother> Factory;

        GaussSmoother (const Config&, const dStorm::Config &, 
                       int imw, int imh);

        void smooth( const Image &in );

      protected:
        data_cpp::Vector<int> xkern, ykern;
    };
}
#endif
