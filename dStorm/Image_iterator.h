#ifndef DSTORM_IMAGE_ITERATOR_H
#define DSTORM_IMAGE_ITERATOR_H

#include "Image.h"
#include <boost/iterator/iterator_facade.hpp>
#include <boost/units/Eigen/Array>

namespace dStorm {

template <typename PixelType, int Dimensions>
template <typename IteratedType>
class Image<PixelType,Dimensions>::_iterator
: public boost::iterator_facade<
    _iterator<IteratedType>, IteratedType, boost::random_access_traversal_tag>
{
  private:
    friend class boost::iterator_core_access;
    IteratedType *p;
    Position pos;
    const Image* im;
    friend class Image<PixelType,Dimensions>;
    _iterator(const Image& i)
        : p( const_cast<IteratedType*>(i.ptr()) ), pos( Position::Constant( 0 * camera::pixel ) ), im(&i) {}
    _iterator(const Image& i, const Position& pos)
        : p( NULL ), pos(pos), im(&i) { _seek(); }

    IteratedType& dereference() const { return *p; }
    bool equal(const iterator& o) const 
        { return p == o.p; }
    template <class OtherIteratedType>
    bool equal(const _iterator<OtherIteratedType>& o) const 
        { return p == o.p; }
    void increment() { 
        ++p; pos[0] += 1 * camera::pixel;
        for ( int i = 1; i < Dimensions; i++ )
            if ( pos[i-1] >= im->sz[i-1] ) { 
                pos[i-1] = 0; 
                p -= im->sz[i-1].value() * im->offsets[i-1]; 
                pos[i] += 1 * camera::pixel;
                p += im->offsets[i]; 
            } else 
                break;
    }
    void decrement() { 
        --p; pos[0] -= 1 * camera::pixel;
        for ( int i = 1; i < Dimensions && pos[i-1].value() < 0; ++i ) { 
            pos[i-1] += im->sz[i-1];
            pos[i] -= 1 * camera::pixel;
            p += im->sz[i-1].value() * im->offsets[i-1] - im->offsets[i];
        }
    }
    void advance(int n) { 
        for ( int i = 0; i < Dimensions; ++i) {
            pos[i] += n * camera::pixel; 
            if ( pos[i] > im->sz[i] ) {
                n = pos[i] / im->sz[i];
                pos[i] = (pos[i].value() % im->sz[i].value()) * camera::pixel;
            } else if ( pos[i] < 0 * camera::pixel ) {
                n = pos[i] / im->sz[i] - 1;
                pos[i] = (pos[i].value() % im->sz[i].value()) * camera::pixel + im->sz[i];
            } else 
                break;
        }
        _seek();
    }
    int distance_to(const const_iterator& o) const { 
        int rv = 0;
        for (int i = Dimensions-1; i >= 0; --i) {
            rv *= im->sz[i].value();
            rv += (o.pos[i] - pos[i]).value();
        }
        return rv;
    }

    void _seek() {
        p = const_cast<IteratedType*>(im->ptr());
        for (int i = 0; i < Dimensions; ++i)
            p += pos[i].value() * im->offsets[i];
    }

    template <class> friend class _iterator;
  public:
    _iterator() : p(NULL), im(NULL) {}
    _iterator(const _iterator& o)
        : p(o.p), pos(o.pos), im(o.im) {}
    template <class OtherIteratedType>
        _iterator(const _iterator<OtherIteratedType>& o) 
        : p(o.p), pos(o.pos), im(o.im) {}

    const Position& position() const { return pos; }
    quantity<camera::length,int> x() { return pos.x(); }
    quantity<camera::length,int> y() { return pos.y(); }
    quantity<camera::length,int> z() { return pos.z(); }
    _iterator& seek(const Position& npos) { pos = npos; _seek(); }
};

template <typename PixelType, int Dimensions>
typename Image<PixelType,Dimensions>::iterator
Image<PixelType,Dimensions>::begin() 
{
    return _iterator<PixelType>(*this);
}

template <typename PixelType, int Dimensions>
typename Image<PixelType,Dimensions>::iterator
Image<PixelType,Dimensions>::end() 
{
    Position e = Position::Constant( 0 * camera::pixel );
    e[Dimensions-1] = sz[Dimensions-1];
    return iterator(*this, e);
}

template <typename PixelType, int Dimensions>
typename Image<PixelType,Dimensions>::const_iterator
Image<PixelType,Dimensions>::begin()  const
{
    return const_iterator(*this);
}

template <typename PixelType, int Dimensions>
typename Image<PixelType,Dimensions>::const_iterator
Image<PixelType,Dimensions>::end()  const
{
    Position e = Position::Constant( 0 * camera::pixel );
    e[Dimensions-1] = sz[Dimensions-1];
    return const_iterator(*this, e);
}

}

#endif
