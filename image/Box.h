#ifndef DSTORM_IMAGE_RECTANGLE_H
#define DSTORM_IMAGE_RECTANGLE_H

#include "image/Image.h"
#include "units/icl.h"
#include <boost/icl/interval.hpp>
#include <boost/icl/closed_interval.hpp>
#include <boost/array.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include "Direction.h"

namespace dStorm {
namespace image {

template <int Dimensions>
class Box {
    typedef typename ImageTypes<Dimensions>::Span Span;
    typedef boost::icl::closed_interval< Span > Side;
    Side sides[Dimensions];
    Box() {}
public:
    typedef typename ImageTypes<Dimensions>::Position Position;
    static Box ZeroOrigin( typename ImageTypes<Dimensions>::Size );
    Box( Position lower, Position upper );
    Box intersection( const Box& o ) const;

    Span get_lower_edge( Direction dir ) const;
    Span get_upper_edge( Direction dir ) const;
    void set_range( Span lower, Span upper, Direction dir );
    
    Position lower_corner() const;
    Position upper_corner() const;

    Span width( Direction dimension ) const;
    typename boost::units::power_typeof_helper< Span, boost::units::static_rational<Dimensions> >::type
        volume() const;

    struct const_iterator;
    const_iterator begin() const;
    const_iterator end() const;
};

template <int Dimensions>
struct Box<Dimensions>::const_iterator 
: public boost::iterator_facade< const_iterator, Position, boost::random_access_traversal_tag >
{
public:
    const_iterator( const Box& box, const Position& corner ) : box(box), pos(corner) {}
private:
    friend class boost::iterator_core_access;
    const Box& box;
    mutable Position pos;
    bool equal( const const_iterator& o ) const { return (pos == o.pos).all(); }
    Position& dereference() const { return pos; }
    void increment() { advance(1); }
    void decrement() { advance(-1); }
    void advance( int n );
};

}
}

#endif
