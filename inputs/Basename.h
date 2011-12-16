#ifndef DSTORM_INPUT_BASENAME_H
#define DSTORM_INPUT_BASENAME_H

#include "Basename_decl.h"
#include <dStorm/input/Source.h>
#include <dStorm/input/chain/Filter.h>

#include <simparm/Entry.hh>
#include <simparm/Structure.hh>
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/chain/Context.h>

namespace dStorm {
namespace input {

using namespace chain;

namespace Basename {

class Config : public simparm::Object {
  public:
    simparm::StringEntry output;

    Config();
    void registerNamedEntries();
};

class ChainLink 
: public chain::Filter, public simparm::Listener , simparm::Structure<Config>
{
    chain::MetaInfo::Ptr traits;
    std::string default_output_basename;
    bool user_changed_output;

  protected:
    void operator()(const simparm::Event&);

  public:
    ChainLink();
    ChainLink(const ChainLink&);
    ChainLink* clone() const { return new ChainLink(*this); }
    simparm::Node& getNode() { return static_cast<Config&>(*this); }

    AtEnd traits_changed( TraitsRef r, Link* l);

    BaseSource* makeSource() { return Forwarder::makeSource(); }
};

}

}
}

#endif
