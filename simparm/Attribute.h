#ifndef SIMPARM_ATTRIBUTE_HH
#define SIMPARM_ATTRIBUTE_HH

#include <string>
#include <stdexcept>
#include <boost/signals2/signal.hpp>
#include "BaseAttribute.h"

namespace boost { template <typename Inner> class optional; }

namespace simparm {

namespace detail {

template <typename Type>
inline bool value_is_optional( const Type& ) 
    { return false; }
template <typename Inner>
inline bool value_is_optional( const boost::optional<Inner>& ) 
    { return true; }

template <typename Type>
inline void unset_value( Type ) 
    { throw std::runtime_error("Non-optional type cannot be unset."); }
template <typename Inner>
inline void unset_value( boost::optional<Inner>& b ) 
    { b.reset(); }

}

template <typename Type>
class Attribute : public BaseAttribute {
    boost::signals2::signal< void () > value_changed;
    std::string ident;

private:
    Type value;

    std::string get_name() const { return ident; }
    boost::optional< std::string > get_value() const;
    bool set_value(const std::string& i );
    void unset_value() { detail::unset_value( value ); }
    bool value_is_optional() const { return detail::value_is_optional( value ); }

    bool valueChange(const Type &to);

  public:
    Attribute(std::string ident, const Type& def_val)
        : ident(ident), value(def_val), change_is_OK(NULL) {}
    Attribute( const Attribute& o ) 
        : ident(o.ident), value(o.value), change_is_OK(NULL) {}
    ~Attribute() {}

    operator const Type&() const { return value; }
    const Type& operator()() const { return value; }

    Attribute& operator=(const Attribute<Type>& o)
        { return ((*this) = o.value); }
    template <typename OtherType>
    Attribute& operator=(const OtherType &o) 
        { valueChange( Type(o) ); return *this; }
    Attribute& operator+=(const Type &o) 
        { valueChange(o + value); return *this; }
    Attribute& operator-=(const Type &o) 
        { valueChange(value - o); return *this; }
    Attribute& operator*=(const Type &o) 
        { valueChange(value * o); return *this; }
    Attribute& operator/=(const Type &o) 
        { valueChange(value / o); return *this; }

    struct ChangeWatchFunction { 
        virtual ~ChangeWatchFunction() {}
        virtual bool operator()(const Type&,const Type&) = 0; 
    };
        
    /* This function will be called before any change to the value of this
     * attribute happens. If it returns false, no change occurs. */
    ChangeWatchFunction *change_is_OK;

    Connection notify_on_value_change( Listener listener )
        { return Connection( new boost::signals2::scoped_connection( value_changed.connect( listener ) ) ); }
};

}

#endif
