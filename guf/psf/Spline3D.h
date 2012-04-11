#ifndef DSTORM_PSF_SPLINE3D_H
#define DSTORM_PSF_SPLINE3D_H

#include "Base3D.h"
#include <nonlinfit/append.h>
#include <nonlinfit/access_parameters.hpp>
#include <nonlinfit/DerivationSummand.h>
#include <nonlinfit/DerivationSummand.h>
#include <boost/mpl/iter_fold.hpp>
#include <boost/mpl/range_c.hpp>
#include <dStorm/polynomial_3d.h>
#include <dStorm/Direction.h>
#include <dStorm/threed_info/DepthInfo.h>
#include <boost/optional/optional.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm {
namespace threed_info{ class Spline3D; }
namespace guf {
namespace PSF {

class Spline3D
: public Base3D
{
    boost::shared_ptr<const threed_info::DepthInfo> spline;
    template <class Num, typename Expression> friend class Parameters;
  public:
    typedef Base3D::Variables Variables;
    Spline3D& copy( const BaseExpression& f ) { return *this = dynamic_cast<const Spline3D&>(f); }
    void set_spline( boost::shared_ptr<const threed_info::DepthInfo> s ) { spline = s; }

    Eigen::Matrix< quantity<MeanZ::Unit>, 2, 1 > get_sigma() const;

    bool form_parameters_are_sane() const;
    const threed_info::DepthInfo& get_spline() const { return *spline; }
};

template <typename Num>
class Parameters< Num, Spline3D >
: public BaseParameters<Num>
{
    boost::optional< Eigen::Array<Num,2,1> > compute_sigma_();
    void compute_prefactors_();

  protected:
    const Spline3D* expr;
    Eigen::Array<Num,2,1> z_deriv_prefactor;
  public:
    Parameters() {}
    Parameters( const Spline3D& expr ) : BaseParameters<Num>(expr), expr(&expr) {}
};

}
}
}

#endif
