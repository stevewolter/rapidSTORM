#include "guf/MultiKernelLambda.h"
#include <nonlinfit/Bind.h>

typedef dStorm::guf::MultiKernelLambda< nonlinfit::Bind<InstantiatedExpression, dStorm::guf::Assignment>, 2>::type
    InstantiatedFunction;
