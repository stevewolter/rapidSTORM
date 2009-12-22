#include "DisplayManager.h"

namespace dStorm {
namespace Display {

static Manager* m;

void Manager::setSingleton(Manager& manager) {
    m = &manager;
}

Manager& Manager::getSingleton() {
    return *m;
}

}
}
