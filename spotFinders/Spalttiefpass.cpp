#include <simparm/BoostUnits.hh>
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/Image.h>
#include "averageSmooth.h"

#include <simparm/Object.hh>
#include <simparm/Structure.hh>
#include <simparm/Entry.hh>

namespace dStorm {
namespace spotFinders {

class Spalttiefpass : public engine::spot_finder::Base {
    struct _Config : public simparm::Object {
        simparm::Entry< quantity<camera::length,int> > mask_size;
        void registerNamedEntries() {}
        _Config() : simparm::Object("Average", "Smooth by average"),
            mask_size("SmoothingMaskSize", "Smoothing mask width", 5 * camera::pixel) {}
    };
    const _Config config;
public:
    typedef simparm::Structure<_Config> Config;
    typedef engine::spot_finder::Builder<Spalttiefpass> Factory;

    Spalttiefpass (const Config& config, const engine::spot_finder::Job& job )
        : Base(job), config(config) {}
    Spalttiefpass* clone() const { return new Spalttiefpass(*this); }

    void smooth( const engine::Image2D &in ) {
        smoothByAverage( in, smoothed, 
            config.mask_size() / camera::pixel / 2,
            config.mask_size() / camera::pixel / 2 );
    }
};

std::auto_ptr<engine::spot_finder::Factory> make_Spalttiefpass() { 
    return std::auto_ptr<engine::spot_finder::Factory>(new Spalttiefpass::Factory()); 
}

}
}
