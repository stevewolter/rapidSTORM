#ifndef DSTORM_FILLHOLESMOOTHING_H
#define DSTORM_FILLHOLESMOOTHING_H

#include <simparm/Object.hh>
#include <simparm/Structure.hh>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/SpotFinder.h>

namespace locprec {
    class FillholeSmoother : public dStorm::engine::spot_finder::Base {
      private:
        dStorm::engine::SmoothedImage buffer[3];
        int rms1, rms2;

        class _Config;
      public:
        typedef simparm::Structure<_Config> Config;
        typedef dStorm::engine::spot_finder::Builder<FillholeSmoother> Factory;

        FillholeSmoother (const Config& myconf,
                          const dStorm::engine::spot_finder::Job &conf);
        ~FillholeSmoother() {}
        FillholeSmoother* clone() const { return new FillholeSmoother(*this); }

        void smooth( const dStorm::engine::Image2D &in );
    };

    class FillholeSmoother::_Config : public simparm::Object
    {
      protected:
        void registerNamedEntries();
      public:
        /** Mask sizes */
        simparm::Entry<unsigned long> spots, background;

        _Config();
    };
}
#endif
