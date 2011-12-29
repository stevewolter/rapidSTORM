#include "InputMutex.h"

namespace dStorm {
namespace input {
boost::mutex& global_mutex() {
    static boost::mutex mutex;
    return mutex;
}
}
}
