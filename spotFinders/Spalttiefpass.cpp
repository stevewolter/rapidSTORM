#include "Spalttiefpass.h"

namespace dStorm {
namespace spotFinders {

std::auto_ptr<engine::spot_finder::Factory> make_Spalttiefpass() { 
    return std::auto_ptr<engine::spot_finder::Factory>(new Spalttiefpass::Factory()); 
}

}
}
