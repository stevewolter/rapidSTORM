#ifndef DSTORM_TRANSMISSIONS_COLOURDISPLAY_BASE_H
#define DSTORM_TRANSMISSIONS_COLOURDISPLAY_BASE_H

#include <dStorm/outputs/BinnedLocalizations.h>
#include <limits>
#include <dStorm/Pixel.h>
#include <dStorm/display/DataSource.h>
#include <boost/array.hpp>
#include <stdint.h>
#include "../Image.h"
#include "density_map/DummyListener.h"
#include "../ColourScheme.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

typedef boost::array<float,3> RGBWeight;

inline dStorm::Pixel operator*( const RGBWeight& r, uint8_t brightness );
inline dStorm::Pixel operator*( uint8_t brightness, const RGBWeight& r );

void rgb_weights_from_hue_saturation
    ( float hue, float saturation, RGBWeight& weight );
void convert_xy_tone_to_hue_sat( 
    float x, float y, float& hue, float& sat );

typedef ColourScheme Base;

}
}
}

#endif
