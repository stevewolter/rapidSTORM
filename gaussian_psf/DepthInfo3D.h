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
#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm {
namespace gaussian_psf {

class DepthInfo3D
: public Base3D
{
    typedef boost::shared_ptr<const threed_info::DepthInfo> DepthInfo;
    DepthInfo spline[2];
    template <class Num, typename Expression> friend class Parameters;
  public:
    typedef Base3D::Variables Variables;
    DepthInfo3D& copy( const BaseExpression& f ) { return *this = dynamic_cast<const DepthInfo3D&>(f); }
    void set_spline( DepthInfo sx, DepthInfo sy ) ;

    // Returns the PSF standard deviation in micrometers.
    Eigen::Vector2d get_sigma() const;

    bool form_parameters_are_sane() const;
    const threed_info::DepthInfo& get_spline(Direction dir) const
        { return *spline[dir]; }
};

template <typename Num>
class Parameters< Num, DepthInfo3D >
: public BaseParameters<Num>
{
    Eigen::Array<Num,2,1> compute_sigma_();
    void compute_prefactors_();

  protected:
    const DepthInfo3D* expr;
    Eigen::Array<Num,2,1> z_deriv_prefactor;
  public:
    Parameters() {}
    Parameters( const DepthInfo3D& expr ) : BaseParameters<Num>(expr), expr(&expr) {}
};

}
}

#endif
