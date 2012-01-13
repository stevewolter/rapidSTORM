#include "guf/psf/DisjointEvaluator.h"
#include <nonlinfit/plane/Disjoint.hpp>
#include <nonlinfit/plane/DisjointData.hpp>

typedef nonlinfit::plane::xs_disjoint< 
    InstantiatedNumberType, dStorm::guf::PSF::LengthUnit, DisjointWidth >::type
    InstantiatedTag;
