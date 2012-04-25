#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "guf/psf/LengthUnit.h"
#include <boost/mpl/vector.hpp>
#include <nonlinfit/plane/Joint.h>
#include <nonlinfit/plane/Disjoint.hpp>

namespace dStorm {
namespace guf {

/** Prioritized lists of nonlinfit data tags for fitting functions. */
typedef boost::mpl::vector<
#if defined(USE_SPECIALIZED_FITTERS)
    nonlinfit::plane::xs_disjoint<double,PSF::LengthUnit,20>::type,
    nonlinfit::plane::xs_disjoint<double,PSF::LengthUnit,18>::type,
    nonlinfit::plane::xs_disjoint<double,PSF::LengthUnit,16>::type,
#endif
    nonlinfit::plane::xs_disjoint<double,PSF::LengthUnit,14>::type,
#if defined(USE_SPECIALIZED_FITTERS)
    nonlinfit::plane::xs_disjoint<double,PSF::LengthUnit,12>::type,
    nonlinfit::plane::xs_disjoint<double,PSF::LengthUnit,10>::type,
    nonlinfit::plane::xs_disjoint<double,PSF::LengthUnit,8>::type,
#endif
    nonlinfit::plane::xs_joint<double,PSF::LengthUnit,8>::type,
#if defined(USE_SPECIALIZED_FITTERS)
    nonlinfit::plane::xs_disjoint<float,PSF::LengthUnit,20>::type,
    nonlinfit::plane::xs_disjoint<float,PSF::LengthUnit,16>::type,
    nonlinfit::plane::xs_disjoint<float,PSF::LengthUnit,12>::type,
    nonlinfit::plane::xs_disjoint<float,PSF::LengthUnit,8>::type,
#endif
    nonlinfit::plane::xs_joint<float,PSF::LengthUnit,8>::type
> evaluation_tags;

static const int MaxWindowWidth = 24;

}
}
