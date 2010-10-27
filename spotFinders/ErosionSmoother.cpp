#include "ErosionSmoother.h"

namespace dStorm {
namespace spotFinders {

std::auto_ptr<engine::SpotFinderFactory> make_Erosion() { 
    return std::auto_ptr<engine::SpotFinderFactory>(new ErosionSmoother::Factory()); 
}

}
}
