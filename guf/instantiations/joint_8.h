#include "gaussian_psf/JointEvaluator.h"
#include <nonlinfit/plane/Joint.h>
#include <nonlinfit/plane/JointData.h>

typedef nonlinfit::plane::xs_joint< 
    InstantiatedNumberType, dStorm::gaussian_psf::LengthUnit, 8 >::type InstantiatedTag;
