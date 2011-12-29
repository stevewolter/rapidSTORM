#ifndef DSTORM_EXPRESSION_CONFIG_H
#define DSTORM_EXPRESSION_CONFIG_H

#include "SimpleFilters.h"
#include <simparm/Object.hh>
#include <simparm/TriggerEntry.hh>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/units/systems/camera/intensity.hpp>
#include "Config_decl.h"
#include "CommandLine.h"
#include "types.h"
#include <dStorm/output/Capabilities.h>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm {
namespace expression {

struct Config : public simparm::Object, public simparm::Listener, public config::ExpressionManager
{
    Config();
    Config(const Config&);
    ~Config();

    void operator()(const simparm::Event&);
    bool can_work_with(output::Capabilities);
    bool determine_output_capabilities( output::Capabilities& cap ) { return true; }

  private:
    void registerNamedEntries();

    friend class Source;

    boost::shared_ptr< variable_table > variables;
    SimpleFilters simple;
    typedef boost::ptr_vector< config::CommandLine > Lines;
    Lines lines;
    simparm::TriggerEntry new_line;
    int next_ident;

    void expression_changed( std::string, std::auto_ptr<source::LValue> ) {}
    simparm::Node& getNode()  { return *this; }

};

}
}

#endif
