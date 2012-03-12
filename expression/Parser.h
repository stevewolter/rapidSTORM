#ifndef DSTORM_EXPRESSION_PARSER_H
#define DSTORM_EXPRESSION_PARSER_H

#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>

namespace dStorm {
namespace expression {

class AbstractSyntaxTree;
class VariableTable;
class Variable;

class Parser {
    struct Pimpl;
    boost::shared_ptr<Pimpl> pimpl;
public:
    Parser();
    boost::shared_ptr<AbstractSyntaxTree> parse_boolean( const std::string& ) const;
    boost::shared_ptr<AbstractSyntaxTree> parse_numeric( const std::string& ) const;

    VariableTable& get_variable_table();
    const VariableTable& get_variable_table() const;
    void add_variable( std::auto_ptr<Variable> );
};

}
}

#endif
