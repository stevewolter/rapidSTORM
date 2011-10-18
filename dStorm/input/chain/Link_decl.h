#ifndef DSTORM_INPUT_LINK_DECL_H
#define DSTORM_INPUT_LINK_DECL_H

#include <boost/ptr_container/clone_allocator.hpp>

namespace dStorm {
namespace input {
namespace chain {

class Link;
class Terminus;

}
}
}

namespace boost {
template <>
dStorm::input::chain::Link* new_clone<dStorm::input::chain::Link>
    ( const dStorm::input::chain::Link& );
template <>
void delete_clone<dStorm::input::chain::Link>(const dStorm::input::chain::Link*);
}

#endif
