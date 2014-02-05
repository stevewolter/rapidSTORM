#include "fit_window/fit_position_out_of_range.h"

namespace dStorm {
namespace fit_window {

fit_position_out_of_range::fit_position_out_of_range()
: std::runtime_error("Selected fit position not in all layers of image") {}

}
}

