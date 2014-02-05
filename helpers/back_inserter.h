#ifndef DSTORM_BOOST_BACK_INSERTER_H

#include <iterator>
#include <boost/iterator/iterator_facade.hpp>

namespace boost {

template <typename Container>
class back_insert_iterator 
: public boost::iterator_facade< 
    back_insert_iterator<Container>,
    typename Container::value_type,
    boost::incrementable_traversal_tag >
{
    Container* c;
    friend class boost::iterator_core_access;
    typedef typename Container::value_type Value;

    void increment() {}
    Value& dereference() const { 
        c->push_back( Value() );
        return c->back(); 
    }
    
  public:
    back_insert_iterator( Container& c ) : c(&c) {}
};

template <typename Container>
back_insert_iterator<Container> back_inserter( Container& c )
    { return back_insert_iterator<Container>(c); }

}

#endif
