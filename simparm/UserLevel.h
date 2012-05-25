#ifndef SIMPARM_USER_LEVEL_H
#define SIMPARM_USER_LEVEL_H

namespace simparm {

enum UserLevel { Beginner = 10, Intermediate = 20, 
                    Expert = 30, Debug = 40 };

inline std::istream& operator>>(std::istream &i, UserLevel& ul)
    { int v; i >> v; ul = (UserLevel)v; return i; }

}

#endif
