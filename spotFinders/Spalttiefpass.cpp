#include "Spalttiefpass.h"

namespace dStorm {
namespace spotFinders {

std::auto_ptr<engine::SpotFinderFactory> make_Spalttiefpass() { 
    return std::auto_ptr<engine::SpotFinderFactory>(new Spalttiefpass::Factory()); 
}

}
}
