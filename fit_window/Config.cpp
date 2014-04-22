#include "fit_window/Config.h"

namespace dStorm {
namespace fit_window {

Config::Config()
: fit_window_size("FitWindowSize", 600 * boost::units::si::nanometre)
{
    fit_window_size.set_user_level( simparm::Intermediate );
}

void Config::attach_ui( simparm::NodeHandle at ) {
     fit_window_size.attach_ui(at);
}

}
}
