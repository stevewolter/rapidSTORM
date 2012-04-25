#include "gaussian_psf/StandardFunction.h"
#include <nonlinfit/Bind.h>

typedef dStorm::gaussian_psf::StandardFunction< nonlinfit::Bind<InstantiatedExpression, dStorm::guf::Assignment>, 2>::type
    InstantiatedFunction;
