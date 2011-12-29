#include "fwd.h"
#include <boost/thread/mutex.hpp>

namespace dStorm {
namespace input {
boost::mutex& global_mutex();
}
}
