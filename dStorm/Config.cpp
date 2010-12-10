#include "Config.h"
#include <math.h>
#include <limits>

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#include <cc++/thread.h>

using namespace dStorm;

#define NAME "Engine"
#define DESC "Processing options"

_Config::_Config() throw()
:   ConfigObject(NAME, DESC),
    sigma_x("SigmaX", "Std. dev. in X direction", 2),
    sigma_y("SigmaY", "Std. dev. in Y direction", 2),
    sigma_xy("SigmaXY", "Correlation between X and Y direction", 0),
    delta_sigma("DeltaSigma", "Accepted error in sigma estimation", 0.05),
    sigma_xy_negligible_limit("CorrNegligibleLimit",
        "Limit up to which X-Y correlation is considered negligible", 0.1),
    marquardtStartLambda("MarquardtStartLambda",
        "Start value for Marquardt lambda factor", 1E-4),
    maximumIterationSteps("MaximumIterationSteps",
        "Maximum number of iteration steps for spot fitting", 10),
    negligibleStepLength("NegligibleStepLength", 
        "Maximum length of negligibly short iteration step", 1E-2),
    successiveNegligibleSteps("SuccessiveNegligibleSteps",
        "Number of successive negligibly short steps indicating fit "
        "success", 1),
    maskSizeFactor("MaskSizeFactor", "Proportionality factor "
                    "for smoothing and NMS masks", 1.5),
    fitSizeFactor("FitSizeFactor", "Proportionality factor for fit "
                    "window size", 3),
    spotFindingMethod("SpotFindingMethod", "Spot finding method"),
    fixSigma("FixSigma", "Disable std. dev. estimation", false),
    motivation("Motivation", "Spot search eagerness", 3),
    amplitude_threshold("AmplitudeThreshold", 
                        "Amplitude discarding threshold", 3000),
    amplitude_limit("AmplitudeLimit", "Amplitude upper limit", 
                        std::numeric_limits<double>::infinity()),
    reconstruction_mask_1("ReconstructionMask1", "", 3),
    reconstruction_mask_2("ReconstructionMask2", "", 25),
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

    sigma_xy_negligible_limit.setUserLevel(ConfigEntry::Intermediate);
    marquardtStartLambda.setUserLevel(ConfigEntry::Expert);
    maximumIterationSteps.setUserLevel(ConfigEntry::Intermediate);
    negligibleStepLength.setUserLevel(ConfigEntry::Intermediate);
    successiveNegligibleSteps.setUserLevel(ConfigEntry::Expert);

    maskSizeFactor.setUserLevel(ConfigEntry::Expert);
    fitSizeFactor.setUserLevel(ConfigEntry::Expert);

    delta_sigma.setUserLevel(ConfigEntry::Expert);

    spotFindingMethod.setUserLevel(ConfigEntry::Expert);

    fixSigma.setUserLevel(ConfigEntry::Beginner);
    fixSigma.setHelp("If this option is enabled, the program "
                     "will not try to guess correct values for the "
                     "PSF standard deviation. This is useful for very "
                     "dense spot populations where the overlap between "
                     "spots prevents correct parameter estimation.");

    motivation.setHelp("Abort spot search when this many successive "
                        "bad candidates are found.");
    motivation.setUserLevel(ConfigEntry::Intermediate);

    amplitude_threshold.setUserLevel(ConfigEntry::Beginner);
    amplitude_threshold.setHelp("Every fit attempt with an amplitude higher "
                                "than this threshold will be considered a "
                                "localization, and those below the threshold "
                                "are discarded immediately. Compared with the "
                                "other amplitude threshold in the Viewer, this "
                                "threshold is already enforced during computation,"
                                "thus saving computation time and avoiding false "
                                "positives; however, contrary to the other threshold, "
                                "it's application is not reversible.");
    amplitude_limit.setUserLevel(ConfigEntry::Beginner);
    amplitude_limit.setHelp("Every fit attempt with an amplitude higher "
        "than this value will be considered an artifact and discarded "
        "from the results. This can be useful for filtering double "
        "fluorophore emissions.");
    reconstruction_mask_1.setUserLevel(ConfigEntry::Expert);
    reconstruction_mask_2.setUserLevel(ConfigEntry::Expert);

    pistonCount.setHelp("Use this many parallel threads to compute the "
                        "STORM result. If you notice a low CPU usage, "
                        "raise this value to the number of cores you "
                        "have.");
    pistonCount = 2;
}

void _Config::registerEntries() throw() {
    spotFindingMethod.addChoice("Average", "Smooth by average and sort "
                                            "local maxima");
    spotFindingMethod.addChoice("Median", "Smooth by median and sort "
                                            "local maxima");
    spotFindingMethod.addChoice("Erosion", "Erode image");
    spotFindingMethod.addChoice("Reconstruction", "Morphologically "
                                            "reconstruct image");
    spotFindingMethod.addChoice("Gaussian", "Smooth with gaussian "
                                            "kernel");
    spotFindingMethod.setValue("Average");

    register_entry(&sigma_x);
    register_entry(&sigma_y);
    register_entry(&sigma_xy);
    register_entry(&delta_sigma);
    register_entry(&sigma_xy_negligible_limit);
    register_entry(&marquardtStartLambda);
    register_entry(&maximumIterationSteps);
    register_entry(&negligibleStepLength);
    register_entry(&successiveNegligibleSteps);
    register_entry(&fixSigma);
    register_entry(&maskSizeFactor);
    register_entry(&fitSizeFactor);
    register_entry(&spotFindingMethod);
    register_entry(&motivation);
    register_entry(&amplitude_threshold);
    register_entry(&amplitude_limit);
    register_entry(&reconstruction_mask_1);
    register_entry(&reconstruction_mask_2);
    push_back( pistonCount );
}


void Config::SigmaUserLevel::operator()
    (ConfigNode& src, Cause cause, ConfigNode *) throw() 
{
    if (&src == &config.fixSigma && cause == ValueChanged) {
        ConfigEntry::UserLevel userLevel
            = (config.fixSigma()) ? ConfigEntry::Beginner
                         : ConfigEntry::Expert;
        config.sigma_x.setUserLevel(userLevel);
        config.sigma_y.setUserLevel(userLevel);
        config.sigma_xy.setUserLevel(userLevel);
    }
}

Config::Config() throw() 
: ConfigObject(NAME, DESC) ,
  user_level_watcher( *this )
{ 
    registerEntries(); 
}

Config::Config(const Config& c) throw() 
: ConfigObject(c), 
  _Config(c),
  user_level_watcher( *this ) /* Fresh callbacks - don't want to fremd-listen */
{ 
    registerEntries(); 
}
