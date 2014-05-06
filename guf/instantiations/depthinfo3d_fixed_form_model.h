#include "gaussian_psf/DepthInfo3D.h"
#include "gaussian_psf/fixed_form.h"
#include "nonlinfit/Bind.h"

typedef nonlinfit::Bind<dStorm::gaussian_psf::DepthInfo3D, dStorm::gaussian_psf::FixedForm> InstantiatedExpression;
