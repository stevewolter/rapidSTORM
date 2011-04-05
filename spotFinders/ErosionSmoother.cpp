#include "ErosionSmoother.h"

namespace dStorm {
namespace spotFinders {

std::auto_ptr<engine::spot_finder::Factory> make_Erosion() { 
    return std::auto_ptr<engine::spot_finder::Factory>(new ErosionSmoother::Factory()); 
}

}
}
