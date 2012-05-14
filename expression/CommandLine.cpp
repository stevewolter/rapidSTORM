#include "CommandLine.h"
#include "Filter.h"
#include "VariableLValue.h"
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {
namespace expression {
namespace config {

CommandLine::CommandLine( std::string ident, boost::shared_ptr<Parser> parser )
: simparm::Object("CommandLine" + ident, "Command line") ,
  simparm::Listener( simparm::Event::ValueChanged ),
    lvalue("LValue", "Value to assign to"),
    expression("Expression", "Expression to assign from"),
    parser(parser),
    manager(NULL)
{
    lvalue.userLevel = simparm::Object::Intermediate;
    expression.userLevel = simparm::Object::Intermediate;

    expression.helpID = "#ExpressionFilter_Expression";
    lvalue.helpID = "#ExpressionFilter_LValue";

    lvalue.addChoice( make_filter() );
    const VariableTable& variables = parser->get_variable_table();
    for ( variable_table::const_iterator i = variables.begin(); i != variables.end(); ++i )
        lvalue.addChoice( make_variable_lvalue(*i) );
    registerNamedEntries();

}

CommandLine::~CommandLine() {
    stop_receiving_changes_from( lvalue.value );
    manager = NULL;
}

void CommandLine::operator()( const simparm::Event& )
{
    if ( expression() != "" )
        lvalue().set_expression_string( expression(), *parser );
    publish();
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
            e.reset( lvalue().make_lvalue() );
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
    parser(o.parser), manager(o.manager)
{
    registerNamedEntries();
}

}
}
}

