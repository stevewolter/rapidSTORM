#include "guf/psf/JointEvaluator.h"
#include <nonlinfit/plane/Joint.h>
#include <nonlinfit/plane/JointData.h>

typedef nonlinfit::plane::xs_joint< 
    InstantiatedNumberType, dStorm::guf::PSF::LengthUnit, 8 >::type InstantiatedTag;
