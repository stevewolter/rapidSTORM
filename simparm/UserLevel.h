#ifndef SIMPARM_USER_LEVEL_H
#define SIMPARM_USER_LEVEL_H

#include <iosfwd>

namespace simparm {

enum UserLevel { Beginner = 10, Intermediate = 20, 
                    Expert = 30, Debug = 40 };

std::istream& operator>>(std::istream &i, UserLevel& ul);

}

#endif
