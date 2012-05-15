#ifndef SIMPARM_LINK_IMPL_HH
#define SIMPARM_LINK_IMPL_HH

#include "Link.hh"
#include "Link_iterator.hh"
#include <iostream>
#include <stdexcept>

namespace simparm {

template <class From, class To> 
struct SameObjectChecker { static inline void checker(const From&, const To&) {} };

template <class Same> 
struct SameObjectChecker<Same,Same> { 
    static inline void checker(const Same& a, const Same& b) { 
        if ( &a == &b ) throw std::runtime_error("Tried to link to self"); }
};

template <typename Type, typename List>
LinkEnd<Type,List>::LinkEnd
    ( Type& object, List& list, typename List::value_type list_node,
        typename List::iterator where ) 
: object( object ),
  list( list ),
  iter( list.insert( where, list_node ) )
{
}

template <typename Type, typename List>
LinkEnd<Type,List>::~LinkEnd() {
    list.erase( iter );
}

template <class From, class To>
Link<From,To>::List::~List() {
    while (!this->empty())
        delete this->front();
}

template <class From, class To>
Link<From,To>::Link( From& up, To& down )
: f( up, up.template get_link_list<From,To,true>(),
     this, up.template get_link_list<From,To,true>().end() ),
  t( down, down.template get_link_list<From,To,false>(),
     this, down.template get_link_list<From,To,false>().end() )
{
    SameObjectChecker<From,To>::checker( up, down );
}

template <class From, class To>
Link<From,To>::Link( 
    From& up, To& down, 
    const down_iterator& up_where,
    const up_iterator& down_where )
: f( up, up.template get_link_list<From,To,true>(), 
     this, down_where.get_base() ),
  t( down, down.template get_link_list<From,To,false>(), 
     this, up_where.get_base() )
{
    SameObjectChecker<From,To>::checker( up, down );
}

template <class From, class To>
Link<From,To>::~Link() {
    t->link_removed( FirstEnd, f.get() );
    f->link_removed( SecondEnd, t.get() );
}

}

#endif
