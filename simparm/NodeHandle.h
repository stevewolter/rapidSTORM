#ifndef SIMPARM_NODE_HANDLE_HH
#define SIMPARM_NODE_HANDLE_HH

#include <boost/smart_ptr/shared_ptr.hpp>

namespace simparm {

class Node;

typedef boost::shared_ptr< Node > NodeHandle;

}


#endif
