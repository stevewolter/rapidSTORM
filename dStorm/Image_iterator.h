#ifndef DSTORM_IMAGE_ITERATOR_H
#define DSTORM_IMAGE_ITERATOR_H

#include "BaseImage.h"
#include <boost/iterator/iterator_facade.hpp>
#include <dStorm/unit_matrix_operators.h>

namespace dStorm {

template <typename PixelType, int Dimensions>
template <typename IteratedType>
class BaseImage<PixelType,Dimensions>::_iterator
: public boost::iterator_facade<
    _iterator<IteratedType>, IteratedType, boost::random_access_traversal_tag>
{
  public:
    typedef Eigen::Matrix<int,Dimensions-1,1,Eigen::DontAlign> Sizes;
    typedef Eigen::Matrix<int,Dimensions,1,Eigen::DontAlign> Position;
  private:
    friend class boost::iterator_core_access;
    IteratedType *p;
    Position pos;
    Sizes step;
    friend class BaseImage<PixelType,Dimensions>;
    _iterator(IteratedType *p, Sizes sz)
        : p(p), pos(Position::Zero()), step(sz) {}

    IteratedType& dereference() const { return *p; }
    bool equal(const iterator& o) const 
        { return p == o.p; }
    template <class OtherIteratedType>
    bool equal(const _iterator<OtherIteratedType>& o) const 
        { return p == o.p; }
    void increment() { 
        ++p; ++pos[0];
        for ( int i = 0; i < pos.rows()-1; i++ )
            if ( pos[i] >= step[i] )
                { pos[i] = 0; ++pos[i+1]; }
            else break;
    }
    void decrement() { 
        --p; --x;
        for ( int i = 0; i < pos.rows()-1; i++ )
            if ( step[i] < 0 )
                { pos[i] = step[i]-1; ++pos[i+1]; }
            else break;
    }
    void advance(int n) { 
        p += n; 
        x += n;
        for ( int i = 0; i < pos.rows()-1; ) {
            if ( step[i] < 0 )
                { pos[i] += step[i]; ++pos[i+1]; }
            else if ( pos[i] >= step[i] )
                { pos[i] -= step[i]; --pos[i+1]; }
            else 
                ++i;
        }
    }
    void distance(const const_iterator& o) 
        { return o.p - p; }

    template <class> friend class _iterator;
  public:
    _iterator() : p(NULL) {}
    template <class OtherIteratedType>
        _iterator(const _iterator<OtherIteratedType>& o) 
        : p(o.p), pos(o.pos), step(o.step) {}

    const Position& position() { return pos; }
    int x() { return pos.x(); }
    int y() { return pos.y(); }
};

template <typename PixelType, int Dimensions>
typename BaseImage<PixelType,Dimensions>::iterator
BaseImage<PixelType,Dimensions>::begin() 
{
    typename iterator::Sizes s;
    for (int i = 0; i < Dimensions-1; i++)
        s[i] = sz[i] / cs_units::camera::pixel;
    return _iterator<PixelType>(img.get(), s);
}

template <typename PixelType, int Dimensions>
typename BaseImage<PixelType,Dimensions>::iterator
BaseImage<PixelType,Dimensions>::end() 
{
    return iterator(img.get()+size_t(sz.x().value())*sz.y().value(), iterator::Sizes::Ones());
}

template <typename PixelType, int Dimensions>
typename BaseImage<PixelType,Dimensions>::const_iterator
BaseImage<PixelType,Dimensions>::begin()  const
{
    typename iterator::Sizes s;
    for (int i = 0; i < Dimensions-1; i++)
        s[i] = sz[i] / cs_units::camera::pixel;
    return const_iterator(img.get(), s);
}

template <typename PixelType, int Dimensions>
typename BaseImage<PixelType,Dimensions>::const_iterator
BaseImage<PixelType,Dimensions>::end()  const
{
    return const_iterator(img.get()+size_t(sz.x().value())*sz.y().value(), const_iterator::Sizes::Zero());
}

}

#endif
