#include "Callback.hh"
#include "Link.hh"
#include "Link_impl.hh"

#include <iostream>
#include "Node.hh"

namespace simparm {

struct Publisher::Pimpl {
    Publisher& node;
    Pimpl(Publisher& node) : node(node) {}

    typedef Link<Pimpl,Listener::Data> ListenerLink;
    typedef ListenerLink::List ListenerList;
    ListenerLink::List callbacks;

    template <class From, class To, bool au>
    typename Link<From,To>::List&
        get_link_list() { return callbacks; }
    void link_removed( ListenerLink::WhichEnd, Listener::Data& ) {
    }
};

struct Listener::Data {
    typedef Publisher::Pimpl::ListenerLink ListenerLink;

    Publisher::Pimpl::ListenerList attached_to;
    Event::Cause limit_to;
    Listener& cb;

    Data(Listener& cb) : cb(cb) {}

    template <class From, class To, bool at_upper>
        typename Link<From,To>::List& get_link_list();
    void link_removed( ListenerLink::WhichEnd, Publisher::Pimpl& ) 
        {}
};

Publisher::Publisher() 
: data( new Pimpl(*this) )
{
}

Publisher::~Publisher() {}

void Publisher::addChangeCallback(Listener& callback) {
    new Pimpl::ListenerLink( *data, *callback.data );
}

void Publisher::removeChangeCallback(Listener& callback) 
{
    for ( Pimpl::ListenerList::iterator
        i = data->callbacks.begin(); 
        i != data->callbacks.end();
        i++)
    {
        if ( &(*i)->down_end() == callback.data ) {
            delete *i;
            return;
        }
    }
}

template <>
Link<Publisher::Pimpl,Listener::Data>::List&
Listener::Data::get_link_list
    <Publisher::Pimpl,Listener::Data,false>() 
    { return attached_to; }

void Publisher::notifyChangeCallbacks(
    Event::Cause cause,
    Publisher *a) 
{
    std::list<Listener::Data*> callbacks;
    for ( Pimpl::ListenerList::const_iterator i 
             = data->callbacks.begin();
          i != data->callbacks.end();
          i++ )
        callbacks.push_back( & (*i)->down_end() );

    LinkChangeEvent e( *this, cause, *a );

    for ( std::list<Listener::Data*>::iterator i = callbacks.begin();
                                         i != callbacks.end(); i++)
        if ( (*i)->limit_to == cause ||
             (*i)->limit_to == Event::All )
        {
            (*i)->cb ( e );
        }
}

Listener::Listener(Event::Cause limit_to)
    : data(new Listener::Data(*this)) 
    { data->limit_to = limit_to; }
Listener::Listener(const Listener& o) 
    : data(new Listener::Data(*this))
{
    data->limit_to = o.data->limit_to;
    for (Publisher::Pimpl::ListenerList::const_iterator 
            i = o.data->attached_to.begin();
            i != o.data->attached_to.end(); i++)
        (*i)->up_end().node.addChangeCallback(*this);
}
Listener::~Listener() {
    delete data;
}
Listener& 
Listener::operator=(const Listener& o) {
    while ( ! data->attached_to.empty() )
        delete data->attached_to.front();

    for (Publisher::Pimpl::ListenerList::const_iterator 
            i = o.data->attached_to.begin(); 
            i != o.data->attached_to.end(); i++)
        (*i)->up_end().node.addChangeCallback(*this);
    return *this;
}

template class Link<Publisher::Pimpl,Listener::Data>;


}
