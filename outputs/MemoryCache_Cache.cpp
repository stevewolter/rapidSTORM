#include "MemoryCache_Cache.h"
#include <dStorm/traits/scalar.h>
#include <dStorm/traits/scalar_iterator.h>
#include <dStorm/traits/tags.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <dStorm/localization/field_index_enumeration.h>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>
#include <boost/ptr_container/ptr_inserter.hpp>

namespace dStorm {
namespace memory_cache {

template <typename Field, typename Tag>
class Implementation : public Store
{
  public:
    typedef typename boost::fusion::result_of::value_at<Localization, Field >::type::Traits TraitsType;
    typedef typename Tag::template in<TraitsType> TaggedTraits;
    typedef traits::Scalar<TraitsType> Scalar;
    typedef typename Scalar::template result_of<Tag>::get Value;

  private:
    Scalar scalar;
    std::vector< Value > values;
  public:
    Implementation( const Scalar& scalar )
        : scalar(scalar) {}
    Implementation<Field,Tag>* clone() const 
        { return new Implementation(*this); }
    Implementation<Field,Tag>* make_empty_clone() const
        { return new Implementation(scalar); }
    void store(const_loc_iter from, const_loc_iter to) {
        values.reserve( values.size() + (to - from) );
        for ( const_loc_iter i = from; i != to; ++i ) {
            values.push_back( scalar.template get_field<Tag,Field::value>( *i ) );
        }
    }
    void recall(int offset, loc_iter from, loc_iter to) const {
        for ( loc_iter i = from; i != to; ++i ) {
            scalar.template set_field<Tag,Field::value>( *i ) = values[ offset + i - from ];
        }
    }
};

struct CacheCreator {
    typedef void result_type;

    template <typename Field, typename Tag, typename OutputIterator>
    void operator()( Field, Tag, const input::Traits<Localization>& traits, OutputIterator o ) const
    {
        typedef Implementation<Field,Tag> Result;
        typedef typename Result::Scalar Scalar;
        for ( typename Scalar::Iterator i = Scalar::begin(); i != Scalar::end(); ++i )
            if ( i->is_given( traits ) && Result::TaggedTraits::in_localization )
                *o = new Result(*i);
    }
};

struct FieldCacheCreator
{
    typedef void result_type;
    template <typename Field, typename OutputIterator>
    void operator()( Field, const input::Traits<Localization>& traits, OutputIterator o ) const
    {
        boost::mpl::for_each< traits::tags >(
            boost::bind( CacheCreator(), Field(), _1, traits, o ) );
    }
};

boost::ptr_vector<Store>
Store::instantiate_necessary_caches( const input::Traits<Localization>& traits )
{
    boost::ptr_vector<Store> rv;
    boost::mpl::for_each< localization::FieldIndices >
        ( boost::bind( FieldCacheCreator(),
            _1, traits, boost::ptr_container::ptr_back_inserter(rv) ) );
    return rv;
}

}
}
