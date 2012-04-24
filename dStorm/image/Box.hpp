#ifndef DSTORM_IMAGE_BOX_HPP
#define DSTORM_IMAGE_BOX_HPP

#include "Box.h"

namespace dStorm {
namespace image {

template <int Dimensions>
Box<Dimensions> Box<Dimensions>::ZeroOrigin( typename ImageTypes<Dimensions>::Size sz )
{
    Box<Dimensions> rv;
    for (int i = 0; i < Dimensions; ++i)
        rv.sides[i] = Side( 0 * camera::pixel, sz[i] - 1 * camera::pixel );
    return rv;
}

template <int Dimensions>
Box<Dimensions>::Box( Position lower, Position upper )
{
    for (int i = 0; i < Dimensions; ++i)
        sides[i] = Side( lower[i], upper[i] );
}

template <int Dimensions>
Box<Dimensions> Box<Dimensions>::intersection( const Box& o ) const
{
    Box rv;
    for (int i = 0; i < Dimensions; ++i)
        rv.sides[i] = sides[i] & o.sides[i];
    return rv;
}


template <int Dimensions>
typename Box<Dimensions>::Span Box<Dimensions>::get_lower_edge(Direction dir) const
    { return lower( sides[dir] ); }
template <int Dimensions>
typename Box<Dimensions>::Span Box<Dimensions>::get_upper_edge(Direction dir) const
    { return upper( sides[dir] ); }
template <int Dimensions>
void Box<Dimensions>::set_range(Span lower, Span upper, Direction dir) 
    { sides[dir] = Side( lower, upper ); }

template <int Dimensions>
typename Box<Dimensions>::Position Box<Dimensions>::lower_corner() const {
    Position pos;
    for (int i = 0; i < Dimensions; ++i) pos[i] = lower( sides[i] );
    return pos;
}
template <int Dimensions>
typename Box<Dimensions>::Position Box<Dimensions>::upper_corner() const {
    Position pos;
    for (int i = 0; i < Dimensions; ++i) pos[i] = upper( sides[i] );
    return pos;
}

template <int Dimensions>
typename Box<Dimensions>::Span Box<Dimensions>::width( Direction dim ) const {
    return upper( sides[dim] ) - lower( sides[dim] ) + 1 * camera::pixel;
}

template <int Dimensions>
typename boost::units::power_typeof_helper< typename Box<Dimensions>::Span, boost::units::static_rational<Dimensions> >::type 
Box<Dimensions>::volume() const {
    int volume = 1;
    for (int i = 0; i < Dimensions; ++i)
        volume *= width( static_cast<Direction>(i) ).value();
    return boost::units::power_typeof_helper< typename Box<Dimensions>::Span, boost::units::static_rational<Dimensions> >::type::from_value(volume);
}

template <int Dimensions>
void Box<Dimensions>::const_iterator::advance(int n ) {
    pos[0] += n * camera::pixel;
    for (int i = 0; i < Dimensions-1; ++i) {
        while ( pos[i] < lower( box.sides[i] ) ) 
            { pos[i+1] -= 1 * camera::pixel; pos[i] += box.width(static_cast<Direction>(i)); }
        while ( pos[i] > upper( box.sides[i] ) ) 
            { pos[i+1] += 1 * camera::pixel; pos[i] -= box.width(static_cast<Direction>(i)); }
    }
}

template <int Dimensions>
typename Box<Dimensions>::const_iterator Box<Dimensions>::begin() const { return const_iterator( *this, lower_corner() ); }
template <int Dimensions>
typename Box<Dimensions>::const_iterator Box<Dimensions>::end() const { 
    Position end = lower_corner();
    end[Dimensions-1] = upper( sides[Dimensions-1] ) + 1 * camera::pixel;
    return const_iterator( *this, end ); 
}

}
}

#endif
