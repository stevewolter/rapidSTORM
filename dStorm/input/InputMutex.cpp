#include "InputMutex.h"

namespace dStorm {
namespace input {
ost::Mutex& global_mutex() {
    static ost::Mutex mutex;
    return mutex;
}
}
}
