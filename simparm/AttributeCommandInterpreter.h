#ifndef SIMPARM_ATTRIBUTE_COMMAND_INTERPRETER_HH
#define SIMPARM_ATTRIBUTE_COMMAND_INTERPRETER_HH

#include <string>
#include <sstream>
#include <boost/optional/optional.hpp>

namespace simparm {

template <typename Type>
struct AttributeCommandInterpreter {
    static std::string to_string( const Type& t ) {
        std::stringstream ss;
        ss << "set "; to_config_stream(ss, t); 
        return ss.str();
    }
    static bool from_stream( const std::string& command, std::istream& rest, Type& value ) {
        if ( command == "set" ) {
            from_config_stream(rest, value);
            return true;
        } else 
            return false;
    }
};

template <typename Type>
struct AttributeCommandInterpreter< boost::optional<Type> >;

}

#endif
