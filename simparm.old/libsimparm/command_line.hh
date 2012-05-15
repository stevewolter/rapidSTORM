#ifndef SIMPARM_COMMANDLINE_HH
#define SIMPARM_COMMANDLINE_HH

#include "Object.hh"

namespace simparm {

int readConfig(Node&, int argc, char *argv[]);
void printHelp(const Node&, std::ostream &o);

}

#endif
