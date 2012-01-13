#ifndef DSTORM_STANDARD_MODELS_H
#define DSTORM_STANDARD_MODELS_H

#include <nonlinfit/sum/Lambda.h>
#include "guf/constant_background.hpp"

namespace dStorm {
namespace guf {
namespace PSF {

template <typename PSF, int Kernels> struct StandardFunction;

template <typename PSF>
struct StandardFunction<PSF,1>
{
    typedef nonlinfit::sum::Lambda< 
        boost::mpl::vector< PSF, constant_background::Expression > > type;
};

template <typename PSF>
struct StandardFunction<PSF,2>
{
    typedef nonlinfit::sum::Lambda< 
        boost::mpl::vector< PSF, constant_background::Expression, PSF > > type;
};


}
}
}

#endif
