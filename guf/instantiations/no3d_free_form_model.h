#include "gaussian_psf/No3D.h"
#include "gaussian_psf/free_form.h"
#include "nonlinfit/Bind.h"

typedef nonlinfit::Bind<dStorm::gaussian_psf::No3D, dStorm::gaussian_psf::FreeForm> InstantiatedExpression;
