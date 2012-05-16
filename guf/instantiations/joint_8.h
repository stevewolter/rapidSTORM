#include <nonlinfit/plane/Joint.h>
#include <nonlinfit/plane/JointData.h>

#ifdef USE_MEASURED_PSF
#include "measured_psf/Evaluator.h"
#else
#include "gaussian_psf/JointEvaluator.h"
#endif

typedef nonlinfit::plane::xs_joint< 
    InstantiatedNumberType, dStorm::gaussian_psf::LengthUnit, 8 >::type InstantiatedTag;
