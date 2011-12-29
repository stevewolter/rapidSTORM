#ifndef DSTORM_EROSIONSMOOTHER_H
#define DSTORM_EROSIONSMOOTHER_H

#include <dStorm/engine/Image.h>
#include <dStorm/engine/SpotFinder.h>
#include <simparm/Structure.hh>
#include <dStorm/helpers/dilation.h>
#include <simparm/Object.hh>

namespace dStorm {
    class ErosionSmoother : public engine::spot_finder::Base {
      private:
        const int mw, mh;
        struct _Config : public simparm::Object {
            void registerNamedEntries() {}
            _Config() : simparm::Object("Erosion", "Erode image") {}
        };
      public:
        typedef simparm::Structure<_Config> Config;
        typedef engine::spot_finder::Builder<ErosionSmoother> Factory;

        ErosionSmoother (const Config&, const engine::spot_finder::Job& job)
            : Base(job),
              mw(msx+(1-msx%2)), mh(msy+(1-msy%2))
            {}
        ~ErosionSmoother() {}
        ErosionSmoother* clone() const { return new ErosionSmoother(*this); }

        void smooth( const engine::Image2D &in ) {
            rectangular_erosion( in, smoothed, mw/2, mh/2, 0, 0);
        }
    };
}
#endif
