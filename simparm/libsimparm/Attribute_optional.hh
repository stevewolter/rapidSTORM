#ifndef SIMPARM_ATTRIBUTE_OPTIONAL_HH
#define SIMPARM_ATTRIBUTE_OPTIONAL_HH

#include "Attribute.hh"
#include <boost/optional/optional.hpp>

namespace simparm {

template <typename _Type>
struct AttributeCommandInterpreter< boost::optional<_Type> > {
    typedef boost::optional<_Type> Type;
    static std::string to_string( const Type& t ) {
        std::stringstream ss;
        if ( t.is_initialized() ) {
            ss << "set "; to_config_stream(ss, *t); 
        } else 
            ss << "unset";
        return ss.str();
    }
    static bool from_stream( const std::string& command, std::istream& rest, Type& value ) {
        if ( command == "set" ) {
            _Type t;
            from_config_stream(rest, t);
            value = t;
            return true;
        } else if ( command == "unset" ) {
            value = Type();
            return true;
        } else 
            return false;
    }
};

}

#endif
