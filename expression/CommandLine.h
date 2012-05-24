#ifndef DSTORM_EXPRESSION_COMMANDLINE_H
#define DSTORM_EXPRESSION_COMMANDLINE_H

#include "Source_decl.h"
#include <simparm/Object.h>
#include <simparm/ChoiceEntry_Impl.h>
#include <simparm/ManagedChoiceEntry.h>
#include "Parser.h"
#include "localization_variable_decl.h"
#include <dStorm/localization/Traits.h>
#include <dStorm/Localization.h>
#include <simparm/ObjectChoice.h>

namespace dStorm {
namespace expression {
class Parser;

namespace config {

struct LValue : public simparm::ObjectChoice {
    LValue( const std::string& name, const std::string& desc ) : simparm::ObjectChoice(name, desc) {}
    virtual ~LValue() {}
    LValue* clone() const = 0;
    virtual source::LValue* make_lvalue() const = 0;
    virtual void set_expression_string( const std::string&, Parser& ) = 0;
    void attach_ui( simparm::NodeHandle to ) { attach_parent( to ); }
};

inline LValue* new_clone( const LValue& v ) { return v.clone(); }

struct ExpressionManager {
    virtual ~ExpressionManager() {}
    virtual void expression_changed( std::string ident, std::auto_ptr<source::LValue> expression ) = 0;
};

struct CommandLine {
    CommandLine( std::string node_ident, boost::shared_ptr<Parser> );
    CommandLine* clone() const { return new CommandLine(*this); }
    ~CommandLine();

    void set_manager( ExpressionManager* manager );
    void attach_ui( simparm::NodeHandle at );

  private:
    template <int Field>
    inline void add_localization_lvalues();
    template <int Field>
    struct Instantiator;

    typedef simparm::ManagedChoiceEntry< LValue > LValues;
    simparm::Object disambiguator;
    LValues lvalue;
    simparm::StringEntry expression;
    boost::shared_ptr<Parser> parser;
    ExpressionManager* manager;
    simparm::BaseAttribute::ConnectionStore listening[2];

    void publish();
    void set_expression_string();
};

}
}
}

#endif
