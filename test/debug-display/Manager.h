#ifndef DEBUG_DISPLAY_MANAGER_H
#define DEBUG_DISPLAY_MANAGER_H

#include "dStorm/helpers/DisplayManager.h"
#include <iostream>

class Manager : public dStorm::Display::Manager {
  public:
    Manager(std::ostream& write_to);
};

#endif
