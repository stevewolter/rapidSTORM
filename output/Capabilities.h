#ifndef DSTORM_OUTPUT_CAPABILITIES_H
#define DSTORM_OUTPUT_CAPABILITIES_H

#include <bitset>

namespace dStorm {
namespace output {

class Capabilities : public std::bitset<2>
{
  public:
    Capabilities() {}

    enum Capability {
        SourceImage,
        InputBuffer,
    };

    Capabilities& set_source_image(bool has_cap = true)
        { set( SourceImage, has_cap ); return *this; }
    Capabilities& set_input_buffer(bool has_cap = true)
        { set( InputBuffer, has_cap ); return *this; }
};

std::ostream& operator<<(std::ostream&, Capabilities caps);

}
}

#endif
