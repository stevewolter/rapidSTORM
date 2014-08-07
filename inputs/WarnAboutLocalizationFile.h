#include <memory>
#include "engine/Image.h"
#include "input/Link.h"

namespace dStorm {
namespace inputs {

std::unique_ptr< input::Link<engine::ImageStack> > make_warn_about_localization_file();

}
}
