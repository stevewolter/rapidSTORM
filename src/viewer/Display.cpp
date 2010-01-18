#include "Display_impl.h"
#include "ColourDisplay_impl.h"

namespace dStorm {
namespace viewer {

template 
    class Display< HueingColorizer<ColourSchemes::BlackWhite> >;
template
    class Display< HueingColorizer<ColourSchemes::BlackRedYellowWhite> >;
template
    class Display< HueingColorizer<ColourSchemes::FixedHue> >;
template
    class Display< HueingColorizer<ColourSchemes::TimeHue> >;
template
    class Display<HueingColorizer<ColourSchemes::ExtraHue> >;
template
    class Display< HueingColorizer<ColourSchemes::ExtraSaturation> >;

}
}
