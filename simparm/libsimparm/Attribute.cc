#include "Attribute.hh"

namespace simparm {

template <>
std::string read_value_from_input<std::string>(std::istream &i) { 
    std::string rv;
    char c;
    i.get(c);   /* eat leading space. */
    if ( i && c != ' ' ) rv = c;
    while ( (i.get(c)) && c != '\n' && c != '\r' ) { rv += c; }
    return rv; 
}

template <> bool read_value_from_input<bool>(std::istream &i) { 
    std::string rv;
    i >> rv;
    if (rv == "true" || rv == "1")
        return true;
    else if (rv == "false" || rv == "0")
        return false;
    else
        throw std::runtime_error("Invalid boolean value " + rv);
}

}
