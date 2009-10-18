#ifndef DSTORM_DISPLAY_HELPERS_H
#define DSTORM_DISPLAY_HELPERS_H

#include <iostream>

#include "DataSource.h"

inline wxColor
makeColor( const dStorm::Display::Color& c ) {
    return wxColor( c.r, c.g, c.b );
}

extern const wxChar *SI_prefixes[];

extern void
make_SI_prefix( float original_value, float& rest, const wxChar *& unit_prefix );

extern std::ostream&
operator<<(std::ostream& o, const wxRect& rect );

#endif
