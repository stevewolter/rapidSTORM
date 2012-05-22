#include "Node.hh"
#include <sstream>
#include <iostream>
#include <functional>
#include <algorithm>
#include <cassert>
#include <stl_helpers.hh>
#include "Object.hh"
#include <set>

#include "Node_debug.hh"
#include "Node_Pimpl.hh"
#include "Link_impl.hh"

#include "Message.hh"

#if 0
using namespace std;

struct Lock {
    pthread_mutex_t* mutex;
  public:
    Lock( pthread_mutex_t* m ) 
        : mutex(m) { if ( m ) pthread_mutex_lock(m); }
    ~Lock() { if ( mutex ) pthread_mutex_unlock(mutex); }
};

namespace simparm {

template <typename Ty> 
void destruct(Ty* p) { delete p; }

bool Node::is_valid_name(const std::string& to) {
    if ( to == "" ) return false;
    for (unsigned int i = 0; i < to.size(); i++)
        if ( !isalnum( to[i] ) && to[i] != '_' )
            return false;
    return true;
}

Node::Node(const std::string& name) 
    : data(new Pimpl(*this)), name(name)
{
    if ( ! is_valid_name(name) ) {
        assert( false );
        throw std::invalid_argument("'" + name + "' is not a valid name "
                                    "for a simparm node.");
    }
                                    
    DEBUG_ONLY( announce_new_node( this, name ) );
}

Node::Pimpl::Pimpl(Node& n) 
    : node(n),
      _active(0), 
      declared(false),
      _currentlyDefining(NULL),
      mutex(NULL) {}

Node::Node(const Node &n) 
    : data(new Pimpl(*this)), name(n.name)
{ 
    Lock lock( n.data->mutex );
    DEBUG_ONLY( announce_new_node( this, name ) ); 
}

Node::~Node() {
    assert( data->subtree_is_dag() );
    pthread_mutex_t *mutex = data->mutex;

    assert( data->_active != -1 );
    DEBUG_ONLY( data->_active = -1 );
    {
        Lock lock(mutex);
        delete data;
    }
    if ( mutex != NULL ) {
        pthread_mutex_destroy( mutex );
        delete mutex;
    }
    DEBUG_ONLY( announce_dead_node(this) );
}

Node::Pimpl::~Pimpl() {
}

Node::iterator Node::insert_( const iterator& where, Node& object ) 
{
    Pimpl::TreeLink *link;
  {
    Lock lock( data->mutex );

    /* This link object needs no deallocation because it is managed by
     * the parents/children lists in data, where it is self-inserted. */
    link = new Pimpl::TreeLink(
        *this, object, where, object.data->parents.end() );

    const std::string& childName = object.name;

    if ( data->byName.find(childName) != data->byName.end() )
        throw std::runtime_error(
            "Name conflict for name '" + childName + "' in '" + getName() 
            + "'");
    else
        data->byName.insert( make_pair(childName, link) );

    data->byNode.insert( make_pair( &object, link ) );

    if ( isActive() ) {
        object.setActivity( true );
        print( object.define() );
    }
  }

    object.notifyChangeCallbacks( Event::AddedParent, this );
    this->notifyChangeCallbacks( Event::AddedChild, &object );

    assert( data->subtree_is_dag() );
    return link->get_iterator_in_up();
}

void Node::Pimpl::link_removed( TreeLink::WhichEnd end, Pimpl& other_end )
{
  {
    Lock lock( mutex );
    if ( end == TreeLink::SecondEnd ) {
        byName.erase( other_end.node.getName() );
        byNode.erase( &other_end.node );

        std::string undefinition;
        if (node.isActive())
            node.print( other_end.node.undefine() ); 
    }
  }

    assert( subtree_is_dag() );
}

Node::iterator Node::erase( const iterator& it )
{
    iterator prev = it;
    --prev;
    it.break_link();
    return prev;
}

void Node::erase( Node& node ) {
    Lock lock( data->mutex );
    std::map<Node*,TreeLink*>::iterator i = data->byNode.find(&node);
    if ( i != data->byNode.end() ) {
        delete i->second;
    } else
        throw std::invalid_argument("Node " + node.name +
                                    " is no member of " + name);
}

void Node::erase(const std::string& name) { 
    Lock lock( data->mutex );
    std::map<const std::string,TreeLink*>::iterator
        i = data->byName.find(name);
    if ( i != data->byName.end() ) {
        delete i->second;
    } else
        throw std::invalid_argument("Node " + name +
                                    " is no member of " + this->name);
}

void Node::clearChildren() {
    Lock lock( data->mutex );
    while ( ! data->children.empty() )
        delete data->children.front();
}

void Node::clearParents() {
    Lock lock( data->mutex );
    while ( ! data->parents.empty() )
        delete data->parents.front();
}

void Node::setActivity(bool active) {
    Lock lock( data->mutex );
    bool wasActive = isActive();
    if ( active ) data->_active++; else data->_active--;
    assert( data->_active >= 0 );
    if ( wasActive != isActive() ) {
        bool is_active = isActive();
        for ( iterator i = begin(); i != end(); i++ )
            i->setActivity( is_active );
        notifyChangeCallbacks( Event::ActivityChanged, NULL );
    }
}

void Node::print(const std::string& what) {
    Lock lock( data->mutex );
    if (data->_currentlyDefining) {
        (*data->_currentlyDefining) += "\n" + what;
    } else if (isActive() && !data->parents.empty()) {
        string message = "in";
        if (message.size() > 0 && 
            message[ message.size()-1 ] != ' ') message += " ";
        message += getName() + " " + what;
        for ( parents_iterator i = begin_parents(); i != end_parents();i++)
        {
            i->print( message );
        }
    } 
}

void Node::send( Message& m ) {
    Lock lock( data->mutex );
    if ( m.help_file() == "" && has_child_named("help_file") ) {
        const Node& hf = (*this)["help_file"];
        const Attribute<std::string>* shf = dynamic_cast< const Attribute<std::string>* >( &hf );
        if ( shf != NULL ) m.help_file = *shf;
    }
    bool was_sent = false;
    for ( parents_iterator i = begin_parents(); i != end_parents();i++) {
        i->send( m );
        was_sent = true;
    }
    if ( ! was_sent ) {
        std::cerr << "Unhandled error message at node " << name << ": " << m << std::endl;
    }
}

std::string Node::define() {
    Lock lock( data->mutex );
    std::string definition = "declare " + getTypeDescriptor() + "\n"
                             + "name " + name;
    list<string> child_defs;

    for ( iterator i = begin(); i != end(); i++ )
        child_defs.push_back( i->define() );

    for ( list<string>::iterator i = child_defs.begin(); 
                                    i != child_defs.end(); i++)
        if ( *i != "" )
            definition += "\n" + *i;
    definition += "\nend";
    return definition;
}
std::string Node::undefine() {
    Lock lock( data->mutex );
    return "remove " + getName(); 
}

const Node& Node::operator[](const string& name) const 
{
    Lock lock( data->mutex );
    Pimpl::NameMap::const_iterator i = data->byName.find(name);

    if (i == data->byName.end()) {
        std::stringstream message;
        message << "The node '" << getName() << "' at " << this
                << " has no child node named '" << name << "'";
        throw std::runtime_error( message.str() );
    } else 
        return i->second->down_end();
}

Node& Node::operator[](const string& name)
{
    Lock lock( data->mutex );
    return const_cast<Node&>( const_cast<const Node&>(*this)[name] );
}

void Node::processCommand(istream &in) 
 
{
  Node* target = NULL;
  {
    Lock lock( data->mutex );
    string cmd, name;
    in >> cmd;

    if ( !in ) return;

    if ( cmd == "set" || cmd == "forSet" || cmd == "in" ) {
        in >> name;
    } else
        name = cmd;

    if (cmd == "=") name = "value";
    target = &(*this)[name];
  }
    target->processCommand(in);
}

#ifndef NDEBUG
bool Node::Pimpl::subtree_is_dag() const {
    return true;
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
    Lock lock( data->mutex );
    assert( data->subtree_is_dag() );
    list< list<string> > pre_results;
    ListAccumulator<string> results;

    for ( const_iterator c = begin(); c != end(); c++ )
        pre_results.push_back( c->printValues() );

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

bool Node::has_child_named(const std::string& name) const
{
    Lock lock( data->mutex );
    return (data->byName.find(name) != data->byName.end()); 
}

bool Node::isActive() const { return data->_active != 0; }

int Node::size() const { return data->children.size(); }

Node::const_iterator Node::begin() const { return data->children.begin(); }
Node::const_iterator Node::end() const { return data->children.end(); }
Node::iterator Node::begin() { return data->children.begin(); }
Node::iterator Node::end() { return data->children.end(); }

Node::const_parents_iterator Node::begin_parents() const 
    { return data->parents.begin(); }
Node::const_parents_iterator Node::end_parents() const 
    { return data->parents.end(); }
Node::parents_iterator Node::begin_parents() 
    { return data->parents.begin(); }
Node::parents_iterator Node::end_parents() 
    { return data->parents.end(); }

template <>
Node::TreeLink::List& 
Node::get_link_list<Node,Node,true>() { return data->children; }
template <>
Node::TreeLink::List& 
Node::get_link_list<Node,Node,false>() { return data->parents; }

void Node::link_removed( TreeLink::WhichEnd end, Node& other_end ) {
    data->link_removed( end, *other_end.data );
}

void Node::make_thread_safe() 
{
    if ( is_thread_safe() ) return;
    data->mutex = new pthread_mutex_t;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(data->mutex, &attr);
}

bool Node::is_thread_safe() 
{
    return data->mutex != NULL;
}

template class Link<Node,Node>;

}
#endif
