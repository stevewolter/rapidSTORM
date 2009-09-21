#include "Node.hh"
#include <list>
#include <set>
#include <map>
#include <queue>
#include <string>
#include <sstream>
#include <iostream>
#include <functional>
#include <algorithm>
#include <cassert>
#include <stl_helpers.hh>
#include "Object.hh"

#ifdef NDEBUG
#define DEBUG_ONLY(x)
#endif
#ifndef NDEBUG
#define DEBUG_ONLY(x) x
#include <pthread.h>
#endif

using namespace std;

namespace simparm {

#ifndef NDEBUG
static pthread_mutex_t strucMut = PTHREAD_MUTEX_INITIALIZER;
static std::set<const simparm::Node*> present;
static std::multimap<const simparm::Node*,std::string> names;
static std::map<const Node*, pthread_t> belongers;

static bool check_node_validity(const simparm::Node* node)
 {
    pthread_mutex_lock(&strucMut);
    if ( node == NULL || present.find( node ) == present.end() ) {
        std::stringstream ss;
        ss << "Encountered invalid pointer " << node << " to a Node" ;
        if ( names.find( node ) != names.end() ) {
            ss << ". Node is deleted and was named: " ;
            for (std::multimap<const simparm::Node*,std::string>::const_iterator
                i = names.find( node ); i->first == node; i++)
                ss << i->second << " ";
        }
        pthread_mutex_unlock(&strucMut);
        throw std::runtime_error(ss.str());
    } else if ( belongers.find(node) != belongers.end() &&
#ifdef PTW32_VERSION
                belongers[node].p != pthread_self().p )
#else
                belongers[node] != pthread_self() )
#endif
    {
        pthread_mutex_unlock(&strucMut);
        stringstream ss;
        ss << "Node access violation for node " << node << " with name "
           << node->getName() << "\n.";
        const std::list<Node*>& parents = node->getParents();
        for (std::list<Node*>::const_iterator i = parents.begin();
             i != parents.end(); i++)
            ss << "Parent " << (*i) << " (" << (*i)->getName() << ")\n";
        throw std::runtime_error(ss.str());
                                 
    }
    pthread_mutex_unlock(&strucMut);
    return true;
}

void print_node_debug_info( const Node *list, std::string prefix )
{
    try {
        check_node_validity( list );
        cerr << prefix << list << " " << list->getName() << "\n";
        for ( std::list<Node*>::const_iterator i = list->begin(); i != list->end(); i++)
            print_node_debug_info( *i, prefix + "  ");
    } catch (const std::exception& e) {
        cerr << prefix << "Node is invalid: " << e.what() << "\n";
    }
}

void print_debug_info( const Node* list ) {
    print_node_debug_info( list, "" );
}

void collect_subtree( const Node *a, std::set<const Node*>* into )
{
    try {
        check_node_validity( a );
        if ( into->find(a) != into->end() ) {
            into->insert( a );
            std::for_each( a->begin(), a->end(),
                bind2nd( ptr_fun(&collect_subtree), into ) );
        }
    } catch (const std::exception &e) {}
}

bool subtrees_are_distinct( const Node *a, const Node *b ) {
    std::set<const Node *> forA, forB;
    std::list<const Node *> intersect;
    collect_subtree( a, &forA );
    collect_subtree( b, &forB );

    std::set_intersection( forA.begin(), forA.end(), forB.begin(), forB.end(),
                           back_inserter(intersect) );
    return ( intersect.size() == 0 );
}

void check_and_insert_subtree(const Node *orig, const Node *node,
                              std::set<const Node*>* present)

{
    check_node_validity(node);

    if ( ! present->insert(node).second ) {
        stringstream ss;
        ss << "Entry " << node << " (" << node->getName() <<
              ") is part of cycle in config graph.\n";

        throw std::runtime_error(ss.str());
    }
    try {
        pthread_mutex_lock(&strucMut);
        pthread_mutex_unlock(&strucMut);
        for ( std::list<Node*>::const_iterator i = node->begin();
              i != node->end(); i++)
        {
            check_and_insert_subtree( orig, *i, present );
        }
        pthread_mutex_lock(&strucMut);
        pthread_mutex_unlock(&strucMut);
    } catch (const std::exception &e) {
        stringstream addline;
        addline << "Parent " << node << "(" << node->getName() << ")\n" << e.what();
        throw std::runtime_error(addline.str());
    }
    present->erase(node);
}

void announce_new_node( Node *node, std::string name ) {
    pthread_mutex_lock( &strucMut );
    present.insert( node );
    names.insert( make_pair(node, name) );
    pthread_mutex_unlock( &strucMut );
}

void announce_dead_node( Node *node ) {
    pthread_mutex_lock( &strucMut );
    present.erase( node );
    belongers.erase( node );
    pthread_mutex_unlock( &strucMut );
}

static void my_tree_recursive( const Node *node ) {
    belongers.insert( make_pair(node, pthread_self()) );
    for_each (node->begin(), node->end(), ptr_fun(&my_tree_recursive));
}
void my_tree (const Node *node) {
    pthread_mutex_lock( &strucMut );
    my_tree_recursive( node );
    pthread_mutex_unlock( &strucMut );
}

#endif


struct Node::Data {
    std::map<std::string,Node*> childrenByName;
    std::set<Node*> managed;
    std::queue< std::pair<CallbackOp,Callback*> > deferred_callback_actions;
};

struct Node::Callback::Data {
    std::multiset<Node*> attached_to;
};

template <typename Ty> 
void destruct(Ty* p) { delete p; }

Node::Callback::Callback() : data(new Node::Callback::Data()) {}
Node::Callback::Callback(const Callback& o) 
    : data(new Node::Callback::Data())
{
    for (std::multiset<Node*>::const_iterator 
            i = o.data->attached_to.begin();
            i != o.data->attached_to.end(); i++)
        (*i)->addChangeCallback(*this);
}
Node::Callback::~Callback() {
    while ( ! data->attached_to.empty() )
        (*data->attached_to.begin())->removeChangeCallback(*this);
    delete data;
}
Node::Callback& 
Node::Callback::operator=(const Node::Callback& o) {
    while ( ! data->attached_to.empty() )
        (*data->attached_to.begin())->removeChangeCallback(*this);

    for (std::multiset<Node*>::const_iterator 
            i = o.data->attached_to.begin(); 
            i != o.data->attached_to.end(); i++)
        (*i)->addChangeCallback(*this);
    return *this;
}

Node::Node() 
    : data(new Data()),
      _active(0), declared(false), _currentlyDefining(NULL),
      currently_iterating_changecallbacks(false)
{ DEBUG_ONLY( announce_new_node( this, "" ) ); }
Node::Node(const Node &) 
: data(new Data()),
    _active(0), declared(false), _currentlyDefining(NULL),
    currently_iterating_changecallbacks(false)
{ DEBUG_ONLY( announce_new_node( this, "copy" ) ); }
Node::~Node() {
    assert( subtree_is_dag() );
    assert( parents.empty() );

    while ( ! changecallbacks.empty() )
        removeChangeCallback( *changecallbacks.front() );

    while (! children.empty() ) {
        erase(*children.front());
    }

    list<Node*> copy_of_managed;
    copy( data->managed.begin(), data->managed.end(), 
            back_inserter(copy_of_managed) );
    for_each (copy_of_managed.begin(), copy_of_managed.end(),
                destruct<Node> );

    delete data;

    DEBUG_ONLY( announce_dead_node(this) );
}

void Node::push_front(Node& object) 
{
    add_object( object, true ) ;
}

void Node::push_back(Node& object) 
{
    add_object( object, false ) ;
}

void Node::add_object(Node& object, bool front)
{ 
    assert( subtree_is_dag() );
    assert( check_node_validity(&object) );

    std::string childName = object.getName();

    if ( childName != "" && 
         data->childrenByName.find(childName) != 
         data->childrenByName.end() )
        throw std::runtime_error(
            "Name conflict for name '" + childName + "' in '" + getName() 
            + "'");

    object.parents.push_back( this );
    if ( front )
        children.push_front( &object );
    else
        children.push_back( &object );

    data->childrenByName[object.getName()] = &object;

    if (isActive()) { 
        object.setActivity(true);
        print( object.define() );
    }

    object.notifyChangeCallbacks( Callback::AddedParent, this );
    this->notifyChangeCallbacks( Callback::AddedChild, &object );

    assert( subtree_is_dag() );
}
void Node::erase(Node& object)
{
    assert( subtree_is_dag() );
    assert( check_node_validity(&object) );

    if ( std::find( children.begin(), children.end(), &object )
         == children.end() ) return;

    std::string undefinition;
    if (isActive()) { 
        undefinition = object.undefine(); 
    }
    nstd::erase( children, &object );
    data->childrenByName.erase( object.getName() );

    object.notifyChangeCallbacks( Callback::RemovedParent, this );
    this->notifyChangeCallbacks( Callback::RemovedChild, &object );

    if ( data->managed.erase( &object ) > 0 )
        delete &object;
    else
        nstd::erase( object.parents, this );

    if ( isActive() ) 
        print( undefinition );
    assert( subtree_is_dag() );
}
void Node::erase(const std::string& name)
 
    { erase( (*this)[name] ); }

void Node::clear() {
    while ( ! children.empty() )
        erase( *children.front() );
}

void Node::removeFromAllParents() {
    while ( ! parents.empty() ) {
        Node *parent = parents.front();
        parent->erase(*this);
        if ( parent == parents.front() )
            parents.pop_front();
    }
    removeAllChildren();
}

void Node::removeAllChildren() {
    while ( ! children.empty() )
        erase( *children.front() );
}

void Node::make_to_sibling_of(const Node &other) {
    nstd::for_each( other.parents, 
        bind2nd(mem_fun(&Node::register_entry), this) );
}

void Node::setActivity(bool active) {
    bool wasActive = isActive();
    if ( active ) _active++; else _active--;
    assert( _active >= 0 );
    if ( wasActive != isActive() ) {
        nstd::for_each( children,
            bind2nd( mem_fun(&Node::setActivity), isActive() ) );
        notifyChangeCallbacks( Callback::ActivityChanged, NULL );
    }
}

void Node::print(const std::string& what) {
    if (_currentlyDefining) {
        (*_currentlyDefining) += "\n" + what;
    } else if (isActive() && !parents.empty()) {
        string message = getPrefix();
        if (message.size() > 0 && 
            message[ message.size()-1 ] != ' ') message += " ";
        message += getName() + " " + what;
        std::list<Node*>::iterator i;
        for ( i = parents.begin(); i != parents.end(); i++)
            (*i)->print( message );
    } 
}
std::string Node::define() {
    std::string definition = "declare " + getTypeDescriptor();
    list<string> child_defs;
    transform( children.begin(), children.end(), 
                back_inserter(child_defs),
                mem_fun(&Node::define) );
    for ( list<string>::iterator i = child_defs.begin(); 
                                    i != child_defs.end(); i++)
        if ( *i != "" )
            definition += "\n" + *i;
    definition += "\nend";
    return definition;
}
std::string Node::undefine() {
    return "remove " + getName(); 
}

struct CallbackInfo {
    Node &source;
    Node::Callback::Cause cause;
    Node *argument;
    CallbackInfo(Node &s, Node::Callback::Cause c, Node *a)
        : source(s), cause(c), argument(a) {}
};
static void notifyChangeCallback
    ( Node::Callback* callback, CallbackInfo* info )
    { (*callback)( info->source, info->cause, info->argument ); }

class Counter {
    int &var;

  public:
    Counter(int &var) : var(var) { var += 1; }
    ~Counter()                    { var -= 1; }
};

void Node::notifyChangeCallbacks(Node::Callback::Cause cause,
    Node *a) 
{
    assert( subtree_is_dag() );

    CallbackInfo info( *this, cause, a );
    {
        Counter twl( currently_iterating_changecallbacks );
        nstd::for_each( changecallbacks,
            std::bind2nd( ptr_fun(notifyChangeCallback), &info) );
    }
    if ( ! currently_iterating_changecallbacks &&
         ! data->deferred_callback_actions.empty() )
        do_deferred_callback_management();

    /* Hack for attributes: Generate a message for the parent as well. */
    if ( cause == Node::Callback::ValueChanged && 
                dynamic_cast<Object*>(this) == NULL )
        for (list<Node*>::iterator i = parents.begin(); i != parents.end();
                                                                i++)
            (*i)->notifyChangeCallbacks( cause, this );
}

void Node::manage_callback( CallbackOp action, Callback *argument ) {
    assert( check_node_validity(this) );
    assert( argument != NULL );

    if ( action == Add ) {
        changecallbacks.push_back( argument );
        argument->data->attached_to.insert(this);
    } else if (action == Remove) {
        nstd::erase( changecallbacks, argument );
        argument->data->attached_to.erase( this );
    }
}

void Node::do_deferred_callback_management() {
    while ( data->deferred_callback_actions.size() > 0 ) {
        manage_callback( data->deferred_callback_actions.front().first, 
                         data->deferred_callback_actions.front().second );
        data->deferred_callback_actions.pop();
    }
}

void Node::addChangeCallback(Callback& callback) {
    if ( currently_iterating_changecallbacks )
        data->deferred_callback_actions.push( make_pair(Add, &callback) );
    else
        manage_callback( Add, &callback );
}

void Node::removeChangeCallback(Callback& callback) {
    if ( currently_iterating_changecallbacks ) {
        data->deferred_callback_actions.push( 
            make_pair(Remove, &callback) );
    } else
        manage_callback( Remove, &callback );
}

const Node& Node::operator[](const string& name) const 

{
    std::map<std::string,Node*>::const_iterator i 
        = data->childrenByName.find(name);

    if (i == data->childrenByName.end()) {
        std::stringstream message;
        message << "The node '" << getName() << "' at " << this
                << " has no child node named '" << name << "'";
        throw std::invalid_argument( message.str() );
    } else 
        return *i->second;
}

Node& Node::operator[](const string& name)

{
    std::map<std::string,Node*>::const_iterator i 
        = data->childrenByName.find(name);

    if (i == data->childrenByName.end()) {
        std::stringstream message;
        message << "The node '" << getName() << "' at " << this
                << " has no child node named '" << name << "'";
        throw std::invalid_argument( message.str() );
    } else 
        return *i->second;
}

void Node::processCommand(istream &in) 
 
{
    string cmd, name;
    in >> cmd;

    if ( !in ) return;

    if ( cmd == "set" || cmd == "forSet" || cmd == "in" ) {
        in >> name;
    } else
        name = cmd;

    if (cmd == "=") name = "value";
    (*this)[name].processCommand(in);
}

#ifndef NDEBUG
bool Node::subtree_is_dag() const {
    try {
        std::set<const Node*> store;
        check_and_insert_subtree( this, this, &store );
        return true;
    } catch (const std::runtime_error& e) {
        cerr << e.what() << endl;
        return false;
    }
}
#endif

template <typename Ty>
class ListAccumulator : public std::list<Ty>,
    public std::unary_function<std::list<Ty>&,void> 
{
  public:
    void operator()(std::list<Ty> &b) { this->splice( this->end(), b ); }
};

std::list<std::string> Node::printValues() const {
    assert( subtree_is_dag() );
    list< list<string> > pre_results;
    ListAccumulator<string> results;

    for ( std::list<Node*>::const_iterator c = begin(); c != end(); c++ )
        pre_results.push_back( (*c)->printValues() );

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

void Node::manage(Node *o) {
    data->managed.insert(o);
}

bool Node::has_child_named(const std::string& name) const
{ return (data->childrenByName.find(name) != data->childrenByName.end()); }

}
