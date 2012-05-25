#include "Config.h"

namespace dStorm {
namespace fit_window {

Config::Config()
: fit_window_size("FitWindowSize", "Fit window radius", 600 * boost::units::si::nanometre),
  allow_disjoint("DisjointFitting", "Allow disjoint fitting", true),
  double_computation("DoublePrecision", "Compute with 64 bit floats", true)
{
    fit_window_size.set_user_level( simparm::Intermediate );
    allow_disjoint.set_user_level( (simparm::Expert) );
    double_computation.set_user_level( (simparm::Intermediate) );
}

void Config::attach_ui( simparm::NodeHandle at ) {
     fit_window_size.attach_ui(at);
     allow_disjoint.attach_ui(at);
     double_computation.attach_ui(at);
}

}
}
