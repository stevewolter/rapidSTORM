#ifndef CONFIGOBJECT_HH
#define CONFIGOBJECT_HH

#include <list>
#include <queue>
#include <string>
#include <map>
#include <set>
#include <iostream>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <cassert>
#include <typeinfo>
#include <functional>

namespace simparm {
using std::string;

template <typename T> class ConfigAttribute;

class ConfigNode {
  private:
    bool subtree_is_dag() const throw();
    void check_and_insert_subtree(std::set<const ConfigNode*>* present)
        const throw(std::runtime_error);

  public:
    class Callback : public std::unary_function<ConfigNode&,void> {
        std::multiset<ConfigNode*> attached_to;
        friend class ConfigNode;

      public:
        enum Cause { AddedChild, AddedParent, RemovedChild, RemovedParent, 
                     ValueChanged, ActivityChanged, Other };

        Callback() throw();
        Callback(const Callback&) throw();
        virtual ~Callback();
        Callback& operator=(const Callback&) throw();

        void receive_changes_from(ConfigNode &node) throw()
            { node.addChangeCallback(*this); }

        /** Event receival method. */
        virtual void operator()(ConfigNode& source_of_event,
                                Cause cause,
                                ConfigNode* argument_if_any) = 0;
    };

    typedef std::list<ConfigNode*>::iterator iterator;
    typedef std::list<ConfigNode*>::const_iterator const_iterator;

  protected:
    std::list<ConfigNode*> parents, children;
    std::map<std::string,ConfigNode*> childrenByName;
    std::list<Callback*> changecallbacks;
    std::set<ConfigNode*> managed;

    int _active;
    bool declared;
    std::string *_currentlyDefining;

    /* Deferred callback management - needed when callback signals its own
     * removal. */
    bool currently_iterating_changecallbacks;
    enum CallbackOp { Add, Remove };
    std::queue< std::pair<CallbackOp,Callback*> > deferred_callback_actions;
    void do_deferred_callback_management() throw();
    void manage_callback( CallbackOp action, Callback *argument ) throw();
    
  protected:
    void notifyChangeCallbacks( Callback::Cause cause, ConfigNode *argument_if_any );

    void removeFromAllParents() throw();

    virtual std::string getTypeDescriptor() const
        throw() = 0;

  public:
    ConfigNode() throw();
    ConfigNode(const ConfigNode& o) throw();
    virtual ~ConfigNode();
    virtual ConfigNode *clone() const = 0;

    /** Supplies a name for this node. Every method which overrides this
     *  method must also make sure to call removeFromAllParents() before
     *  its' destructor is finished since the name is needed in
     *  destruction. */
    virtual std::string getName() const throw() = 0;
    virtual std::string getPrefix() const throw() { return "in"; }

    virtual void push_back(ConfigNode& object) throw();
    virtual void erase(ConfigNode& object) throw();
    /** @throw invalid_argument when no ConfigNode with the given name is 
     *         found amongst the direct children. */
    virtual void erase(const std::string& name) 
        throw(std::invalid_argument);
    virtual void clear() throw();

    void make_to_sibling_of(const ConfigNode &other) throw();

    /** Legacy interface for add() */
    virtual void register_entry(ConfigNode* object) throw()
        { if (object != NULL) push_back(*object); }

    virtual bool isActive() throw() { return _active != 0; }
    virtual void setActivity(bool active) throw();

    virtual void print(const std::string& what) throw();

    virtual std::string define() throw();
    virtual std::string undefine() throw();
    virtual std::list<std::string> printValues() const throw();
    virtual void printHelp(std::ostream &) const throw() {}
    virtual void processCommand(std::istream& from) 
        throw(std::invalid_argument);

    void addChangeCallback(Callback& callback) throw();
    void removeChangeCallback(Callback& callback) throw();

    std::list<ConfigNode*>& getChildren() throw() 
        { return children; }
    std::list<ConfigNode*>& getParents() throw() 
        { return parents; }
    const std::list<ConfigNode*>& getChildren() const throw() 
        { return children; }
    const std::list<ConfigNode*>& getParents() const throw() 
        { return parents; }

    /** Destruct o when this object is destructed.*/
    void manage(ConfigNode* o) throw() { managed.insert(o); }
    /** Destruct the content of o when this object is destructed.*/
    void manage(std::auto_ptr<ConfigNode> o) throw() { this->manage(o.release()); }

    const ConfigNode& operator[](const std::string& name) const
        throw(std::invalid_argument);
    ConfigNode& operator[](const std::string& name)
        throw(std::invalid_argument);
    int size() const throw() { return children.size(); }
    std::list<ConfigNode*>::const_iterator begin() const throw()
        { return children.begin(); }
    std::list<ConfigNode*>::const_iterator end() const throw()
        { return children.end(); }

    bool has_child_named(const std::string& name) const throw()
        { return (childrenByName.find(name) != childrenByName.end()); }
};

template <typename InputType>
inline InputType read_value_from_input(std::istream &i) { 
    InputType newVal;
    (i >> newVal);
    return newVal; 
}
template <>
inline std::string read_value_from_input<std::string>(std::istream &i) { 
    std::string rv;
    std::getline(i, rv);
    return rv; 
}
template <>
inline bool read_value_from_input<bool>(std::istream &i) { 
    std::string rv;
    i >> rv;
    if (rv == "true" || rv == "1")
        return true;
    else if (rv == "false" || rv == "0")
        return false;
    else
        throw std::runtime_error("Invalid boolean value " + rv);
}


template <typename Type>
class ConfigAttribute : public ConfigNode {
  protected:
    std::string _name;
    Type value;

    std::string getPrefix() const throw() { return ""; }
    std::string getName() const throw() { return name(); }
    std::string _getValueString(const Type &t) const throw() {
        std::stringstream ss;
        if ( typeid(Type) == typeid(bool) )
            ss << std::boolalpha;
        ss << t;
        return ss.str();
    }

    virtual std::string getTypeDescriptor() const throw() 
        { return "ConfigAttribute"; }

    inline void valueChange(const Type &to) throw() {
        if ( change_is_OK == NULL || (*change_is_OK)( value, to ) ) {
            this->print(_getValueString(to));
            value = to;
            this->notifyChangeCallbacks(Callback::ValueChanged, NULL);
        }
    }

  public:
    ConfigAttribute(string ident, const Type& def_val) throw()
        : ConfigNode(), _name(ident), value(def_val), change_is_OK(NULL) {}
    ~ConfigAttribute() { this->removeFromAllParents(); }
    virtual ConfigAttribute *clone() const 
        { return new ConfigAttribute(*this); }

    operator const Type&() const throw() { return value; }
    const Type& operator()() const throw() { return value; }
    ConfigAttribute& operator=(const ConfigAttribute<Type>& o)
        { return ((*this) = o.value); }
    ConfigAttribute& operator=(const Type &o) 
        { valueChange(o); return *this; }
    ConfigAttribute& operator+=(const Type &o) 
        { valueChange(o + value); return *this; }
    ConfigAttribute& operator-=(const Type &o) 
        { valueChange(value - o); return *this; }

    const std::string& name() const throw() { return _name; }

    std::string define() throw() { 
        return name() + " " + _getValueString(value);
    }

    std::string getValueString() const throw() { return _getValueString(value); }

    void printHelp(std::ostream &) const throw() {}
    void processCommand(std::istream& from) throw() 
        { Type tempValue; from >> tempValue; (*this) = tempValue;  }

    friend std::istream& operator>>
        (std::istream &i, ConfigAttribute<Type>& target)
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

class ConfigObject : public ConfigNode {
  public:
    ConfigObject(string name, string desc) throw();
    ConfigObject(const ConfigObject& o) throw();
    virtual ~ConfigObject();

    ConfigAttribute<std::string> name;
    ConfigAttribute<std::string> desc;

    std::string getTypeDescriptor() const throw()  
        { return "ConfigObject"; }
    std::string getName() const throw() { return name(); }
    void setName(const std::string& to) { name = to; }

    void setDesc(const std::string& new_desc) throw() { desc = new_desc; }
};

class GeneralConfigObject : public ConfigObject {
  private:
    std::string prefix, typeDescriptor;
  protected:
    std::string getPrefix() const throw() { return prefix; }
    std::string getTypeDescriptor() const throw() {return typeDescriptor;}
  public:
    GeneralConfigObject(std::string name, std::string desc,
                      std::string prefix, std::string typeDescriptor) 
    throw()
        : ConfigObject(name,desc),
          prefix(prefix), typeDescriptor(typeDescriptor) {}
    GeneralConfigObject* clone() const throw()
        { return new GeneralConfigObject(*this); }
};


}

#endif
