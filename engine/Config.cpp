#include "engine/Config.h"
#include <math.h>
#include <limits>
#include "help_context.h"
#include "engine/SpotFinder.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#include <cc++/thread.h>
#include <simparm/ChoiceEntry_Impl.hh>

using namespace dStorm;

_Config::_Config()
:   Set("Engine", "Processing options"),
    sigma_x("SigmaX", "Std. dev. in X direction", 2),
    sigma_y("SigmaY", "Std. dev. in Y direction", 2),
    sigma_xy("SigmaXY", "Correlation between X and Y direction", 0),
    delta_sigma("DeltaSigma", "Accepted error in sigma estimation", 0.05),
    sigma_xy_negligible_limit("CorrNegligibleLimit",
        "Limit up to which X-Y correlation is considered negligible", 0.1),
    marquardtStartLambda("MarquardtStartLambda",
        "Start value for Marquardt lambda factor", 1E-2),
    maximumIterationSteps("MaximumIterationSteps",
        "Maximum number of iteration steps for spot fitting", 15),
    negligibleStepLength("NegligibleStepLength", 
        "Maximum length of negligibly short iteration step", 5E-3),
    successiveNegligibleSteps("SuccessiveNegligibleSteps",
        "Number of successive negligibly short steps indicating fit "
        "success", 1),
    maskSizeFactor("MaskSizeFactor", "Proportionality factor "
                    "for smoothing and NMS masks", 1.5),
    fitSizeFactor("FitSizeFactor", "Proportionality factor for fit "
                    "window size", 3),
    spotFindingMethod("SpotFindingMethod", "Spot finding method"),
    fixSigma("FixSigma", "Disable std. dev. estimation", false),
    freeSigmaFitting("FreeSigmaFitting", "Fit with free covariance matrix",
                     false),
    asymmetry_threshold("AsymmetryThreshold", 
                        "Threshold for relative spot asymmetry", 1),
    motivation("Motivation", "Spot search eagerness", 3),
    amplitude_threshold("AmplitudeThreshold", 
                        "Amplitude discarding threshold", 3000),
    reconstruction_mask_1("ReconstructionMask1", 
       "Erosion mask size for fillhole smoother", 3),
    reconstruction_mask_2("ReconstructionMask2",
       "Dilation mask size for fillhole smoother", 25),
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
    
    freeSigmaFitting.helpID = HELP_FreeForm;

    sigma_xy_negligible_limit.setUserLevel(Entry::Intermediate);
    marquardtStartLambda.setUserLevel(Entry::Expert);
    maximumIterationSteps.setUserLevel(Entry::Intermediate);
    negligibleStepLength.setUserLevel(Entry::Intermediate);
    successiveNegligibleSteps.setUserLevel(Entry::Expert);

    maskSizeFactor.setUserLevel(Entry::Expert);
    fitSizeFactor.setUserLevel(Entry::Expert);

    delta_sigma.setUserLevel(Entry::Expert);
    asymmetry_threshold.helpID = HELP_AsymmetryThreshold;
    asymmetry_threshold.setHelp(
        "If spot residues are found to be more asymmetric than this "
        "value, double-spot analysis is performed. 0.1 is a good "
        "'aggressive' value here for much double-spot analysis, 1 "
        "disables the feature completely.");

    fixSigma.helpID = HELP_FixSigma;
    fixSigma.setUserLevel(Entry::Beginner);
    fixSigma.setHelp("If this option is enabled, the program "
                     "will not try to guess correct values for the "
                     "PSF standard deviation. This is useful for very "
                     "dense spot populations where the overlap between "
                     "spots prevents correct parameter estimation.");

    motivation.setHelp("Abort spot search when this many successive "
                        "bad candidates are found.");
    motivation.setUserLevel(Entry::Intermediate);

    amplitude_threshold.helpID = HELP_AmplitudeThreshold;
    amplitude_threshold.setUserLevel(Entry::Beginner);
    amplitude_threshold.setHelp("Every fit attempt with an amplitude higher "
                                "than this threshold will be considered a "
                                "localization, and those below the threshold "
                                "are discarded immediately. Compared with the "
                                "other amplitude threshold in the Viewer, this "
                                "threshold is already enforced during computation,"
                                "thus saving computation time and avoiding false "
                                "positives; however, contrary to the other threshold, "
                                "it's application is not reversible.");
    reconstruction_mask_1.setUserLevel(Entry::Expert);
    reconstruction_mask_2.setUserLevel(Entry::Expert);

    pistonCount.helpID = HELP_CPUNumber;
    pistonCount.setHelp("Use this many parallel threads to compute the "
                        "STORM result. If you notice a low CPU usage, "
                        "raise this value to the number of cores you "
                        "have.");
    pistonCount = 2;

    spotFindingMethod.helpID = HELP_Smoother;
    spotFindingMethod.set_auto_selection( true );
    spotFindingMethod.addChoice(SpotFinder::Average,
        "Average", "Smooth by average");
    spotFindingMethod.addChoice(SpotFinder::Median,
        "Median", "Smooth by median");
    spotFindingMethod.addChoice(SpotFinder::Erosion,
        "Erosion", "Erode image");
#ifdef D3_INTERNAL_VERSION
    spotFindingMethod.addChoice(SpotFinder::Reconstruction,
      "Reconstruction", "Morphologically reconstruct image");
#endif
    spotFindingMethod.addChoice(SpotFinder::Gaussian,
       "Gaussian", "Smooth with gaussian " "kernel");

}

void _Config::registerNamedEntries() {
    register_entry(&sigma_x);
    register_entry(&sigma_y);
    register_entry(&sigma_xy);
    register_entry(&delta_sigma);
    register_entry(&sigma_xy_negligible_limit);
    register_entry(&fixSigma);
    register_entry(&freeSigmaFitting);

    register_entry(&maskSizeFactor);
    register_entry(&reconstruction_mask_1);
    register_entry(&reconstruction_mask_2);
    register_entry(&spotFindingMethod);

    register_entry(&marquardtStartLambda);
    register_entry(&maximumIterationSteps);
    register_entry(&negligibleStepLength);
    register_entry(&successiveNegligibleSteps);
    register_entry(&fitSizeFactor);

    register_entry(&amplitude_threshold);
    register_entry(&motivation);
    register_entry(&asymmetry_threshold);

    push_back( pistonCount );
}


void Config::SigmaUserLevel::operator()
    (Node& src, Cause cause, Node *) 
{
    if (&src == &config.fixSigma && cause == ValueChanged) {
        Entry::UserLevel userLevel
            = (config.fixSigma()) ? Entry::Beginner
                         : Entry::Expert;
        config.sigma_x.setUserLevel(userLevel);
        config.sigma_y.setUserLevel(userLevel);
        config.sigma_xy.setUserLevel(userLevel);
    }
}

Config::Config() 
: user_level_watcher( *this )
{ 
}

Config::Config(const Config& c) 
: Node(c), 
  simparm::Structure<_Config>(c),
  user_level_watcher( *this ) /* Fresh callbacks - don't want to fremd-listen */
{ 
}
