#ifndef DSTORM_OUTPUT_BINNING_BINNING_DECL_H
#define DSTORM_OUTPUT_BINNING_BINNING_DECL_H

namespace dStorm {
namespace output {
namespace binning {

enum BinningType { 
    IsUnscaled,
    Bounded,
    ScaledByResolution,
    ScaledToInterval,
    InteractivelyScaledToInterval };

struct Unscaled;
struct Scaled;
struct UserScaled;

Unscaled* new_clone( const Unscaled& o );
Scaled* new_clone( const Scaled& o );
UserScaled* new_clone( const UserScaled& o );

}
}
}

#endif
