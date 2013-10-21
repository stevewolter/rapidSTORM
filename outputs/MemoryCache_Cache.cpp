#include "MemoryCache_Cache.h"
#include <dStorm/output/LocalizedImage_traits.h>
#include <dStorm/localization/field_index_enumeration.h>
#include <boost/fusion/include/at.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>
#include <boost/ptr_container/ptr_inserter.hpp>

#include <iostream>

namespace dStorm {
namespace memory_cache {

template <typename Field>
class Implementation : public Store
{
  public:
    typedef typename boost::fusion::result_of::value_at<Localization, Field >::type::Traits TraitsType;
    typedef typename TraitsType::ValueType Value;

  private:
    std::vector< Value > values;
  public:
    Implementation() {}
    Implementation* clone() const { return new Implementation(*this); }
    Implementation* make_empty_clone() const { return new Implementation(); }
    void store(const_loc_iter from, const_loc_iter to) {
        for ( ; from != to; ++from )
            values.push_back( boost::fusion::at<Field>( *from ).value() );
    }
    void recall(int offset, loc_iter from, loc_iter to) const {
        for ( loc_iter i = from; i != to; ++i ) {
            boost::fusion::at<Field>(*i) = values[ offset + i - from ];
        }
    }
};

struct CacheCreator {
    typedef void result_type;

    template <typename Field, typename OutputIterator>
    void operator()( Field, const input::Traits<Localization>& traits, OutputIterator o ) const
    {
        typedef typename boost::fusion::result_of::value_at<Localization, Field >::type::Traits TraitsType;
        if ( static_cast<const TraitsType&>( traits ).is_given ) {
            *o = new Implementation<Field>();
        }
    }
};

boost::ptr_vector<Store>
Store::instantiate_necessary_caches( const input::Traits<Localization>& traits )
{
    boost::ptr_vector<Store> rv;
    boost::mpl::for_each< localization::FieldIndices >
        ( boost::bind( CacheCreator(),
            _1, traits, boost::ptr_container::ptr_back_inserter(rv) ) );
    return rv;
}

}
}
