//#include "fast_int_sqrt.h"
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

namespace distance_histogram {

static const int forward_scan_elements[][2] = { {1,0}, {-1,1}, {0,1}, {1,1} };

Histogram::Histogram(
        boost::array< Length, 2 > max_value, Length max_dist
) : bin_size( max_dist ), max_distance_sq( max_dist * max_dist )
{
    Bins::Size size;
    for (int i = 0; i < Dim; ++i)
        size[i] = ceil( max_value[i] / max_dist ) * camera::pixel;
    bins = Bins( size );
    counts.resize( ceil( max_dist ) + 1, 0 );

    BOOST_STATIC_ASSERT( Dim == 2 );
    for (int i = 0; i < Dim_power_3_half; ++i)
        for (int j = 0; j < Dim; ++j)
            this->forward_scan[i][j] = forward_scan_elements[i][j] * camera::pixel;
}

Histogram::~Histogram() {}

void Histogram::push_back( const Point& input ) {
    Eigen::Vector2i bin = floor(input / bin_size).cast<int>();
    bins( bin.x(), bin.y() ).push_back( input );
}

void Histogram::compute() {
    for ( Bins::iterator i = bins.begin(); i != bins.end(); ++i ) {
        autocorrelate( *i );
        for (ForwardScan::iterator j = forward_scan.begin(); j != forward_scan.end(); ++j)
            if ( bins.contains( i.position() + *j ) )
                crosscorrelate( *i, bins( i.position() + *j ) );
    }
}

inline void Histogram::count_distance( const Point& a, const Point& b ) {
    float dist_sq = (a-b).squaredNorm();
    if ( dist_sq < max_distance_sq )
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
    std::fill( counts.begin(), counts.end(), 0 );
}

}
