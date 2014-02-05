#ifndef DSTORM_OUTPUT_OUTPUT_DECL_H
#define DSTORM_OUTPUT_OUTPUT_DECL_H

#include <boost/ptr_container/clone_allocator.hpp>

namespace dStorm {
namespace output {

class Output;

}
}

namespace boost {
template <>
dStorm::output::Output* new_clone<dStorm::output::Output>
    ( const dStorm::output::Output& );
template <>
void delete_clone<dStorm::output::Output>(const dStorm::output::Output*);
}

#endif
