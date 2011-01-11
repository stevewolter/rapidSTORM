#include "Config.h"
#include "doc/help/context.h"
#include <dStorm/output/Traits.h>

namespace dStorm {
namespace fitter {
namespace residue_analysis {

Config::Config() 
: do_double_spot_analysis("DoubleSpotAnalysis", "Enable double spot search", false),
  asymmetry_threshold("AsymmetryThreshold", 
                        "Threshold for relative spot asymmetry", 0.15),
  required_peak_distance("RequiredPeakDistance",
                        "Minimum distance between double-spot peaks in nm", 500)
{
    asymmetry_threshold.helpID = HELP_AsymmetryThreshold;
    asymmetry_threshold.setHelp(
        "If spot residues are found to be more asymmetric than this "
        "value, double-spot analysis is performed. 0.1 is a good "
        "'aggressive' value here for much double-spot analysis, 1 "
        "disables the feature completely.");
    asymmetry_threshold.userLevel = simparm::Object::Intermediate;
    required_peak_distance.userLevel = simparm::Object::Intermediate;
}

Config::~Config() {}

void Config::registerNamedEntries(simparm::Node& n) 
{
    n.push_back(do_double_spot_analysis);
    n.push_back(asymmetry_threshold);
    n.push_back(required_peak_distance);
}

void Config::set_traits( output::Traits& rv ) const
{
    rv.two_kernel_improvement().is_given = do_double_spot_analysis();
}

}
}
}
