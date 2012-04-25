#ifndef DSTORM_STANDARD_MODELS_H
#define DSTORM_STANDARD_MODELS_H

#include <nonlinfit/sum/Lambda.h>
#include "constant_background/model.hpp"

namespace dStorm {
namespace guf {

template <typename PSF, int Kernels> struct MultiKernelLambda;

template <typename PSF>
struct MultiKernelLambda<PSF,1>
{
    typedef nonlinfit::sum::Lambda< 
        boost::mpl::vector< PSF, constant_background::Expression > > type;
};

template <typename PSF>
struct MultiKernelLambda<PSF,2>
{
    typedef nonlinfit::sum::Lambda< 
        boost::mpl::vector< PSF, constant_background::Expression, PSF > > type;
};


}
}

#endif
