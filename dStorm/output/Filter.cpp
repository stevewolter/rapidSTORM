#include "Filter.h"

namespace dStorm {
namespace output {

void Filter::receiveLocalizations(const EngineResult& er)
{ 
    fwd->receiveLocalizations(er); 
}

void Filter::destroy_suboutput() {
    fwd.reset();
}

}
}
