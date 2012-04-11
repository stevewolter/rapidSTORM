#ifndef DSTORM_TRAITS_DEPTHINFO_H
#define DSTORM_TRAITS_DEPTHINFO_H

#include "types.h"
#include <dStorm/Direction.h>

namespace dStorm {
namespace threed_info {

class DepthInfo {
    virtual Sigma get_sigma_( Direction dir, ZPosition z ) const = 0;
    virtual SigmaDerivative get_sigma_deriv_( Direction dir, ZPosition z ) const = 0;
    virtual ZRange z_range_() const = 0;
    virtual ZPosition equifocal_plane_() const = 0;
    virtual std::ostream& print_( std::ostream& ) const = 0;
    virtual std::string config_name_() const = 0;
    virtual bool provides_3d_info_() const = 0;
public:
    virtual ~DepthInfo() {}
    Sigma get_sigma( Direction dir, ZPosition z ) const
        { return get_sigma_(dir,z); }
    Sigma get_sigma_diff( ZPosition z ) const
        { return get_sigma(Direction_X,z) - get_sigma(Direction_Y,z); }
    SigmaDerivative get_sigma_deriv( Direction dir, ZPosition z ) const
        { return get_sigma_deriv_(dir,z); }
    ZRange z_range() const { return z_range_(); }
    ZPosition equifocal_plane() const { return equifocal_plane_(); }
    std::string config_name() const { return config_name_(); }
    friend std::ostream& operator<<( std::ostream&, const DepthInfo& );

    bool provides_3d_info() const { return provides_3d_info_(); }
};
inline std::ostream& operator<<( std::ostream& o, const DepthInfo& c ) 
    { return c.print_(o); }

}
}

#endif
