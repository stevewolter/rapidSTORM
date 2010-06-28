#include "Config.h"

namespace dStorm {
namespace fitter {
namespace residue_analysis {

Config::Config() 
: asymmetry_threshold("AsymmetryThreshold", 
                        "Threshold for relative spot asymmetry", 1),
  required_peak_distance("RequiredPeakDistance",
                        "Minimum distance between double-spot peaks in nm", 250)
{
}

Config::~Config() {}

void Config::registerNamedEntries(simparm::Node& n) 
{
    n.push_back(asymmetry_threshold);
    n.push_back(required_peak_distance);
}

}
}
}
