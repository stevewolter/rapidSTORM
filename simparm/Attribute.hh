#ifndef SIMPARM_ATTRIBUTE_HH
#define SIMPARM_ATTRIBUTE_HH

#include <string>
#include <sstream>
#include <stdexcept>
#include <typeinfo>

#include <boost/mpl/not.hpp>
#include <boost/signals2/signal.hpp>

#include "iostream.hh"
#include "BaseAttribute.hh"
#include "Callback.hh"

namespace boost { template <typename Type> class optional; }

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

template <typename Type>
class Attribute : public BaseAttribute, public Publisher {
  public:
    static Type read_value(std::istream& i) 
        { Type t; from_config_stream(i, t); return t; }
    static std::string stringify(const Type& t) 
        { std::stringstream ss; to_config_stream(ss, t); return ss.str(); }
  private:
    boost::signals2::signal< void (const std::string&) > print;
  protected:
    Type value;

    virtual std::string getTypeDescriptor() const 
        { return "Attribute"; }

    inline void valueChange(const Type &to, bool print = true) {
        if ( change_is_OK == NULL || (*change_is_OK)( value, to ) ) {
            if ( to == value ) {
                /* Do nothing. This is worded with == to avoid NaN
                 * trouble. */
            } else {
                if ( print ) this->print(AttributeCommandInterpreter<Type>::to_string(to));
                value = to;
                this->notifyChangeCallbacks(Event::ValueChanged, NULL);
            }
        } else {
            /* Change is NOT ok. Print the correct value to underline this. */
            this->print(AttributeCommandInterpreter<Type>::to_string(value));
        }
    }

  public:
    Attribute(std::string ident, const Type& def_val)
        : value(def_val), change_is_OK(NULL) {}
    Attribute( const Attribute& );
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

    std::string define() { 
        return this->name + " " + AttributeCommandInterpreter<Type>::to_string(value);
    }

    void printHelp(std::ostream &) const {}
    void processCommand(std::istream& from) {
        std::string command;
        from >> command;
        
        Type value;
        if ( AttributeCommandInterpreter<Type>::from_stream(command, from, value) ) {
            valueChange( value, false );
        } else if ( command == "query" )
            print( define() );
        else
            throw std::runtime_error("Unrecognized command " + command);
    }

    struct ChangeWatchFunction
        { virtual bool operator()(const Type&,const Type&) = 0; };
        
    /* This function will be called before any change to the value of this
     * attribute happens. If it returns false, no change occurs. */
    ChangeWatchFunction *change_is_OK;
};

}

#endif
