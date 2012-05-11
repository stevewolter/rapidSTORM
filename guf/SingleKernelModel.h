#ifndef DSTORM_FITTER_SINGLE_KERNEL_MODEL_H
#define DSTORM_FITTER_SINGLE_KERNEL_MODEL_H

#include "gaussian_psf/fwd.h"
#include "gaussian_psf/parameters.h"

#include <Eigen/Core>
#include <boost/mpl/vector.hpp>
#include <boost/units/quantity.hpp>
#include <nonlinfit/Lambda.h>
#include <boost/units/Eigen/Core>
#include <nonlinfit/access_parameters.hpp>
#include "gaussian_psf/LengthUnit.h"

namespace dStorm {
namespace guf {

using namespace nonlinfit;
using namespace boost::units;

struct SingleKernelModel
{
    virtual ~SingleKernelModel() {}
    virtual Eigen::Matrix< quantity<LengthUnit>, 2, 1 > get_sigma() const = 0;
    virtual SingleKernelModel& copy( const SingleKernelModel& ) = 0;
    virtual quantity<si::length> get_fluorophore_position(int) const =0;
    virtual void set_fluorophore_position(int, quantity<si::length>) const =0;
    virtual double intensity() const =0;
    virtual quantity<si::dimensionless> get_amplitude() const =0;
    virtual void set_amplitude(quantity<si::dimensionless>) =0;
    virtual bool has_z_position() const = 0;
};

}
}

#endif

