#include "Node.hh"
#include "Node_debug.hh"

#include <algorithm>
#include <set>
#include <map>
#include <sstream>
#include <iostream>
#include <list>

#ifndef NDEBUG
namespace simparm {

static pthread_mutex_t strucMut = PTHREAD_MUTEX_INITIALIZER;
static std::set<const simparm::Node*> present;
static std::multimap<const simparm::Node*,std::string> names;
static std::map<const Node*, pthread_t> belongers;

bool check_node_validity(const simparm::Node* node, bool alwaysprint)
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
        throw std::logic_error(ss.str());
    } else if ( belongers.find(node) != belongers.end() &&
#ifdef PTW32_VERSION
                belongers[node].p != pthread_self().p )
#else
                belongers[node] != pthread_self() )
#endif
    {
        pthread_mutex_unlock(&strucMut);
        std::stringstream ss;
        ss << "Node access violation for node " << ((void*)node) << " with name "
           << node->getName() << "\n.";
        for (Node::const_parents_iterator i = node->begin_parents();
             i != node->end_parents(); i++)
            ss << "Parent " << &(*i) << " (" << (*i).getName() << ")\n";
        throw std::logic_error(ss.str());
                                 
    } else if ( alwaysprint ) {
        std::cerr << "Node pointer " << node << " is valid and named ";
        for (std::multimap<const simparm::Node*,std::string>::const_iterator
            i = names.find( node ); i->first == node; i++)
            std::cerr << i->second << " ";
        std::cerr << "\n";
    }
    pthread_mutex_unlock(&strucMut);
    return true;
}

void print_node_debug_info( const Node *list, std::string prefix )
{
    try {
        check_node_validity( list );
        std::cerr << prefix << list << " " << list->getName() << "\n";
        for ( Node::const_iterator i = list->begin(); i != list->end(); i++)
            print_node_debug_info( &*i, prefix + "  ");
    } catch (const std::exception& e) {
        std::cerr << prefix << "Node is invalid: " << e.what() << "\n";
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
            for ( Node::const_iterator i = a->begin(); i != a->end(); i++)
                collect_subtree( &*i, into );
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

    pthread_mutex_lock(&strucMut);
    if ( ! present->insert(node).second ) {
        std::stringstream ss;
        ss << "Entry " << node << " (" << node->getName() <<
              ") is part of cycle in config graph.\n";

        pthread_mutex_unlock(&strucMut);
        throw std::logic_error(ss.str());
    }
    try {
        for ( Node::const_iterator i = node->begin();
              i != node->end(); i++)
        {
            pthread_mutex_unlock(&strucMut);
            check_and_insert_subtree( orig, &*i, present );
            pthread_mutex_lock(&strucMut);
        }
    } catch (const std::exception &e) {
        std::stringstream addline;
        addline << "Parent " << node << "(" << node->getName() << ")\n" << e.what();
        pthread_mutex_unlock(&strucMut);
        throw std::logic_error(addline.str());
    }
    present->erase(node);
    pthread_mutex_unlock(&strucMut);
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
    belongers.insert( std::make_pair(node, pthread_self()) );
    for ( Node::const_iterator i = node->begin(); i != node->end(); i++ )
        my_tree_recursive( &(*i) );
}
void my_tree (const Node *node) {
    pthread_mutex_lock( &strucMut );
    my_tree_recursive( node );
    pthread_mutex_unlock( &strucMut );
}

}
#endif
