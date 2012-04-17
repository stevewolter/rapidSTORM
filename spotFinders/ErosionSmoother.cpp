#include <simparm/BoostUnits.hh>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/SpotFinder.h>
#include <simparm/Structure.hh>
#include <simparm/Entry.hh>
#include <dStorm/image/dilation.h>
#include <simparm/Object.hh>

namespace dStorm {
namespace spotFinders {

class ErosionSmoother : public engine::spot_finder::Base {
    struct _Config : public simparm::Object {
        simparm::Entry< quantity<camera::length,int> > mask_size;
        void registerNamedEntries() { push_back( mask_size ); }
        _Config() 
            : simparm::Object("Erosion", "Erode image"),
              mask_size("SmoothingMaskSize", "Smoothing mask width", 3 * camera::pixel) {}

    };
    const _Config config;
public:
    typedef simparm::Structure<_Config> Config;
    typedef engine::spot_finder::Builder<ErosionSmoother> Factory;

    ErosionSmoother (const Config& c, const engine::spot_finder::Job& job)
        : Base(job), config(c) {}
    ~ErosionSmoother() {}
    ErosionSmoother* clone() const { return new ErosionSmoother(*this); }

    void smooth( const engine::Image2D &in ) {
        const int mw = config.mask_size()/2/camera::pixel;
        rectangular_erosion( in, smoothed, mw, mw, 0, 0);
    }
};

std::auto_ptr<engine::spot_finder::Factory> make_Erosion() { 
    return std::auto_ptr<engine::spot_finder::Factory>(new ErosionSmoother::Factory()); 
}

}
}
