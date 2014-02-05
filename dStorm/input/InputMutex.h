#include "dStorm/input/fwd.h"
#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace dStorm {
namespace input {
boost::recursive_mutex& global_mutex();
typedef boost::lock_guard<boost::recursive_mutex> InputMutexGuard;
}
}
