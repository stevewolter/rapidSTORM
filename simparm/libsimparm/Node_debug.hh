#ifdef NDEBUG
#define DEBUG_ONLY(x)
#else
#define DEBUG_ONLY(x) x
#include "Node.hh"
#include <pthread.h>
#include <set>

namespace simparm {
    bool check_node_validity
        (const simparm::Node* node, bool alwaysprint = false);
    void print_node_debug_info( const Node *list, std::string prefix );
    void print_debug_info( const Node* list );
    void collect_subtree( const Node *a, std::set<const Node*>* into );
    bool subtrees_are_distinct( const Node *a, const Node *b );
    void check_and_insert_subtree(const Node *orig, const Node *node,
                                std::set<const Node*>* present);
    void announce_new_node( Node *node, std::string name );
    void announce_dead_node( Node *node );
}

#endif
