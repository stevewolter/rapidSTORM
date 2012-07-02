#include "binning.h"

namespace dStorm {
namespace binning {

Unscaled::~Unscaled() {}
Unscaled* new_clone( const Unscaled& o )
    { return o.clone(); }

Scaled* new_clone( const Scaled& o )
    { return o.clone(); }

UserScaled* new_clone( const UserScaled& o ) {
    UserScaled* u = o.clone(); 
    return u;
}

}
}
