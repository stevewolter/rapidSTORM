#include "DistanceHistogram.h"
#include <Eigen/Core>
#include <limits>
#include <boost/units/cmath.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/wavenumber.hpp>
#include <boost/units/io.hpp>
#include <boost/units/Eigen/Array>
#include <dStorm/image/iterator.h>
#include <dStorm/image/constructors.h>
#include <boost/bind/bind.hpp>

namespace distance_histogram {

static const int forward_scan_elements[][2] = { {1,0}, {-1,1}, {0,1}, {1,1} };

Histogram::Histogram(
        boost::array< Length, 2 > max_value, Length max_dist, bool periodic
) : bin_size( max_dist ), periodic_boundary(periodic)
{
    Bins::Size size;
    for (int i = 0; i < Dim; ++i) {
        bin_size = std::min( bin_size, max_value[i] * ((periodic_boundary) ? 0.5f : 1.0f) );
        area_size[i] = max_value[i];
        /* Add two extra border cells for periodic boundary conditions */
        size[i] = ( ceil( max_value[i] / bin_size ) + 2 ) * camera::pixel;
    }
    max_distance_sq = bin_size * bin_size;
    bins = Bins( size );

    BOOST_STATIC_ASSERT( Dim == 2 );
    for (int i = 0; i < Dim_power_3_half; ++i)
        for (int j = 0; j < Dim; ++j)
            this->forward_scan[i][j] = forward_scan_elements[i][j] * camera::pixel;
}

Histogram::~Histogram() {}

template <>
inline void Histogram::insert_point<Histogram::Dim>( const Point& input ) {
    Eigen::Vector2i bin = floor(input / bin_size).cast<int>();
    assert( bin.x() >= -1 && bin.y() >= -1 && bin.x() <= bins.width_in_pixels() - 2 && bin.y() <= bins.height_in_pixels() - 2 );
    bins( bin.x() + 1, bin.y() + 1 ).push_back( input );
}

template <int d>
inline void Histogram::insert_point( const Point& input ) {
    insert_point<d+1>( input );
    if ( input[d] > area_size[d] - bin_size )
        insert_point<d+1>( input - area_size[d] * Point::Unit(d) );
    if ( input[d] < bin_size )
        insert_point<d+1>( input + area_size[d] * Point::Unit(d) );
}

void Histogram::push_back( const Point& input ) {
    if ( periodic_boundary )
        insert_point<0>(input);
    else
        insert_point<Dim>(input);
}

void Histogram::compute() {
    counts.resize( ceil( bin_size ) + 1 );
    std::fill( counts.begin(), counts.end(), 0 );

    Bins::Position::Scalar px = 1 * camera::pixel;
    Bins::Position low_border = Bins::Position::Constant( 0 * camera::pixel ),
                   high_border = bins.sizes().array() - 1 * camera::pixel;
    int xh = bins.width_in_pixels()-1, yh = bins.height_in_pixels();

    for ( Bins::iterator i = bins.begin(); i != bins.end(); ++i ) {
        if ( (i.position() == low_border).any() ||
             (i.position() == high_border).any() )
            continue;
        autocorrelate( *i );
        for (ForwardScan::iterator j = forward_scan.begin(); j != forward_scan.end(); ++j) {
            crosscorrelate( *i, bins( i.position() + *j ) );
        }
    }

    if ( periodic_boundary )
        counts.pop_back();
}

inline void Histogram::count_distance( const Point& a, const Point& b ) {
    float dist_sq = (a-b).squaredNorm();
    if ( dist_sq < max_distance_sq && (a.array() < area_size.array()).all() )
        ++counts[ round(sqrt(dist_sq)) ];
}

void Histogram::autocorrelate( const Points& a ) {
    for (Points::const_iterator i = a.begin(); i != a.end(); ++i)
        for (Points::const_iterator j = i+1; j != a.end(); ++j)
            count_distance( *i, *j );
}

void Histogram::crosscorrelate( const Points& a, const Points& b ) {
    for (Points::const_iterator i = a.begin(); i != a.end(); ++i)
        for (Points::const_iterator j = b.begin(); j != b.end(); ++j)
            count_distance( *i, *j );
}

void Histogram::clear() {
    for ( Bins::iterator i = bins.begin(); i != bins.end(); ++i )
        i->clear();
}

}
