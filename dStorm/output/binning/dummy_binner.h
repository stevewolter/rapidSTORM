#ifndef DSTORM_OUTPUT_BINNING_ZERO_BINNER_H
#define DSTORM_OUTPUT_BINNING_ZERO_BINNER_H

#include "binning.h"

namespace dStorm {
namespace output {
namespace binning {

struct Zero : public Scaled {
    Zero* clone() const { return new Zero(); }
    void announce(const Output::Announcement& ) {}
    traits::ImageResolution resolution() const { throw std::logic_error("Not implemented"); }
    void bin_points( const output::LocalizedImage& er, float* target, int stride ) const
        { for (unsigned i = 0; i < er.size(); ++i) { *target = 1E-15; target += stride; } }
    float bin_point( const Localization& ) const { return 0; }
    int field_number() const { throw std::logic_error("Not implemented"); }
    float get_size() const { return 1; }
    std::pair< float, float > get_minmax() const { return std::make_pair(0.0f, 1.0f); }
    double reverse_mapping( float ) const { throw std::logic_error("Not implemented"); }
};

}
}
}

#endif
