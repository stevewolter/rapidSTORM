#include "Filter.h"

namespace dStorm {
namespace output {

Filter::Result Filter::receiveLocalizations(const EngineResult& er)
{ 
    return fwd->receiveLocalizations(er); 
}

void Filter::destroy_suboutput() {
    fwd.reset();
}

}
}
