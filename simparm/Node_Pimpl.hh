#ifndef LINK_IMPL_HH
#define LINK_IMPL_HH

#include "Node.hh"
#include <list>
#include <map>
#include <set>
#include <pthread.h>

namespace simparm {

#if 0
struct Node::Pimpl {
    Node& node;

    typedef Node::TreeLink TreeLink;

    typedef TreeLink::List NodeList;

    typedef std::map<const std::string, TreeLink*> NameMap;
    typedef std::map<Node*, TreeLink*> NodeMap;

    /* The order of objects here is important. First delete the children,
     * because this might generate events along the parents list. Then
     * delete the parents, because they might generate callbacks. Both
     * of these events clean up the byName and byNode maps. */
    NameMap byName;
    NodeMap byNode;
    TreeLink::List parents, children;

    int _active;
    bool declared;
    std::string *_currentlyDefining;

    pthread_mutex_t* mutex;

    bool subtree_is_dag() const;

    Pimpl(Node&);
    ~Pimpl();

    /** Callback function for simparm::Link */
    template <typename From, typename To, bool at_up_node>
        typename Link<From,To>::List& get_link_list();

    void link_removed( TreeLink::WhichEnd end, Pimpl& other_end ); 
};
#endif


}

#endif
