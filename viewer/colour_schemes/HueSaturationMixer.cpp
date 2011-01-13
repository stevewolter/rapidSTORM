#include "HueSaturationMixer.h"
#include <cassert>

#include <dStorm/image/constructors.h>

using namespace boost::units;


namespace dStorm {
namespace viewer {
namespace colour_schemes {

void HueSaturationMixer::set_tone( float hue ) {
    float saturation = 0;
    current_tone = base_tone + ColourVector( hue, saturation );
    tone_point.x() = cosf( 2 * M_PI * current_tone[0] ) 
                        * current_tone[1];
    tone_point.y() = sinf( 2 * M_PI * current_tone[0] ) 
                        * current_tone[1];
}

void HueSaturationMixer::merge_tone( int x, int y, 
                    float old_data_weight, float new_data_weight )
{
    assert( int(colours.width_in_pixels()) > x && int(colours.height_in_pixels()) > y );
    ColourVector hs;
    if ( old_data_weight < 1E-3 ) {
        colours(x,y) = tone_point;
        hs = current_tone;
    } else {
        colours(x,y) = 
            ( colours(x,y) * old_data_weight + 
                tone_point * new_data_weight ) /
                ( old_data_weight + new_data_weight );
        convert_xy_tone_to_hue_sat
            ( colours(x,y).x(), colours(x,y).y(), hs[0], hs[1] );
    }
    rgb_weights_from_hue_saturation( hs[0], hs[1], rgb_weights(x,y) );
            
}

HueSaturationMixer::HueSaturationMixer( double base_hue, double base_saturation ) {
    base_tone[0] = base_hue;
    base_tone[1] = base_saturation;
}
HueSaturationMixer::~HueSaturationMixer() {}

void HueSaturationMixer::setSize(const dStorm::Image<ColourVector,2>::Size& size) {
    colours.invalidate();
    rgb_weights.invalidate();
    colours = dStorm::Image<ColourVector,2>(size);
    rgb_weights = dStorm::Image<RGBWeight,2>(size);
}

}
}
}
