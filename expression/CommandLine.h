#ifndef DSTORM_EXPRESSION_COMMANDLINE_H
#define DSTORM_EXPRESSION_COMMANDLINE_H

#include "Source_decl.h"
#include <simparm/Object.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ManagedChoiceEntry.hh>
#include "Parser.h"
#include "localization_variable_decl.h"
#include <dStorm/localization/Traits.h>
#include <dStorm/Localization.h>

namespace dStorm {
namespace expression {
class Parser;

namespace config {

struct LValue : public simparm::Object {
    LValue( const std::string& name, const std::string& desc ) : simparm::Object(name, desc) {}
    virtual ~LValue() {}
    LValue* clone() const = 0;
    virtual source::LValue* make_lvalue() const = 0;
    virtual void set_expression_string( const std::string&, Parser& ) = 0;
};

inline LValue* new_clone( const LValue& v ) { return v.clone(); }

struct ExpressionManager {
    virtual ~ExpressionManager() {}
    virtual simparm::Node& getNode() = 0;
    virtual void expression_changed( std::string ident, std::auto_ptr<source::LValue> expression ) = 0;
};

struct CommandLine : public simparm::Object, public simparm::Listener {
    CommandLine( std::string node_ident, boost::shared_ptr<Parser> );
    CommandLine(const CommandLine&);
    CommandLine* clone() const { return new CommandLine(*this); }
    ~CommandLine();

    void set_manager( ExpressionManager* manager );

  private:
    template <int Field>
    inline void add_localization_lvalues();
    template <int Field>
    struct Instantiator;

    void registerNamedEntries();

    typedef simparm::ManagedChoiceEntry< LValue > LValues;
    LValues lvalue;
    simparm::StringEntry expression;
    boost::shared_ptr<Parser> parser;
    ExpressionManager* manager;

    void operator()( const simparm::Event& );
    void publish();
};

}
}
}

#endif
