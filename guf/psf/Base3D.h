#ifndef DSTORM_PSF_BASE3D_H
#define DSTORM_PSF_BASE3D_H

#include "BaseExpression.h"
#include <nonlinfit/append.h>
#include <nonlinfit/access_parameters.hpp>

namespace dStorm {
namespace guf {
namespace PSF {

struct Base3D 
: public BaseExpression,
  public nonlinfit::access_parameters< Base3D >
{
    using nonlinfit::access_parameters< Base3D >::operator();
    using nonlinfit::access_parameters< Base3D >::get;

  protected:
    typedef boost::mpl::vector< ZPosition, MeanZ, ZOffset<0>, ZOffset<1> > MyParameters;
    typedef nonlinfit::append< BaseExpression::Variables, MyParameters >::type
        Variables;

    double zposition, axial_mean;
    Eigen::Vector2d z_offset;

    template <typename Type> friend class nonlinfit::access_parameters;
    template <int Index> double& access( ZOffset<Index> ) { return z_offset[Index]; }
    double& access( ZPosition ) { return zposition; }
    double& access( MeanZ ) { return axial_mean; }
    using BaseExpression::access;
};

}
}
}

#endif
