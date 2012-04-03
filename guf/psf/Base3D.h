#ifndef DSTORM_PSF_BASE3D_H
#define DSTORM_PSF_BASE3D_H

#include "BaseExpression.h"
#include <nonlinfit/access_parameters.hpp>
#include <nonlinfit/append.h>

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
    typedef boost::mpl::vector< MeanZ > MyParameters;
    typedef nonlinfit::append< BaseExpression::Variables, MyParameters >::type
        Variables;

    double axial_mean;

    template <typename Num, typename Expression> friend class Parameters;
    double& access( MeanZ ) { return axial_mean; }

    template <typename Type> friend class nonlinfit::access_parameters;
    using BaseExpression::access;
};

}
}
}

#endif
