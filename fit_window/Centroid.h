#ifndef NONLINFIT_IMAGE_CENTROID_H
#define NONLINFIT_IMAGE_CENTROID_H

#include <boost/units/quantity.hpp>
#include <boost/optional/optional.hpp>
#include <boost/units/Eigen/Array>
#include <dStorm/units/microlength.h>

namespace dStorm {
namespace fit_window {

/** This class computes a weighted centroid position of its input
 *  values. The weights may be negative. */
struct Centroid {
    typedef Eigen::Array< double, 2, 1, Eigen::DontAlign> Spot;
  private:

    boost::optional<Spot> min, max;
    Spot weighted_sum[2];
    double total_weight[2];

    void init() {
        for (int i = 0; i < 2; ++i) {
            weighted_sum[i] = Spot::Constant( 0 );
            total_weight[i] = 0.0;
            assert( weighted_sum[i].x() <= 1E40 );
        }
    }
  public:
    Centroid() { init(); }
    Centroid( Spot min, Spot max ) : min(min), max(max) { init(); }
    void add( const Spot& s, float weight ) {
        int i = ( weight > 0 ) ? 0 : 1;
        weight = fabs(weight);
        for (int j = 0; j < weighted_sum[i].rows(); ++j)
            weighted_sum[i][j] += s[j] * double(weight);
        total_weight[i] += weight;
        assert( weighted_sum[i].x() <= 1E40 );
    }
    Centroid& operator+=( const Centroid& o ); 

    Spot current_position() const;
};

}
}

#endif
