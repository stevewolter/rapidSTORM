#include <stdexcept>
#include "Method.h"
#include "Source.h"

namespace dStorm {
namespace input {

std::auto_ptr< BaseSource > BaseMethod::makeSource(const Config &master)
{ 
    std::auto_ptr<BaseSource> src(impl_makeSource()); 
    src->apply_global_settings( master );
    return src;
}

BaseMethod *BaseMethod::clone() const
{ 
    throw std::logic_error(
        "Input methods must be cloned "
        "together with a master config reference."
    ); 
}

}
}
