#include <nonlinfit/plane/Disjoint.hpp>
#include <nonlinfit/plane/DisjointData.hpp>

#ifdef USE_MEASURED_PSF
#define PLANE_FUNCTION_IS_UNDEFINED
#else
#include "gaussian_psf/DisjointEvaluator.h"
#endif

typedef nonlinfit::plane::xs_disjoint< 
    InstantiatedNumberType, dStorm::gaussian_psf::LengthUnit, DisjointWidth >::type
    InstantiatedTag;
