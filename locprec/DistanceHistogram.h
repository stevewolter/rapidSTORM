#ifndef DISTANCE_HISTOGRAM_HISTOGRAM_H
#define DISTANCE_HISTOGRAM_HISTOGRAM_H

#include <Eigen/StdVector>
#include <boost/array.hpp>
#include <dStorm/localization/Traits.h>
#include <dStorm/Localization.h>
#include <boost/units/systems/si/length.hpp>
#include <boost/icl/continuous_interval.hpp>

namespace distance_histogram {

using namespace boost::units;
using namespace boost::icl;

struct Histogram {
    static const int Dim = 2;
  public:
    typedef float Length;
    
  private:
    typedef std::vector< boost::array< Length, Dim > > Input;
    /** This should be a multiple of 4 to ensure alignability. */
    static const int VecsPerBlock = 4;
    static const int BlockSize = VecsPerBlock*Dim;

    typedef Eigen::Matrix<float, VecsPerBlock, Dim, Eigen::ColMajor> Block;
    typedef std::vector<Block> Blocks;

    void init_vec( int bin_count );
    static void sort_into_blocks( const Input&, Blocks& blocks );
    inline void shift_down( const Block& from, Block& to, const int amount );
    inline void fill_start( const Block& from, Block& to, const int amount );
    void shift_blocks( const Blocks&, Blocks&, const int );

    Block boundaries;
    Blocks blocks;
    int offset;
    float min_dimension_size;

  public:
    /** Center of histogram bin */
    std::vector< Length > bin_positions;
    std::vector<int> counts;

    Histogram( boost::array< Length, Dim > max_value );
    void push_back( const boost::array< Length, Dim >& input );
    void compute();
    void clear() { blocks.clear(); blocks.push_back( Block::Zero() ); offset = 0; }
};

}

#endif
