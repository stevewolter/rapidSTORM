#ifndef DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_STRATEGIES_H
#define DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_STRATEGIES_H

#include "debug.h"

#include "BinnedLocalizations.h"
#include "../output/binning/binning.h"
#include "../traits/scalar.h"
#include <boost/ptr_container/ptr_array.hpp>
#include "../helpers/clone_ptr.hpp"

namespace dStorm {
namespace outputs {

namespace binning_strategy {

struct ThreeComponent
: public BinningStrategy 
{
  private:
    typedef output::binning::Scaled ScaledBin;
    typedef output::binning::Unscaled UnscaledBin;

    boost::ptr_array<ScaledBin, 2> xy;
    boost::clone_ptr<UnscaledBin> intensity;

  public:
    ThreeComponent(std::auto_ptr<ScaledBin> x, std::auto_ptr<ScaledBin> y, std::auto_ptr<UnscaledBin> intensity)
        : intensity(intensity) { xy.replace(0, x); xy.replace(1, y); }
    ThreeComponent* clone() const { return new ThreeComponent(*this); }
    ~ThreeComponent() {}

    void announce(const output::Output::Announcement& a) { 
        for (int i = 0; i < 2; ++i) xy[i].announce(a);
        intensity->announce(a);
    }
    Eigen::Matrix<quantity<camera::length>, 2, 1> get_size() {
        Eigen::Matrix<quantity<camera::length>, 2, 1> rv;
        for (int i = 0; i < 2; ++i)
            rv[i] = xy[i].get_size() * camera::pixel;
        return rv;
    }

    traits::Optics<2>::Resolutions get_resolution() {
        traits::Optics<2>::Resolutions rv;
        for (int i = 0; i < 2; ++i) {
            rv[i] = xy[i].resolution();
        }
        return rv;
    }
    void bin_points( const output::LocalizedImage& l, BinningStrategy::Result& r ) {
        static const bool stride = (BinningStrategy::Result::Flags & Eigen::RowMajorBit) ? r.stride() : 1;
        for (int i = 0; i < 2; ++i)
            xy[i].bin_points(l, &r.coeffRef(0,i), stride);
        intensity->bin_points(l, &r.coeffRef(0,2), stride);
    }
};

}

}
}

#endif
