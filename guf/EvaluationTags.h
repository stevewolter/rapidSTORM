#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <boost/mpl/vector.hpp>
#include <nonlinfit/plane/Joint.h>
#include <nonlinfit/plane/Disjoint.hpp>

namespace dStorm {
namespace guf {

/** Prioritized lists of nonlinfit data tags for fitting functions. */
typedef boost::mpl::vector<
#if defined(USE_SPECIALIZED_FITTERS)
    nonlinfit::plane::xs_disjoint<double,20>::type,
    nonlinfit::plane::xs_disjoint<double,18>::type,
    nonlinfit::plane::xs_disjoint<double,16>::type,
#endif
    nonlinfit::plane::xs_disjoint<double,14>::type,
#if defined(USE_SPECIALIZED_FITTERS)
    nonlinfit::plane::xs_disjoint<double,12>::type,
    nonlinfit::plane::xs_disjoint<double,10>::type,
    nonlinfit::plane::xs_disjoint<double,8>::type,
#endif
    nonlinfit::plane::xs_joint<double,8>::type,
#if defined(USE_SPECIALIZED_FITTERS)
    nonlinfit::plane::xs_disjoint<float,20>::type,
    nonlinfit::plane::xs_disjoint<float,16>::type,
    nonlinfit::plane::xs_disjoint<float,12>::type,
    nonlinfit::plane::xs_disjoint<float,8>::type,
#endif
    nonlinfit::plane::xs_joint<float,8>::type
> evaluation_tags;

static const int MaxWindowWidth = 24;

}
}
