#include <simparm/BoostUnits.hh>
#include "Config.h"
#include <simparm/Entry_Impl.hh>

namespace dStorm {
namespace guf {

using namespace boost::units;

Config::Config() 
: simparm::Set("GUF", "Grand unified fitter"),
  theta_dist("ThetaDist", "Two-kernel distance threshold", 500 * boost::units::si::nanometre),
  negligible_x_step("NegligibleStepLength", 
        "Terminate at axial step length", 1E-2f * boost::units::si::nanometre),
  marquardtStartLambda("MarquardtStartLambda",
        "Start value for Marquardt lambda factor", 1E2),
  maximumIterationSteps("MaximumIterationSteps",
        "Maximum number of iteration steps for spot fitting", 20),
  free_sigmas("FreeSigmaFitting", "PSF width is free fit parameter", false),
  output_sigmas("OutputSigmas", "Store PSF width", false),
  laempi_fit("LaempiPosition", "Laempi fit for positions", false),
  disjoint_amplitudes("LaempiAmplitudes", "Disjoint amplitude fit", false),
  two_kernel_fitting("TwoKernelFitting", "Compute two kernel improvement", false),
  mle_fitting("MLEFitting", "Improve fit with ML estimate", false)
{
    free_sigmas.userLevel = simparm::Object::Intermediate;
    output_sigmas.userLevel = simparm::Object::Intermediate;
    mle_fitting.userLevel = simparm::Object::Intermediate;
    theta_dist.userLevel = simparm::Object::Intermediate;
    negligible_x_step.userLevel = (simparm::Object::Intermediate);
    maximumIterationSteps.userLevel = (simparm::Object::Intermediate);
    marquardtStartLambda.userLevel = (simparm::Object::Expert);
}

void Config::registerNamedEntries()
{
    fit_window_config.attach_ui( *this );
    marquardtStartLambda.attach_ui(*this);
    negligible_x_step.attach_ui(*this);
    maximumIterationSteps.attach_ui(*this);
    free_sigmas.attach_ui(*this);
    output_sigmas.attach_ui(*this);
    laempi_fit.attach_ui(*this);
    disjoint_amplitudes.attach_ui(*this);
    two_kernel_fitting.attach_ui(*this);
    theta_dist.attach_ui(*this);
    mle_fitting.attach_ui(*this);
}

nonlinfit::levmar::Config Config::make_levmar_config() const
{
    nonlinfit::levmar::Config rv;
    rv.initial_lambda = marquardtStartLambda();
    return rv;
}

}
}
