#include "UserLevel.h"
#include <iostream>

namespace simparm {

std::istream& operator>>(std::istream &i, UserLevel& ul)
    { int v; i >> v; ul = (UserLevel)v; return i; }

}
