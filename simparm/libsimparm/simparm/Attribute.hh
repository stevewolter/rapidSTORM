#ifndef SIMPARM_ATTRIBUTE_HH
#define SIMPARM_ATTRIBUTE_HH

#include <simparm/Node.hh>

#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <typeinfo>

namespace simparm {

template <typename InputType>
InputType read_value_from_input(std::istream &i) { 
    InputType newVal;
    (i >> newVal);
    return newVal; 
}
template <> std::string read_value_from_input<std::string>(std::istream &i); 
template <> bool read_value_from_input<bool>(std::istream &i); 

template <typename Type>
class Attribute : public Node {
  protected:
    std::string _name;
    Type value;

    std::string getPrefix() const { return ""; }
    std::string getName() const { return name(); }
    std::string _getValueString(const Type &t) const {
        std::stringstream ss;
        if ( typeid(Type) == typeid(bool) )
            ss << std::boolalpha;
        ss << t;
        return ss.str();
    }

    virtual std::string getTypeDescriptor() const 
        { return "Attribute"; }

    inline void valueChange(const Type &to) {
        if ( change_is_OK == NULL || (*change_is_OK)( value, to ) ) {
            if ( to != value ) {
                this->print(_getValueString(to));
                value = to;
                this->notifyChangeCallbacks(Callback::ValueChanged, NULL);
            }
        }
    }

  public:
    Attribute(std::string ident, const Type& def_val)
        : Node(), _name(ident), value(def_val), change_is_OK(NULL) {}
    ~Attribute() { this->removeFromAllParents(); }
    virtual Attribute *clone() const 
        { return new Attribute(*this); }

    operator const Type&() const { return value; }
    const Type& operator()() const { return value; }
    Attribute& operator=(const Attribute<Type>& o)
        { return ((*this) = o.value); }
    Attribute& operator=(const Type &o) 
        { valueChange(o); return *this; }
    Attribute& operator+=(const Type &o) 
        { valueChange(o + value); return *this; }
    Attribute& operator-=(const Type &o) 
        { valueChange(value - o); return *this; }

    const std::string& name() const { return _name; }

    std::string define() { 
        return name() + " " + _getValueString(value);
    }

    std::string getValueString() const { return _getValueString(value); }

    void printHelp(std::ostream &) const {}
    void processCommand(std::istream& from) 
        { (*this) = read_value_from_input<Type>(from); }

    friend std::istream& operator>>
        (std::istream &i, Attribute<Type>& target)
    {
        target = read_value_from_input<Type>(i);
        return i;
    }

    struct ChangeWatchFunction
        { virtual bool operator()(const Type&,const Type&) = 0; };
        
    /* This function will be called before any change to the value of this
     * attribute happens. If it returns false, no change occurs. */
    ChangeWatchFunction *change_is_OK;
};

template <typename Type>
class Attribute<Type*> : public Node {
  public:
    struct NameMap { 
        virtual Type* getElement( const std::string& name ) = 0; 
        virtual std::string getNameForElement( Type *element )  = 0; 
    };
  protected:
    std::string _name;
    Type* value;
    NameMap* nameMap;

    std::string getPrefix() const { return ""; }
    std::string getName() const { return name(); }
    std::string _getValueString(Type *t) const {
        std::stringstream ss;
        ss << ((t == NULL) ? std::string("") 
                          : nameMap->getNameForElement( t ));
        return ss.str();
    }

    virtual std::string getTypeDescriptor() const 
        { return "Attribute"; }

    inline void valueChange(Type *to) {
        if ( change_is_OK == NULL || (*change_is_OK)( value, to ) ) {
            if ( to != value ) {
                this->print(_getValueString(to));
                value = to;
                this->notifyChangeCallbacks(Callback::ValueChanged, NULL);
            }
        }
    }

  public:
    Attribute(std::string ident, NameMap& nameMap)
        : Node(), _name(ident), value(NULL),
          nameMap(&nameMap), change_is_OK(NULL) {}
    ~Attribute() { this->removeFromAllParents(); }
    virtual Attribute *clone() const 
        { return new Attribute(*this); }

    operator const Type&() const { return *value; }
    const Type& operator()() const { return *value; }
    Type& operator()() { return *value; }
    bool hasValue() const { return value != NULL; }

    Attribute& operator=(const Attribute<Type*>& o)
        { return ((*this) = o.value); }
    Attribute& operator=(Type &o) 
        { valueChange(&o); return *this; }
    Attribute& operator=(Type *o) 
        { valueChange(o); return *this; }

    const std::string& name() const { return _name; }

    std::string define() 
        { return name() + " " + _getValueString(value); }

    std::string getValueString() const 
        { return _getValueString(value); }

    void printHelp(std::ostream &) const {}
    void processCommand(std::istream& from) {
        std::string description = 
            read_value_from_input<std::string>(from);
        if  (description == "")
            (*this) = NULL;
        else
            (*this) = nameMap->getElement( description );
    }

    /** Change the current name mapping function.
     *  @return A reference to the old name mapping function. */
    NameMap& changeNameMap( NameMap& newMap ) {
        NameMap* oldMap = &newMap;
        std::swap(nameMap, oldMap);
        return *oldMap;
    }

    friend std::istream& operator>>
        (std::istream &i, Attribute<Type*>& target)
    {
        target->processCommand( i );
        return i;
    }

    struct ChangeWatchFunction
        { virtual bool operator()(const Type*,const Type*) = 0; };
        
    /* This function will be called before any change to the value of this
     * attribute happens. If it returns false, no change occurs. */
    ChangeWatchFunction *change_is_OK;
};
}

#endif
