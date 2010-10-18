#ifndef VERBOSE_INPUT_FILTER_H
#define VERBOSE_INPUT_FILTER_H

#include <dStorm/input/Source_impl.h>
#include <dStorm/input/ChainLink.h>
#include <simparm/Structure.hh>
#include <simparm/Entry.hh>
#include <simparm/Object.hh>

namespace dStorm {
namespace input {

struct Config : public simparm::Object {
    simparm::BoolEntry verbose;
    Config() : Object("VerboseInput", "Verbose input filter"), 
               verbose("BeVerbose", "Be verbose") {}
    void registerNamedEntries() { push_back(verbose); }
};

class VerboseInputFilter 
: public ChainForwardingCapability<dStorm::engine::Image>,
  public 
{
    simparm::Structure<Config> config;
  public:
    VerboseInput() {}
    VerboseInput* clone() const { return new VerboseInput(*this); }

    void modify_traits( TraitsRef );
    void modify_context( ContextRef );

    simparm::Node& getNode() { return config; }
};

}
}

#endif
