#include "gaussian_psf/DisjointEvaluator.h"
#include <nonlinfit/plane/Disjoint.hpp>
#include <nonlinfit/plane/DisjointData.h>

typedef nonlinfit::plane::xs_disjoint< 
    InstantiatedNumberType, DisjointWidth >::type
    InstantiatedTag;
