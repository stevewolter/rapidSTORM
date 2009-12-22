#ifndef DSTORM_MASTERCONFIG_H
#define DSTORM_MASTERCONFIG_H

#include <simparm/Node.hh>
#include <boost/shared_ptr.hpp>
#include "engine/CarConfig.h"

namespace dStorm {

class MasterConfig
{
  public:
    typedef std::auto_ptr<MasterConfig> OwnerPtr;
    typedef MasterConfig* Ptr;
    static OwnerPtr create();
    virtual ~MasterConfig();

    virtual void thread_safely_register_node( simparm::Node& node ) = 0;
    virtual void thread_safely_erase_node( simparm::Node& node ) = 0;

    virtual void add_modules( engine::CarConfig& config ) = 0;

    virtual void read_input(std::istream&) = 0;
};

}

#endif
