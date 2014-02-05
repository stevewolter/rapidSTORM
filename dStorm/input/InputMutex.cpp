#include "dStorm/input/InputMutex.h"

namespace dStorm {
namespace input {
boost::recursive_mutex& global_mutex() {
    static boost::recursive_mutex mutex;
    return mutex;
}
}
}
