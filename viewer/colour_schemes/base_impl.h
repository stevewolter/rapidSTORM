#ifndef DSTORM_VIEWER_COLOUR_SCHEMES_BASE_IMPL_H
#define DSTORM_VIEWER_COLOUR_SCHEMES_BASE_IMPL_H

#include "base.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

inline dStorm::Pixel operator*( const RGBWeight& r, uint8_t b ) {
    return dStorm::Pixel( round(r[0] * b), round(r[1] * b), round(r[2] * b) );
}
inline dStorm::Pixel operator*( uint8_t b, const RGBWeight& r ) {
    return dStorm::Pixel( r[0] * b, r[1] * b, r[2] * b );
}

}
}
}

#endif
