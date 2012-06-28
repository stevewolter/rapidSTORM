#ifndef DSTORM_VIEWER_COLOUR_SCHEMES_H
#define DSTORM_VIEWER_COLOUR_SCHEMES_H

namespace dStorm {
namespace viewer {

class ColourSchemeFactory;

namespace colour_schemes {

std::auto_ptr<ColourSchemeFactory> make_hot_factory();
std::auto_ptr<ColourSchemeFactory> make_mono_factory();
std::auto_ptr<ColourSchemeFactory> make_colored_factory();
std::auto_ptr<ColourSchemeFactory> make_coordinate_factory();

}
}
}

#endif
