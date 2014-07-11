#ifndef DISTANCE_HISTOGRAM_HISTOGRAM_H
#define DISTANCE_HISTOGRAM_HISTOGRAM_H

#include <Eigen/StdVector>
#include <array>
#include <boost/units/systems/si/length.hpp>
#include <boost/icl/continuous_interval.hpp>
#include "localization/Traits.h"
#include "Localization.h"
#include "image/Image.h"

namespace dStorm {
namespace ripley_k {

using namespace boost::units;
using namespace boost::icl;

struct Histogram {
    static const int Dim = 2, Dim_power_3_half = 4;
  public:
    typedef float Length;
    
  private:
    typedef Eigen::Matrix<Length,Dim,1> Point;
    typedef std::vector<Point> Points;
    typedef dStorm::Image< Points, Dim > Bins;

    typedef std::array< Bins::Position, Dim_power_3_half > ForwardScan;
    ForwardScan forward_scan;
    Bins bins;
    Eigen::Matrix<Length,Dim,1> area_size;
    Length bin_size, max_distance_sq;
    bool periodic_boundary;

    template <int ReplicationDim> inline void insert_point( const Point& );
    void copy_bin( Bins::Position from, Bins::Position to );
    inline void count_distance( const Point& a, const Point& b );
    void autocorrelate( const Points& a );
    void crosscorrelate( const Points& a, const Points& b );

  public:
    std::vector<int> counts;

    Histogram( std::array< Length, Dim > max_value, Length max_dist, bool periodic_boundary );
    ~Histogram();

    void push_back( const Point& input );
    void compute();
    void clear();
};

}
}

#endif
