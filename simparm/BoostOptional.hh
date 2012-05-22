#ifndef SIMPARM_ATTRIBUTE_OPTIONAL_HH
#define SIMPARM_ATTRIBUTE_OPTIONAL_HH

#include "Attribute.hh"
#include <boost/optional/optional.hpp>

#include "MinMaxWatcher_decl.hh"

namespace simparm {

template <typename Type>
struct add_boost_optional { typedef boost::optional<Type> type; };
template <typename Type>
struct add_boost_optional< boost::optional<Type> > { typedef boost::optional<Type> type; };

template <typename Inner>
inline const char *typeName( boost::optional<Inner> ) { return typeName(Inner()); }

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

template <typename Inner>
inline boost::optional<Inner> default_value( const boost::optional<Inner>& ) { 
    return boost::optional<Inner>();
}
template <typename Inner>
inline boost::optional<Inner> default_increment( const boost::optional<Inner>& ) { 
    return boost::optional<Inner>();
}
template <typename Inner>
inline bool exceeds( const boost::optional<Inner>& a, const boost::optional<Inner>& b ) { 
    return a.is_initialized() && b.is_initialized() && exceeds(*a, *b);
}
template <typename Inner>
inline bool exceeds( const Inner& a, const boost::optional<Inner>& b ) { 
    return b.is_initialized() && exceeds(a, *b);
}
template <typename Inner>
inline bool exceeds( const boost::optional<Inner>& a, const Inner& b ) { 
    return a.is_initialized() && exceeds(*a, b);
}

template <typename Inner>
inline bool falls_below( const boost::optional<Inner>& a, const boost::optional<Inner>& b ) { 
    return a.is_initialized() && b.is_initialized() && falls_below(*a, *b);
}
template <typename Inner>
inline bool falls_below( const boost::optional<Inner>& a, const Inner& b ) { 
    return a.is_initialized() && falls_below(*a, b);
}
template <typename Inner>
inline bool falls_below( const Inner& a, const boost::optional<Inner>& b ) { 
    return b.is_initialized() && falls_below(a, *b);
}

}

#include "Attributes.hh"

namespace simparm {

template <typename Inner, typename Field>
struct Attributes< boost::optional<Inner>, Field, void >
: public Attributes< Inner, Field, void >
{
    typedef Attributes<Inner,Field,void> Base;
    Attributes( Attribute<Field>& e ) 
        : Base(e), is_optional("is_optional", true ) {}
    Attributes( const Attributes& a, Attribute<Field>& e ) 
        : Base(a, e), is_optional(a.is_optional) {}
    void registerNamedEntries( simparm::Node& n ) {
        n.add_attribute( is_optional );
        Base::registerNamedEntries(n);
    }
  private:
   Attribute<bool> is_optional;
};

}

#endif
