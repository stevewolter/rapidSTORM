#ifndef DSTORM_PSF_ZHUANG_H
#define DSTORM_PSF_ZHUANG_H

#include "Base3D.h"
#include <nonlinfit/access_parameters.hpp>
#include <nonlinfit/DerivationSummand.h>

namespace dStorm {
namespace guf {
namespace PSF {

class Polynomial3D
: public Base3D,
  public nonlinfit::access_parameters<Polynomial3D>
{
  public:
    static const int Order = 4;
  private:
    typedef boost::mpl::vector< 
        DeltaSigma<0,1>, DeltaSigma<0,2>, DeltaSigma<0,3>, DeltaSigma<0,4>, 
        DeltaSigma<1,1>, DeltaSigma<1,2>, DeltaSigma<1,3>, DeltaSigma<1,4> > ExtraParameters;
    template <typename Type> friend class nonlinfit::access_parameters;
    template <class Num, typename Expression> friend class Parameters;
    template <class Num, typename Expression, int Size> friend class JointEvaluator;
    template <class Num, typename Expression, int Size> friend class DisjointEvaluator;
    Eigen::Array< double, 2, 5 > delta_sigma;
    template <int Index, int Term> double& access( DeltaSigma<Index,Term> ) { return delta_sigma(Index,Term); }
    using Base3D::access;

  public:
    typedef nonlinfit::append< Base3D::Variables, ExtraParameters >::type
        Variables;

    Polynomial3D& copy( const BaseExpression& f ) { return *this = dynamic_cast<const Polynomial3D&>(f); }

    Eigen::Matrix< quantity<MeanZ::Unit>, 2, 1 > get_sigma() const;

    bool form_parameters_are_sane() const;
    boost::units::quantity< Micrometers > get_delta_sigma
        ( int dimension, int term ) const;

    using nonlinfit::access_parameters<Polynomial3D>::operator();
    using nonlinfit::access_parameters<Polynomial3D>::get;
};

}
}
}

#endif
