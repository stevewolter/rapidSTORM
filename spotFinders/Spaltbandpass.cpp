#include <simparm/BoostUnits.h>
#include "engine/SpotFinder.h"
#include "engine/SpotFinderBuilder.h"
#include "engine/Image.h"
#include "spotFinders/averageSmooth.h"
#include "image/iterator.h"

#include <simparm/Object.h>
#include <simparm/Entry.h>
#include <simparm/GUILabelTable.h>

namespace dStorm {
namespace spaltbandpass_smoother {

struct Config {
    simparm::Entry< quantity<camera::length,int> > fg_mask, bg_mask;
    void attach_ui( simparm::NodeHandle at ) { fg_mask.attach_ui( at ); bg_mask.attach_ui( at ); }
    Config() 
      : fg_mask("ForegroundMaskSize", 5 * camera::pixel),
        bg_mask("BackgroundMaskSize", 15 * camera::pixel)
    {
        fg_mask.set_user_level( simparm::Intermediate );
        bg_mask.set_user_level( simparm::Intermediate );
    }
    static std::string get_name() { return "DifferenceOfAverage"; }
    static std::string get_description() { return simparm::GUILabelTable::get_singleton().get_description( get_name() ); }
};

class Spaltbandpass : public engine::spot_finder::Base {
    typedef engine::SmoothedImage Image;
    const Config config;
    Image background;
public:
    Spaltbandpass (const Config& config, const engine::spot_finder::Job& job )
        : Base(job), config(config), background( this->smoothed.sizes() ) {}
    Spaltbandpass* clone() const { return new Spaltbandpass(*this); }

    void smooth( const engine::Image2D &in ) {
        spotFinders::smoothByAverage( in, smoothed, 
            config.fg_mask() / camera::pixel / 2,
            config.fg_mask() / camera::pixel / 2 );
        spotFinders::smoothByAverage( in, background, 
            config.bg_mask() / camera::pixel / 2,
            config.bg_mask() / camera::pixel / 2 );
        Image::const_iterator bg = background.begin();
        for ( Image::iterator i = smoothed.begin(); i != smoothed.end(); ++i ) {
            *i = ( *i > *bg ) ? *i - *bg : 0;
            ++bg;
        }
    }
};

std::auto_ptr<engine::spot_finder::Factory> make_spot_finder_factory() { 
    return std::auto_ptr<engine::spot_finder::Factory>(
        new engine::spot_finder::Builder<Config,Spaltbandpass>()); 
}

}
}
