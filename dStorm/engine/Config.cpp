#include "Config.h"
#include <math.h>
#include <limits>
#include "doc/help/context.h"
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitter.h>
#include <dStorm/engine/SpotFitterFactory.h>

#include <dStorm/helpers/thread.h>
#include <simparm/ChoiceEntry_Impl.hh>
#include <dStorm/output/Basename.h>

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

namespace dStorm {
namespace engine {

_Config::_Config()
:   Set("Engine", "Processing options"),
    sigma_x("SigmaX", "Std. dev. in X direction", 2 * cs_units::camera::pixel),
    sigma_y("SigmaY", "Std. dev. in Y direction", 2 * cs_units::camera::pixel),
    sigma_xy("SigmaXY", "Correlation between X and Y direction", 0),
    delta_sigma("DeltaSigma", "Accepted error in sigma estimation", 0.05),
    maskSizeFactor("MaskSizeFactor", "Proportionality factor "
                    "for smoothing and NMS masks", 1.5),
    fitSizeFactor("FitSizeFactor", "Proportionality factor for fit "
                    "window size", 3),
    spotFindingMethod("SpotFindingMethod", "Spot finding method"),
    spotFittingMethod("SpotFittingMethod", "Spot fitting method"),
    fixSigma("FixSigma", "Disable std. dev. estimation", false),
    motivation("Motivation", "Spot search eagerness", 3),
    amplitude_threshold("AmplitudeThreshold", "Amplitude discarding threshold"),
    pistonCount("CPUNumber", "Number of CPUs to use")
{
    PROGRESS("Building dStorm Config");
    
    sigma_x.setHelp("The value of the squared standard deviation in X "
                    "direction of an exponential curve that fits the "
                    "expected spots.");
    sigma_y.setHelp("The value of the squared standard deviation in Y "
                    "direction of an exponential curve that fits the "
                    "expected spots.");
    sigma_xy.setHelp("Correlation between X and Y data for an "
                     "exponential "
                     "curve that fits the expected spots.");
    
    amplitude_threshold.value = 3000 * cs_units::camera::ad_counts;
    amplitude_threshold().reset();

    maskSizeFactor.setUserLevel(Object::Expert);
    fitSizeFactor.setUserLevel(Object::Expert);

    delta_sigma.setUserLevel(Object::Expert);

    fixSigma.helpID = HELP_FixSigma;
    fixSigma.setUserLevel(Object::Beginner);
    fixSigma.setHelp("If this option is enabled, the program "
                     "will not try to guess correct values for the "
                     "PSF standard deviation. This is useful for very "
                     "dense spot populations where the overlap between "
                     "spots prevents correct parameter estimation.");

    motivation.setHelp("Abort spot search when this many successive "
                        "bad candidates are found.");
    motivation.setUserLevel(Object::Intermediate);

    amplitude_threshold.helpID = HELP_AmplitudeThreshold;
    amplitude_threshold.setUserLevel(Object::Beginner);
    amplitude_threshold.setHelp("Every fit attempt with an amplitude higher "
                                "than this threshold will be considered a "
                                "localization, and those below the threshold "
                                "are discarded immediately. Compared with the "
                                "other amplitude threshold in the Viewer, this "
                                "threshold is already enforced during computation,"
                                "thus saving computation time and avoiding false "
                                "positives; however, contrary to the other threshold, "
                                "it's application is not reversible.");

    pistonCount.setUserLevel(Object::Expert);
    pistonCount.helpID = HELP_CPUNumber;
    pistonCount.setHelp("Use this many parallel threads to compute the "
                        "STORM result. If you notice a low CPU usage, "
                        "raise this value to the number of cores you "
                        "have.");
#if defined(_SC_NPROCESSORS_ONLN)
    int pn = sysconf(_SC_NPROCESSORS_ONLN);
    pistonCount = (pn == 0) ? 8 : pn;
#elif defined(HAVE_WINDOWS_H)
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    pistonCount = info.dwNumberOfProcessors;
#else
    pistonCount.setUserLevel(Object::Beginner);
    pistonCount = 8;
#endif

    spotFindingMethod.helpID = HELP_Smoother;
    spotFindingMethod.set_auto_selection( true );

    spotFittingMethod.set_auto_selection( true );
}

_Config::~_Config() {
    /* No special stuff here. Declared in this file to avoid necessity
     * of including simparm::ChoiceEntry implementation when using
     * Config. */
}

void _Config::registerNamedEntries() {
    push_back(sigma_x);
    push_back(sigma_y);
    push_back(sigma_xy);
    push_back(delta_sigma);
    push_back(fixSigma);
    push_back(amplitude_threshold);

    push_back(maskSizeFactor);
    push_back(fitSizeFactor);
    push_back(spotFindingMethod);
    push_back(spotFittingMethod);

    push_back(motivation);

    push_back( pistonCount );
}


Config::SigmaUserLevel::SigmaUserLevel(_Config &config)
 : Node::Callback( Event::ValueChanged ),
   config(config) 
{ 
    receive_changes_from( config.fixSigma.value );
    adjust();
}

void Config::SigmaUserLevel::operator()(const Event&) 
{
    adjust();
}

void Config::SigmaUserLevel::adjust() {
    Object::UserLevel userLevel
        = (config.fixSigma()) ? Object::Beginner
                        : Object::Expert;
    config.sigma_x.setUserLevel(userLevel);
    config.sigma_y.setUserLevel(userLevel);
    config.sigma_xy.setUserLevel(userLevel);
}

Config::Config() 
: user_level_watcher( *this )
{ 
    registerNamedEntries();
}

Config::Config(const Config& c) 
: _Config(c),
  user_level_watcher( *this ) /* Fresh callbacks - don't want to fremd-listen */
{ 
    registerNamedEntries();
}

void _Config::addSpotFinder( std::auto_ptr<SpotFinderFactory> factory ) {
    spotFindingMethod.addChoice( factory );
}

void _Config::addSpotFitter( std::auto_ptr<SpotFitterFactory> factory ) {
    spotFittingMethod.addChoice( factory );
}

void _Config::set_variables( output::Basename& bn ) const
{
    std::stringstream ss;
    if ( amplitude_threshold().is_set() ) 
        ss << (*amplitude_threshold()).value();
    else
        ss << "auto";
    bn.set_variable("thres", ss.str());

    spotFindingMethod().set_variables( bn );
    spotFittingMethod().set_variables( bn );
}

}
}
