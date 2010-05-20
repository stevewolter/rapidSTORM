#include <stdexcept>
#include "Method.h"
#include "Source.h"

namespace dStorm {
namespace input {

BaseMethod::BaseMethod(
    const std::string& name,
    const std::string& desc
) : simparm::Object(name, desc),
    output_file_basename("OutputFileBasename", "")
{}

std::auto_ptr< BaseSource > BaseMethod::makeSource(const Config &master)
{ 
    std::auto_ptr<BaseSource> src(impl_makeSource()); 
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
