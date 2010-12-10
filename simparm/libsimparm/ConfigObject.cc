#define NDEBUG

#include "ConfigObject.hh"
#include <algorithm>
#include <numeric>
#include <functional>
#include <cassert>

#include "stl_helpers.hh"

using namespace std;

#ifdef NDEBUG
#define DEBUG_ONLY(x)
#endif
#ifndef NDEBUG
#define DEBUG_ONLY(x) x
#endif

#ifndef NDEBUG
static std::set<const simparm::ConfigNode*> present;
static std::multimap<const simparm::ConfigNode*,std::string> deleted;
static bool check_node_validity(const simparm::ConfigNode* node) {
    if ( node == NULL || ::present.find( node ) == ::present.end() ) {
        std::stringstream ss;
        ss << "Encountered invalid pointer " << node << " to a ConfigNode" ;
        if ( deleted.find( node ) != deleted.end() ) {
            ss << ". Node is deleted and was named: " ;
            for (std::multimap<const simparm::ConfigNode*,std::string>::const_iterator
                i = deleted.find( node ); i->first == node; i++)
                ss << i->second << " ";
        }
        cerr << ss.str();
        return false;
    }
    return true;
}
#endif


namespace simparm {
template <typename Ty> 
void destruct(Ty* p) { delete p; }

ConfigNode::Callback::Callback() throw() {}
ConfigNode::Callback::Callback(const Callback& o) throw() 
{
    for (std::multiset<ConfigNode*>::const_iterator i = 
                o.attached_to.begin(); i != o.attached_to.end(); i++)
        (*i)->addChangeCallback(*this);
}
ConfigNode::Callback::~Callback() {
    while ( ! attached_to.empty() )
        (*attached_to.begin())->removeChangeCallback(*this);
}
ConfigNode::Callback& 
ConfigNode::Callback::operator=(const ConfigNode::Callback& o) throw() {
    while ( ! attached_to.empty() )
        (*attached_to.begin())->removeChangeCallback(*this);

    for (std::multiset<ConfigNode*>::const_iterator i = 
                o.attached_to.begin(); i != o.attached_to.end(); i++)
        (*i)->addChangeCallback(*this);
    return *this;
}

ConfigNode::ConfigNode() throw() 
    : _active(0), declared(false), _currentlyDefining(NULL),
      currently_iterating_changecallbacks(false)
    { DEBUG_ONLY( present.insert(this) ); }
ConfigNode::ConfigNode(const ConfigNode &) throw() 
    : _active(0), declared(false), _currentlyDefining(NULL),
      currently_iterating_changecallbacks(false)
    { DEBUG_ONLY( present.insert(this) ); }
ConfigNode::~ConfigNode() {
    assert( check_node_validity(this) );
    assert( parents.empty() );
    assert( subtree_is_dag() );

    while ( ! changecallbacks.empty() )
        removeChangeCallback( *changecallbacks.front() );

    while (! children.empty() ) {
        erase(*children.front());
    }

    list<ConfigNode*> copy_of_managed;
    copy( managed.begin(), managed.end(), 
            (copy_of_managed.begin()) );
    for_each (copy_of_managed.begin(), copy_of_managed.end(),
                destruct<ConfigNode> );

    DEBUG_ONLY( present.erase( this ) );
}

ConfigObject::ConfigObject(string name, string desc) throw()
: ConfigNode(), name("name", name), desc("desc", desc)
{
    push_back(this->name);
    push_back(this->desc);
}
ConfigObject::ConfigObject(const ConfigObject& o) throw()
: ConfigNode(o), name(o.name), desc(o.desc) 
{
    push_back(name);
    push_back(desc);
}
ConfigObject::~ConfigObject() { 
    DEBUG_ONLY( deleted.insert( make_pair(this, name()) ) );
    removeFromAllParents(); 
}

void ConfigNode::push_back(ConfigNode& object) throw()
{ 
    assert( subtree_is_dag() );
    object.parents.push_back( this );
    children.push_back( &object );
    childrenByName[object.getName()] = &object;

    if (isActive()) { 
        object.setActivity(true);
        print( object.define() );
    }

    object.notifyChangeCallbacks( Callback::AddedParent, this );
    this->notifyChangeCallbacks( Callback::AddedChild, &object );

    assert( subtree_is_dag() );
}
void ConfigNode::erase(ConfigNode& object) throw()
{
    assert( subtree_is_dag() );
    if (isActive()) { 
        print( object.undefine() ); 
        object.setActivity(false); 
    }
    nstd::erase( children, &object );
    nstd::erase( object.parents, this );
    childrenByName.erase( object.getName() );

    object.notifyChangeCallbacks( Callback::RemovedParent, this );
    this->notifyChangeCallbacks( Callback::RemovedChild, &object );

    if ( managed.erase( &object ) > 0 )
        delete &object;
    assert( subtree_is_dag() );
}
void ConfigNode::erase(const std::string& name)
throw(std::invalid_argument) 
    { erase( (*this)[name] ); }

void ConfigNode::clear() throw() {
    while ( ! children.empty() )
        erase( *children.front() );
}

void ConfigNode::removeFromAllParents() throw() {
    while ( ! parents.empty() )
        parents.front()->erase(*this);
}

void ConfigNode::make_to_sibling_of(const ConfigNode &other) throw() {
    nstd::for_each( other.parents, 
        bind2nd(mem_fun(&ConfigNode::register_entry), this) );
}

void ConfigNode::setActivity(bool active) throw() {
    bool wasActive = isActive();
    if ( active ) _active++; else _active--;
    assert( _active >= 0 );
    if ( wasActive != isActive() ) {
        nstd::for_each( children,
            bind2nd( mem_fun(&ConfigNode::setActivity), isActive() ) );
        notifyChangeCallbacks( Callback::ActivityChanged, NULL );
    }
}

void ConfigNode::print(const std::string& what) throw() {
    if (_currentlyDefining) {
        (*_currentlyDefining) += "\n" + what;
    } else if (isActive())
        nstd::for_each( parents, bind2nd(mem_fun( &ConfigNode::print ),
                    getPrefix() + getName() + " " + what) );
}
std::string ConfigNode::define() throw() {
    std::string definition = "declare " + getTypeDescriptor();
    list<string> child_defs;
    transform( children.begin(), children.end(), 
                back_inserter(child_defs),
                mem_fun(&ConfigNode::define) );
    for ( list<string>::iterator i = child_defs.begin(); 
                                    i != child_defs.end(); i++)
        if ( *i != "" )
            definition += "\n" + *i;
    definition += "\nend";
    return definition;
}
std::string ConfigNode::undefine() throw() {
    return "remove " + getName(); 
}

struct CallbackInfo {
    ConfigNode &source;
    ConfigNode::Callback::Cause cause;
    ConfigNode *argument;
    CallbackInfo(ConfigNode &s, ConfigNode::Callback::Cause c, ConfigNode *a)
        : source(s), cause(c), argument(a) {}
};
static void notifyChangeCallback
    ( ConfigNode::Callback* callback, CallbackInfo& info )
    { (*callback)( info.source, info.cause, info.argument ); }

class TrueWhileLiving {
    bool &var;

  public:
    TrueWhileLiving(bool &var) throw() : var(var) { var = true; }
    ~TrueWhileLiving() throw()                    { var = false; }
};

void ConfigNode::notifyChangeCallbacks(ConfigNode::Callback::Cause cause,
    ConfigNode *a) 
{
    assert( subtree_is_dag() );

    CallbackInfo info( *this, cause, a );
    TrueWhileLiving twl( currently_iterating_changecallbacks );
    nstd::for_each( changecallbacks,
        bind2nd( ptr_fun(notifyChangeCallback), info) );
}

void ConfigNode::manage_callback( CallbackOp action, Callback *argument ) throw() {
    assert( check_node_validity(this) );
    assert( argument != NULL );

    if ( action == Add ) {
        changecallbacks.push_back( argument );
        argument->attached_to.insert(this);
    } else if (action == Remove) {
        nstd::erase( changecallbacks, argument );
        argument->attached_to.erase( this );
    }
}

void ConfigNode::do_deferred_callback_management() throw() {
    while ( deferred_callback_actions.size() > 0 ) {
        manage_callback( deferred_callback_actions.front().first, 
                         deferred_callback_actions.front().second );
        deferred_callback_actions.pop();
    }
}

void ConfigNode::addChangeCallback(Callback& callback) throw() {
    if ( currently_iterating_changecallbacks )
        deferred_callback_actions.push( make_pair(Add, &callback) );
    else
        manage_callback( Add, &callback );
}

void ConfigNode::removeChangeCallback(Callback& callback) throw() {
    if ( currently_iterating_changecallbacks )
        deferred_callback_actions.push( make_pair(Remove, &callback) );
    else
        manage_callback( Remove, &callback );
}

const ConfigNode& ConfigNode::operator[](const string& name) const 
    throw(std::invalid_argument)
{
    std::map<std::string,ConfigNode*>::const_iterator i 
        = childrenByName.find(name);

    if (i == childrenByName.end())
        throw std::invalid_argument("No config node named " + name);
    else 
        return *i->second;
}

ConfigNode& ConfigNode::operator[](const string& name)
    throw(std::invalid_argument)
{
    std::map<std::string,ConfigNode*>::const_iterator i 
        = childrenByName.find(name);

    if (i == childrenByName.end())
        throw std::invalid_argument("No config node named " + name);
    else 
        return *i->second;
}

void ConfigNode::processCommand(istream &in) 
    throw(std::invalid_argument) 
{
    string cmd, name;
    in >> cmd;

    if ( cmd == "set" || cmd == "forSet" || cmd == "in" ) {
        in >> name;
    } else
        name = cmd;

    if (name == "=") name = "value";
    try {
        (*this)[name].processCommand(in);
    } catch (const std::invalid_argument& e) {
        throw std::invalid_argument(
            "Unknown command: \"" + cmd + "\"");
    }
}

bool ConfigNode::subtree_is_dag() const throw() {
    try {
        std::set<const ConfigNode*> store;
        check_and_insert_subtree( &store );
        return true;
    } catch (const std::runtime_error& e) {
        cerr << e.what() << endl;
        return false;
    }
}

void ConfigNode::check_and_insert_subtree
    (std::set<const ConfigNode*>* present) const throw(std::runtime_error) 
{
    if ( ! present->insert(this).second ) {
        stringstream ss;
        ss << "Entry " << this << " (" << getName() <<
              ") is part of cycle in config graph.\n";
        for (std::list<ConfigNode*>::const_iterator i = begin(); 
             i != end(); i++)
            ss << "Direct child " << (*i)->getName() << endl;

        throw std::runtime_error(ss.str());
    }
    try {
        for_each( begin(), end(), 
            bind2nd(mem_fun(&ConfigNode::check_and_insert_subtree), present) );
    } catch (const std::exception &e) {
        std::string myName = getName();
        std::string desc = e.what();
        desc = "Parent " + myName + "\n" + desc;
        throw std::runtime_error(desc);
    }
    present->erase(this);
}

template <typename Ty>
class ListAccumulator : public std::list<Ty>,
    public std::unary_function<std::list<Ty>&,void> 
{
  public:
    void operator()(std::list<Ty> &b) throw() { this->splice( this->end(), b ); }
};

std::list<std::string> ConfigNode::printValues() const throw() {
    list< list<string> > pre_results;
    ListAccumulator<string> results;

    std::transform( begin(), end(), back_inserter(pre_results), 
        mem_fun( &ConfigNode::printValues ) );
    results = std::for_each( pre_results.begin(), pre_results.end(), 
                       ListAccumulator<std::string>() );

    if ( false && results.size() > 2 )
        ;
    else {
        string prefix = getName() + ".";
        /* Prepend the prefix to all results */
        std::transform( results.begin(), results.end(), results.begin(),
            std::bind1st( std::plus<string>(), prefix ) );
    }

    return results;
}

};
