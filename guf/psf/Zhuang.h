#ifndef DSTORM_PSF_ZHUANG_H
#define DSTORM_PSF_ZHUANG_H

#include "Base3D.h"
#include <nonlinfit/access_parameters.hpp>
#include <nonlinfit/DerivationSummand.h>

namespace dStorm {
namespace guf {
namespace PSF {

class Zhuang
: public Base3D,
  public nonlinfit::access_parameters<Zhuang>
{
    typedef boost::mpl::vector< DeltaSigma<0>, DeltaSigma<1> > ExtraParameters;
    template <typename Type> friend class nonlinfit::access_parameters;
    template <class Num, typename Expression> friend class BaseEvaluator;
    template <class Num, typename Expression, int Size> friend class JointEvaluator;
    template <class Num, typename Expression, int Size> friend class DisjointEvaluator;
    Eigen::Vector2d delta_sigma;
    template <int Index> double& access( DeltaSigma<Index> ) { return delta_sigma[Index]; }
    using Base3D::access;

  public:
    typedef nonlinfit::append< Base3D::Variables, ExtraParameters >::type
        Variables;

    Zhuang& copy( const BaseExpression& f ) { return *this = dynamic_cast<const Zhuang&>(f); }

    Eigen::Matrix< quantity<MeanZ::Unit>, 2, 1 > get_sigma() const;

    bool form_parameters_are_sane() const;

    using nonlinfit::access_parameters<Zhuang>::operator();
    using nonlinfit::access_parameters<Zhuang>::get;

    typedef Zhuang PSF;
};

}
}
}

#endif
