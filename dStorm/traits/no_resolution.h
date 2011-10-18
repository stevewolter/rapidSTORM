#ifndef DSTORM_TRAITS_NORESOLUTION_H
#define DSTORM_TRAITS_NORESOLUTION_H

#include "base.h"

namespace dStorm {
namespace traits {

template <typename Base>
struct NoResolution {
    static const bool has_resolution = false;

  private:
    typedef boost::optional<boost::units::quantity< si::dimensionless, int> > resolution_scalar;
  public:
    typedef typename value<Base>::MoS::template value<resolution_scalar>::type
        ResolutionType; 
    typedef resolution_scalar user_resolution_type;

    ResolutionType resolution() const { assert(false); throw std::logic_error("Reading from nonexistent resolution"); }
    ResolutionType& resolution() { assert(false); throw std::logic_error("Writing to nonexistent resolution"); }

    resolution_scalar from_user_resolution_unit( const user_resolution_type& u ) { return u; }
    ResolutionType to_user_resolution_unit( const resolution_scalar& u ) { return u; }
};

}
}

#endif
