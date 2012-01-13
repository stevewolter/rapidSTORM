#include "guf/psf/StandardFunction.h"
#include <nonlinfit/Bind.h>

typedef dStorm::guf::PSF::StandardFunction< nonlinfit::Bind<InstantiatedExpression, dStorm::guf::Assignment>, 1>::type
    InstantiatedFunction;
