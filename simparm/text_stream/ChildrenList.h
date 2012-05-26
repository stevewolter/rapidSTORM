#ifndef SIMPARM_TEXT_STREAM_CHILDREN_LIST_H
#define SIMPARM_TEXT_STREAM_CHILDREN_LIST_H

#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/remove.hpp>
#include <vector>

namespace simparm {
namespace text_stream {

template <typename Type>
struct ChildrenList {
    std::vector< Type* > elements;
    struct name_is {
        const std::string& n; 
        name_is( const std::string& n ) : n(n) {}
        bool operator()( const Type* t ) { return t->get_name() == n; }
    };
public:
    void add( Type& t ) 
        { elements.push_back(&t); }
    void remove( Type& t ) 
        { elements.erase( boost::range::remove( elements, &t ) ); }
    Type* look_up( const std::string& name ) { 
        typename std::vector< Type* >::const_iterator i = boost::range::find_if( elements, name_is(name) );
        if ( i != elements.end() ) return *i; else return NULL;
    }
    template <typename Functor>
    void for_each( Functor f ) const 
        { boost::range::for_each( elements, f ); }
};

}
}

#endif
