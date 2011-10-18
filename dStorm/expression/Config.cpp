#include "types.h"
#include "Config.h"
#include "Source_filters.h"
#include "Parser.h"
#include "CommandLine.h"
#include "localization_variable_decl.h"
#include <dStorm/Localization.h>
#include <dStorm/localization/Traits.h>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Entry.hh>
#include <dStorm/traits/scalar_iterator.h>
#include <dStorm/traits/tags.h>
#include <boost/mpl/for_each.hpp>
#include <dStorm/output/FilterBuilder.h>
#include <dStorm/output/Capabilities.h>

namespace dStorm {
namespace expression {
namespace config {

struct Filter : public LValue {
    tree_node expression;

    Filter() : LValue("expression", "Expression") {}
    Filter* clone() const { return new Filter(*this); }

    source::Filter* make_lvalue() const { return new source::Filter(expression); }
    void set_expression_string( const std::string& expression, Parser& parser )  {
        std::string::const_iterator start = expression.begin(), end = expression.end();
        bool r = phrase_parse( start, end, parser, boost::spirit::ascii::space, this->expression );
        if ( ! r || start != end ) {
            throw std::runtime_error("Unable to parse expression '" + expression + "'");
        }
    }
};

#if 0
static tree_node parse_number( const std::string& expression, Filter::Parser& parser )
{
    tree_node rv;

    std::string::const_iterator start = expression.begin(), end = expression.end();
    bool r = phrase_parse( start, end, parser.numeric, boost::spirit::ascii::space, rv );
    if ( ! r || start != end )
        throw std::runtime_error("Unable to parse numeric expression '" + expression + "'");
    return rv;
}
#endif

struct VariableLValue : public LValue
{
    boost::shared_ptr< variable > v;
    tree_node expression;

    VariableLValue( const variable& v ) 
        : LValue(v.name, v.name), v(v.clone()), expression(DynamicQuantity()) {}
    VariableLValue* clone() const { return new VariableLValue(*this); }
    source::LValue* make_lvalue() const { return new source::Assignment( *v, expression ); }
    void set_expression_string( const std::string& expression, Parser& parser ) {
        std::string::const_iterator start = expression.begin(), end = expression.end();
        bool r = phrase_parse( start, end, parser.numeric, boost::spirit::ascii::space, this->expression );
        if ( ! r || start != end ) {
            throw std::runtime_error("Unable to parse expression '" + expression + "'");
        }
    }
};

CommandLine::CommandLine( std::string ident, boost::shared_ptr<variable_table> variables )
: simparm::Object("CommandLine" + ident, "Command line") ,
  simparm::Listener( simparm::Event::ValueChanged ),
    lvalue("LValue", "Value to assign to"),
    expression("Expression", "Expression to assign from"),
    variables(variables),
    manager(NULL)
{
    lvalue.userLevel = simparm::Object::Intermediate;
    expression.userLevel = simparm::Object::Intermediate;

    init_parser_symbol_table();
    lvalue.addChoice( new Filter() );
    for ( variable_table::const_iterator i = variables->begin(); i != variables->end(); ++i )
        lvalue.addChoice( new VariableLValue(*i) );
    registerNamedEntries();

}

CommandLine::~CommandLine() {
    stop_receiving_changes_from( lvalue.value );
    manager = NULL;
}

void CommandLine::operator()( const simparm::Event& )
{
    if ( expression() != "" )
        lvalue().set_expression_string( expression(), parser );
    publish();
}

void CommandLine::init_parser_symbol_table()
{
    for (boost::ptr_vector<variable>::iterator i = variables->begin(); i != variables->end(); ++i) {
        parser.symbols.add( i->name.c_str(), i - variables->begin() );
    }
}

void CommandLine::registerNamedEntries()
{
    push_back( lvalue );
    push_back( expression );
    receive_changes_from( lvalue.value );
    receive_changes_from( expression.value );
}

void CommandLine::publish() {
    if ( manager ) {
        std::auto_ptr<source::LValue> e;
        if ( expression() != "" )
            e.reset( lvalue.value().make_lvalue() );
        manager->expression_changed( getName(), e );
    }
}
void CommandLine::set_manager( ExpressionManager* manager ) {
    this->manager = manager;
    if ( manager ) manager->getNode().push_back( *this );
    publish();
}

CommandLine::CommandLine(const CommandLine& o)
: simparm::Object(o),
    simparm::Listener( simparm::Event::ValueChanged ),
    lvalue(o.lvalue), expression(o.expression),
    variables(o.variables), manager(o.manager)
{
    init_parser_symbol_table();
    registerNamedEntries();
}

}

Config::Config() 
: simparm::Object("Expression", "Expression filter"),
  simparm::Listener( simparm::Event::ValueChanged ),
  variables( variables_for_localization_fields() ),
  simple( variables ),
  new_line("NewExpression", "Add expression"),
  next_ident(0)
{ 
    new_line.userLevel = simparm::Object::Intermediate;
    registerNamedEntries(); 
    new_line.trigger();
}

Config::Config(const Config& o) 
: simparm::Object(o),
  simparm::Listener( simparm::Event::ValueChanged ),
  variables(o.variables), simple(o.simple),
  lines(o.lines), new_line(o.new_line), next_ident(o.next_ident)
{ 
    registerNamedEntries(); 
}

Config::~Config() {
    simple.set_manager( NULL );
    for ( boost::ptr_vector< config::CommandLine >::iterator i = lines.begin(); i != lines.end(); ++i )
        i->set_manager( NULL );
}

void Config::registerNamedEntries() {
    simple.set_manager( this );
    for ( boost::ptr_vector< config::CommandLine >::iterator i = lines.begin(); i != lines.end(); ++i )
        i->set_manager( this );
    push_back( new_line );
    receive_changes_from( new_line.value );
}

void Config::operator()(const simparm::Event& e)
{
    if ( new_line.triggered() ) {
        std::stringstream ident;
        ident << next_ident++;
        new_line.untrigger();
        lines.push_back( new config::CommandLine(ident.str(), variables) );
        push_back( lines.back() );
    }
}

bool Config::can_work_with(output::Capabilities)
 { return true; }


}

namespace output {

template <>
std::auto_ptr<OutputSource> make_output_source<expression::Source>()
{
    return std::auto_ptr<OutputSource>( new FilterBuilder<expression::Source>() );
}

}

}
