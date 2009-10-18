#include "Manager.h"
#include "wxManager.h"

namespace dStorm {
namespace Display {

static bool man_was_started = false;

Manager& Manager::getSingleton() {
    man_was_started = true;
    return wxManager::getSingleton();
}

bool Manager::was_started() {
    return man_was_started;
}

}
}
