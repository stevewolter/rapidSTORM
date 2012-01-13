#include <cassert>
#include <nonlinfit/functions/Polynom.h>
#include <nonlinfit/VectorPosition.h>
#include <nonlinfit/terminators/StepLimit.h>
#include "Fitter.hpp"
#include "dejagnu.h"

/** \cond */

namespace nonlinfit {
namespace levmar {

void run_unit_tests(TestState& state) {
    typedef nonlinfit::Bind< static_power::Expression, static_power::BaseValue > Lambda;
    Lambda a;
    static_power::SimpleFunction<0,1> function(a);

    a( static_power::Variable() ) = 5;
    a( static_power::Power() ) = 2;
    a( static_power::Prefactor() ) = 2;

    Config config;
    Fitter fitter(config);
    VectorPosition< Lambda > mover(a);
    terminators::StepLimit terminator(150);
    fitter.fit( function, mover, terminator );
    state( std::abs( a( static_power::Variable() ).value() ) < 1E-10,
           "LM fitter finds minimum position of trivial function");

}

}
}

/** \endcond */
