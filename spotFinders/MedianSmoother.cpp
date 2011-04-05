#include "MedianSmoother.h"

namespace dStorm {
namespace spotFinders {

std::auto_ptr<engine::spot_finder::Factory> make_Median() { 
    return std::auto_ptr<engine::spot_finder::Factory>(new MedianSmoother::Factory()); 
}

}
}
