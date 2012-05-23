#ifndef DSTORM_EXPRESSION_CONFIG_H
#define DSTORM_EXPRESSION_CONFIG_H

#include "SimpleFilters.h"
#include <simparm/TriggerEntry.hh>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/units/systems/camera/intensity.hpp>
#include "Config_decl.h"
#include "CommandLine.h"
#include <dStorm/output/Capabilities.h>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm {
namespace expression {

class Parser;

struct Config : public config::ExpressionManager
{
    typedef Source Output;

    Config();
    ~Config();

    void attach_ui( simparm::NodeHandle );
    static std::string get_name() { return "Expression"; }
    static std::string get_description() { return "Expression filter"; }
    static simparm::Object::UserLevel get_user_level() { return simparm::Object::Beginner; }

    void make_new_line();
    bool can_work_with(output::Capabilities);
    bool determine_output_capabilities( output::Capabilities& cap ) { return true; }

  private:
    friend class Source;

    boost::shared_ptr<Parser> parser;
    SimpleFilters simple;
    typedef boost::ptr_vector< config::CommandLine > Lines;
    Lines lines;
    simparm::TriggerEntry new_line;
    int next_ident;

    simparm::NodeHandle current_ui;
    simparm::BaseAttribute::ConnectionStore listening;

    void expression_changed( std::string, std::auto_ptr<source::LValue> ) {}
};

}
}

#endif
