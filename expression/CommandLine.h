#ifndef DSTORM_EXPRESSION_COMMANDLINE_H
#define DSTORM_EXPRESSION_COMMANDLINE_H

#include "expression/Source_decl.h"
#include "simparm/Object.h"
#include "simparm/ManagedChoiceEntry.h"
#include "expression/Parser.h"
#include "expression/localization_variable_decl.h"
#include "localization/Traits.h"
#include "Localization.h"
#include "simparm/ObjectChoice.h"

namespace dStorm {
namespace expression {
class Parser;

namespace config {

class LValue : public simparm::ObjectChoice {
  public:
    LValue( const std::string& name, const std::string& desc ) : simparm::ObjectChoice(name, desc) {}
    virtual ~LValue() {}
    LValue* clone() const = 0;
    virtual source::LValue* make_lvalue() const = 0;
    virtual void set_expression_string( const std::string&, Parser& ) = 0;
    void attach_ui( simparm::NodeHandle to ) { attach_parent( to ); }
};

inline LValue* new_clone( const LValue& v ) { return v.clone(); }

class ExpressionManager {
  public:
    virtual ~ExpressionManager() {}
    virtual void expression_changed( std::string ident, std::auto_ptr<source::LValue> expression ) = 0;
};

class CommandLine {
  public:
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
