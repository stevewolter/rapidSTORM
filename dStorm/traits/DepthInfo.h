#ifndef DSTORM_TRAITS_DEPTHINFO_H
#define DSTORM_TRAITS_DEPTHINFO_H

#include <dStorm/threed_info/Polynomial3D.h>

namespace dStorm {
namespace traits {

struct No3D {};

class Spline3D {
    boost::shared_ptr< const threed_info::Spline > spline;
  public:
    Spline3D ( boost::shared_ptr< const threed_info::Spline > s ) : spline(s) {}
    const boost::shared_ptr< const threed_info::Spline > get_spline() const
        { return spline; }
    boost::units::quantity< boost::units::si::length > equifocal_plane() const;
};

typedef boost::variant< traits::Polynomial3D, traits::No3D, traits::Spline3D > DepthInfo;

}
}

#endif
