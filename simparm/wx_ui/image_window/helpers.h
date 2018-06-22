#ifndef DSTORM_DISPLAY_HELPERS_H
#define DSTORM_DISPLAY_HELPERS_H

#include <iostream>

#include "display/DataSource.h"

inline wxColor
makeColor( const dStorm::display::Color& c ) {
    return wxColor( c.red(), c.green(), c.blue() );
}

extern const wxChar *SI_prefixes[];

extern void
make_SI_prefix( float original_value, float& rest, const wxChar *& unit_prefix );

extern std::ostream&
operator<<(std::ostream& o, const wxRect& rect );

#endif
