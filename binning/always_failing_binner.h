#ifndef DSTORM_OUTPUT_BINNING_ALWAYS_FAILING_BINNER_H
#define DSTORM_OUTPUT_BINNING_ALWAYS_FAILING_BINNER_H

#include "binning/binning.h"

namespace dStorm {
namespace binning {

class AlwaysFailingUnscaled : public Unscaled {
    AlwaysFailingUnscaled* clone() const { return new AlwaysFailingUnscaled(); }
    void announce(const output::Output::Announcement& a) {}
    traits::ImageResolution resolution() const { throw std::logic_error("Not implemented"); }
    int bin_points( const output::LocalizedImage&, float* target, int stride ) const {
        return 0;
    }
    boost::optional<float> bin_point( const dStorm::Localization& ) const {
        return boost::optional<float>();
    }
 
  public:
    ~AlwaysFailingUnscaled() {}
};

}
}

#endif
