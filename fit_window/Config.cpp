#include "Config.h"

namespace dStorm {
namespace fit_window {

Config::Config()
: fit_window_size("FitWindowSize", "Fit window radius", 600 * boost::units::si::nanometre),
  allow_disjoint("DisjointFitting", "Allow disjoint fitting", true),
  double_computation("DoublePrecision", "Compute with 64 bit floats", true)
{
    fit_window_size.userLevel = simparm::Object::Intermediate;
    allow_disjoint.userLevel = (simparm::Object::Expert);
    double_computation.userLevel = (simparm::Object::Intermediate);
}

void Config::registerNamedEntries( simparm::Node& at ) {
    at.push_back( fit_window_size );
    at.push_back( allow_disjoint );
    at.push_back( double_computation );
}

}
}
