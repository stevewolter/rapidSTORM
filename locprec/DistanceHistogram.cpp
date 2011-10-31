//#include "fast_int_sqrt.h"
#include "DistanceHistogram.h"
#include <Eigen/Array>
#include <Eigen/Core>
#include <limits>
#include <boost/units/cmath.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/wavenumber.hpp>
#include <boost/units/io.hpp>

namespace distance_histogram {


template <typename Type>
const Type square( const Type x ) { return x*x; }

template <typename Type>
const Type fastabs( const Type x )
{
    return (x ^ (x >> (sizeof(Type)*8-1)) - (x >> (sizeof(Type)*8-1)));
}

template <int ShiftDivisor>
const unsigned int roundDiv(unsigned int in) {
    return ( (in + (1 << (ShiftDivisor-1))) >> ShiftDivisor );
}

void Histogram::init_vec( int bin_count )
{
    counts.resize( bin_count, 0 );

    for (int i = 0; i < bin_count; i++) {
        counts[i] = 0;
    }
}

void Histogram::shift_down( const Block& from, Block& to, const int amount )
{
    to.block( to.rows()-amount, 0, amount, Dim )
        = from.block( 0, 0, amount, Dim );
}

void Histogram::fill_start( const Block& from, Block& to, const int amount )
{
    to.block( 0, 0, amount, Dim ) 
            = from.block( from.rows() - amount, 0, amount, Dim );
}

void Histogram::shift_blocks( const Blocks& blocks, Blocks& shifted_blocks,
                   const int shift )
{
    const int rest = VecsPerBlock - shift;

    for (unsigned int i = 0; i < blocks.size(); i++) {
        shift_down( blocks[i], shifted_blocks[i], rest );
        fill_start( blocks[i], shifted_blocks[i], shift );
    }
}

void Histogram::push_back( const boost::array< Length, Dim >& input ) {
    for (int j = 0; j < Dim; j++)
        blocks.back()(offset, j) = input[j];
    offset += 1;
    if ( offset >= VecsPerBlock ) {
        blocks.push_back( Block::Zero() );
        offset = 0;
    }
}

Histogram::Histogram(
        boost::array< Length, 2 > max_value
) : offset(0)
{
    blocks.push_back( Block::Zero() );
    min_dimension_size = std::numeric_limits<float>::max();
    for (int i = 0; i < 2; i++) {
        min_dimension_size = std::min(min_dimension_size, 
                                      max_value[i]/2);

        for (int j = 0; j < VecsPerBlock; j++)
            boundaries( j, i ) = max_value[i];
    }
    init_vec( int(ceil(min_dimension_size)) );
}

void Histogram::compute() {
    if ( offset == 0 ) blocks.pop_back();
    unsigned int computed_blocks = 0;

    for ( int shift = 0; shift < VecsPerBlock; shift++ ) {
        Blocks shifted_blocks( blocks.size() );
        if ( shift == 0 )
            shifted_blocks = blocks;
        else
            shift_blocks( blocks, shifted_blocks, shift );

        for (unsigned int i = 0; i < blocks.size(); i++) {
            const Block& b = blocks[i];
            /* Inner-block distances */
            for (int j = 0; j < shift; j++) {
                float dist_sq = 0;
                for (int l = 0; l < 2; l++) {
                    float diff = fabs( b(j,l) - b(shift,l) );
                    dist_sq += 
                        pow<2>( std::min( boundaries(j,l) - diff, diff ) );
                }
                int dist = round(sqrt(dist_sq));
                if ( dist < min_dimension_size ) ++counts[dist];
            }

            /* Distances with other blocks */
            for (unsigned int j = i+1; j < blocks.size(); j++) {
                Block diffs = (b-shifted_blocks[j]).cwise().abs();

                Eigen::Matrix<float,VecsPerBlock,1> dists
                    = diffs.cwise().min(boundaries - diffs)
                      .rowwise().squaredNorm()
                      .cwise().sqrt();
                for (int k = 0; k < VecsPerBlock; k++) {
                    assert(dists[k] >= 0);
                    int dist = round(dists[k]);
                    if ( dist < min_dimension_size ) ++counts[ dist ];
                }
                computed_blocks++;
            }
        }
    }
}

}
