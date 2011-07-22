#ifndef DSTORM_VIEWER_COLOUR_SCHEMES_BASE_IMPL_H
#define DSTORM_VIEWER_COLOUR_SCHEMES_BASE_IMPL_H

#include "base.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

inline dStorm::Pixel operator*( const RGBWeight& r, uint8_t b ) {
    for (int i = 0; i < 2; ++i) {
        assert ( int(round(r[i] * b )) >= std::numeric_limits<uint8_t>::min() );
        assert ( int(round(r[i] * b )) <= std::numeric_limits<uint8_t>::max() );
    }
    return dStorm::Pixel( round(r[0] * b), round(r[1] * b), round(r[2] * b) );
}
inline dStorm::Pixel operator*( uint8_t b, const RGBWeight& r ) {
    for (int i = 0; i < 2; ++i) {
        assert ( int(round(r[i] * b )) >= std::numeric_limits<uint8_t>::min() );
        assert ( int(round(r[i] * b )) <= std::numeric_limits<uint8_t>::max() );
    }
    return dStorm::Pixel( round(r[0] * b), round(r[1] * b), round(r[2] * b) );
}

}
}
}

#endif
