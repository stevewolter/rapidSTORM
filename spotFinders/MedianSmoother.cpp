#include "MedianSmoother.h"

namespace dStorm {
namespace spotFinders {

std::auto_ptr<engine::SpotFinderFactory> make_Median() { 
    return std::auto_ptr<engine::SpotFinderFactory>(new MedianSmoother::Factory()); 
}

}
}
