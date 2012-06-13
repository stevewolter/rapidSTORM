#ifndef SIMPARM_CHOICE_H
#define SIMPARM_CHOICE_H

#include <string>
#include <simparm/NodeHandle.h>

namespace simparm {

struct Choice {
    virtual ~Choice() {}
    virtual Choice* clone() const = 0;
    virtual std::string getName() const = 0;
    virtual void attach_ui( simparm::NodeHandle to ) = 0;
};

}

#endif
