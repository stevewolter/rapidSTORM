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
    typedef boost::mpl::vector< MeanZ, ZPosition<0>, ZPosition<1> > MyParameters;
    typedef nonlinfit::append< BaseExpression::Variables, MyParameters >::type
        Variables;

    double axial_mean;
    Eigen::Vector2d zposition;

    template <typename Num, typename Expression> friend class Parameters;

    template <typename Type> friend class nonlinfit::access_parameters;
    template <int Index> double& access( ZPosition<Index> ) { return zposition[Index]; }
    double& access( MeanZ ) { return axial_mean; }
    using BaseExpression::access;
};

}
}
}

#endif
