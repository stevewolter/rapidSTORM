#include <simparm/BoostUnits.h>
#include "Config.h"

namespace dStorm {
namespace guf {

using namespace boost::units;

Config::Config() 
: name_object(getName(), "Levenberg-Marquardt Fitter"),
  theta_dist("ThetaDist", 500 * boost::units::si::nanometre),
  negligible_x_step("NegligibleStepLength", 1E-2f * boost::units::si::nanometre),
  marquardtStartLambda("MarquardtStartLambda", 1E2),
  maximumIterationSteps("MaximumIterationSteps", 20),
  free_sigmas("FreeSigmaFitting", false),
  output_sigmas("OutputSigmas", false),
  laempi_fit("LaempiPosition", false),
  disjoint_amplitudes("LaempiAmplitudes", false),
  two_kernel_fitting("TwoKernelFitting", false),
  mle_fitting("MLEFitting", false)
{
    free_sigmas.set_user_level( simparm::Intermediate );
    output_sigmas.set_user_level( simparm::Intermediate );
    mle_fitting.set_user_level( simparm::Intermediate );
    theta_dist.set_user_level( simparm::Intermediate );
    negligible_x_step.set_user_level( (simparm::Intermediate) );
    maximumIterationSteps.set_user_level( (simparm::Intermediate) );
    marquardtStartLambda.set_user_level( (simparm::Expert) );
}

void Config::attach_ui( simparm::NodeHandle at )
{
    simparm::NodeHandle m = name_object.attach_ui( at );
    fit_window_config.attach_ui( m );
    marquardtStartLambda.attach_ui(m);
    negligible_x_step.attach_ui(m);
    maximumIterationSteps.attach_ui(m);
    free_sigmas.attach_ui(m);
    output_sigmas.attach_ui(m);
    laempi_fit.attach_ui(m);
    disjoint_amplitudes.attach_ui(m);
    two_kernel_fitting.attach_ui(m);
    theta_dist.attach_ui(m);
    mle_fitting.attach_ui(m);
}

nonlinfit::levmar::Config Config::make_levmar_config() const
{
    nonlinfit::levmar::Config rv;
    rv.initial_lambda = marquardtStartLambda();
    return rv;
}

}
}
