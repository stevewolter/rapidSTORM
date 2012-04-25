#include "gaussian_psf/DisjointEvaluator.h"
#include <nonlinfit/plane/Disjoint.hpp>
#include <nonlinfit/plane/DisjointData.hpp>

typedef nonlinfit::plane::xs_disjoint< 
    InstantiatedNumberType, dStorm::gaussian_psf::LengthUnit, DisjointWidth >::type
    InstantiatedTag;
