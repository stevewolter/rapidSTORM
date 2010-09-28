#include "ImageTraits.h"

namespace dStorm {
namespace input {

GenericImageTraits::GenericImageTraits()
: dim(1), first_frame(0 * cs_units::camera::frame),
  photon_response(16 * cs_units::camera::ad_count),
  background_standard_deviation_is_set(false) {}

}
}
