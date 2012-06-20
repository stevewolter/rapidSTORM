#ifndef DISTANCE_HISTOGRAM_HISTOGRAM_H
#define DISTANCE_HISTOGRAM_HISTOGRAM_H

#include <Eigen/StdVector>
#include <boost/array.hpp>
#include <dStorm/localization/Traits.h>
#include <dStorm/Localization.h>
#include <boost/units/systems/si/length.hpp>
#include <boost/icl/continuous_interval.hpp>
#include <dStorm/Image.h>

namespace distance_histogram {

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

    typedef boost::array< Bins::Position, Dim_power_3_half > ForwardScan;
    ForwardScan forward_scan;
    Bins bins;
    Length bin_size, max_distance_sq;

    inline void count_distance( const Point& a, const Point& b );
    void autocorrelate( const Points& a );
    void crosscorrelate( const Points& a, const Points& b );

  public:
    std::vector<int> counts;

    Histogram( boost::array< Length, Dim > max_value, Length max_dist );
    ~Histogram();

    void push_back( const Point& input );
    void compute();
    void clear();
};

}

#endif
