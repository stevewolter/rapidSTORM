#include <simparm/BoostUnits.hh>
#include "Config.h"
#include <simparm/Entry_Impl.hh>
#include "guf/psf/parameters.h"

namespace simparm {
template class simparm::Entry< boost::units::quantity<dStorm::guf::PSF::DeltaSigma<0>::Unit, double> >;
}
namespace dStorm {
namespace guf {

using namespace boost::units;

Config::Config() 
: simparm::Set("GUF", "Grand unified fitter"),
  z_range("ZRange", "Maximum sensible Z distance from equifocused plane", 1000 * boost::units::si::nanometre),
  theta_dist("ThetaDist", "Two-kernel distance threshold", 500 * boost::units::si::nanometre),
  negligible_x_step("NegligibleStepLength", 
        "Terminate at axial step length", 1E-2f * boost::units::si::nanometre),
  marquardtStartLambda("MarquardtStartLambda",
        "Start value for Marquardt lambda factor", 1E2),
  maximumIterationSteps("MaximumIterationSteps",
        "Maximum number of iteration steps for spot fitting", 20),
  free_sigmas("FreeSigmaFitting", "PSF width is free fit parameter", false),
  output_sigmas("OutputSigmas", "Store PSF covariance matrix", false),
  laempi_fit("LaempiPosition", "Laempi fit for positions", false),
  disjoint_amplitudes("LaempiAmplitudes", "Disjoint amplitude fit", false),
  two_kernel_fitting("TwoKernelFitting", "Compute two kernel improvement", false),
  mle_fitting("MLEFitting", "Improve fit with ML estimate", false),
  allow_disjoint("DisjointFitting", "Allow disjoint fitting", true),
  double_computation("DoublePrecision", "Compute with 64 bit floats", true)
{
    free_sigmas.userLevel = simparm::Object::Intermediate;
    output_sigmas.userLevel = simparm::Object::Intermediate;
    mle_fitting.userLevel = simparm::Object::Intermediate;
    theta_dist.userLevel = simparm::Object::Intermediate;
    negligible_x_step.userLevel = (simparm::Object::Intermediate);
    maximumIterationSteps.userLevel = (simparm::Object::Intermediate);
    marquardtStartLambda.userLevel = (simparm::Object::Expert);
    allow_disjoint.userLevel = (simparm::Object::Expert);
    double_computation.userLevel = (simparm::Object::Intermediate);
}

void Config::registerNamedEntries()
{
    push_back( z_range );
    push_back( marquardtStartLambda );
    push_back( negligible_x_step );
    push_back( maximumIterationSteps );
    push_back( free_sigmas );
    push_back( output_sigmas );
    push_back( laempi_fit );
    push_back( disjoint_amplitudes );
    push_back( two_kernel_fitting );
    push_back( theta_dist );
    push_back( mle_fitting );
    push_back( allow_disjoint );
    push_back( double_computation );
}

nonlinfit::levmar::Config Config::make_levmar_config() const
{
    nonlinfit::levmar::Config rv;
    rv.initial_lambda = marquardtStartLambda();
    return rv;
}

}
}
