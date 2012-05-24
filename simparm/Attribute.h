#ifndef SIMPARM_ATTRIBUTE_HH
#define SIMPARM_ATTRIBUTE_HH

#include <string>
#include <sstream>
#include <stdexcept>
#include <typeinfo>

#include <boost/mpl/not.hpp>
#include <boost/signals2/signal.hpp>

#include "iostream.h"
#include "BaseAttribute.h"
#include "AttributeCommandInterpreter.h"

namespace boost { template <typename Type> class optional; }

namespace simparm {

template <typename Type>
class Attribute : public BaseAttribute {
    boost::signals2::signal< void () > value_changed;
    std::string ident;

  protected:
    Type value;

    std::string get_name() const { return ident; }
    std::string get_value() const { return AttributeCommandInterpreter<Type>::to_string(value); }
    void set_value(std::string command, std::istream& i ) { 
        Type temp_value;
        AttributeCommandInterpreter<Type>::from_stream( command, i, temp_value );
        valueChange( temp_value ); 
    }
    void reset_value() { valueChange( Type() ); }

    virtual std::string getTypeDescriptor() const 
        { return "Attribute"; }

    inline void valueChange(const Type &to) {
        if ( change_is_OK == NULL || (*change_is_OK)( value, to ) ) {
            if ( to == value ) {
                /* Do nothing. This is worded with == to avoid NaN
                 * trouble. */
            } else {
                value = to;
                value_changed();
            }
        } else {
            /* Change is NOT ok. Print the correct value to underline this. */
            value_changed();
        }
    }

  public:
    Attribute(std::string ident, const Type& def_val)
        : ident(ident), value(def_val), change_is_OK(NULL) {}
    Attribute( const Attribute& o ) 
        : ident(o.ident), value(o.value), change_is_OK(NULL) {}
    ~Attribute() {}
    virtual Attribute *clone() const 
        { return new Attribute(*this); }

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

    struct ChangeWatchFunction
        { virtual bool operator()(const Type&,const Type&) = 0; };
        
    /* This function will be called before any change to the value of this
     * attribute happens. If it returns false, no change occurs. */
    ChangeWatchFunction *change_is_OK;

    Connection notify_on_value_change( Listener listener )
        { return Connection( new boost::signals2::scoped_connection( value_changed.connect( listener ) ) ); }
};

}

#endif
