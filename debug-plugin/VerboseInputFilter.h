#ifndef VERBOSE_INPUT_FILTER_H
#define VERBOSE_INPUT_FILTER_H

#include <dStorm/input/Source_impl.h>
#include <dStorm/input/chain/FullForwarder.h>
#include <simparm/Structure.hh>
#include <simparm/Entry.hh>
#include <simparm/Object.hh>

struct Config : public simparm::Object {
    simparm::BoolEntry verbose;
    Config() : Object("VerboseInput", "Verbose input filter"), 
               verbose("BeVerbose", "Be verbose") {}
    void registerNamedEntries() { push_back(verbose); }
};

class VerboseInputFilter 
: public dStorm::input::chain::FullForwarder
{
    simparm::Structure<Config> config;
  public:
    VerboseInputFilter() : dStorm::input::chain::Forwarder() {}
    ~VerboseInputFilter() {}
    VerboseInputFilter* clone() const { return new VerboseInputFilter(*this); }

    void modify_traits( TraitsRef );
    void modify_context( ContextRef );

    dStorm::input::Source<dStorm::engine::Image>* makeSource( std::auto_ptr< dStorm::input::Source<dStorm::engine::Image> > );
    dStorm::input::Source<dStorm::Localization>* makeSource( std::auto_ptr< dStorm::input::Source<dStorm::Localization> > );

    simparm::Node& getNode() { return config; }
};

#endif
