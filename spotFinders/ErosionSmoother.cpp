#include <simparm/BoostUnits.h>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFinderBuilder.h>
#include <simparm/Entry.h>
#include <dStorm/image/dilation.h>
#include <simparm/Object.h>
#include <simparm/GUILabelTable.h>

namespace dStorm {
namespace erosion_smoother {

struct Config {
    simparm::Entry< quantity<camera::length,int> > mask_size;
    void attach_ui( simparm::NodeHandle at ) { mask_size.attach_ui( at ); }
    static std::string get_name() { return "Erosion"; }
    static std::string get_description() { return simparm::GUILabelTable::get_singleton().get_description( get_name() ); }
    Config() 
        : mask_size("SmoothingMaskSize", 3 * camera::pixel) {}

};
class SpotFinder : public engine::spot_finder::Base {
    const Config config;
public:
    SpotFinder (const Config& c, const engine::spot_finder::Job& job)
        : Base(job), config(c) {}
    ~SpotFinder() {}
    SpotFinder* clone() const { return new SpotFinder(*this); }

    void smooth( const engine::Image2D &in ) {
        const int mw = config.mask_size()/2/camera::pixel;
        rectangular_erosion( in, smoothed, mw, mw, 0, 0);
    }
};

std::auto_ptr<engine::spot_finder::Factory> make_spot_finder_factory() { 
    return std::auto_ptr<engine::spot_finder::Factory>(
        new engine::spot_finder::Builder< Config, SpotFinder >()); 
}

}
}
