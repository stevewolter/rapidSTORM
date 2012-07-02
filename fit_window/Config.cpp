#include "Config.h"

namespace dStorm {
namespace fit_window {

Config::Config()
: fit_window_size("FitWindowSize", 600 * boost::units::si::nanometre),
  allow_disjoint("DisjointFitting", true),
  double_computation("DoublePrecision", true)
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
