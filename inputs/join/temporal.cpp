#include "temporal.hpp"
#include <dStorm/ImageTraits.h>
#include <Eigen/Core>
#include <dStorm/localization/record.h>
#include <dStorm/output/LocalizedImage.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <boost/variant/apply_visitor.hpp>
#include <boost/bind/bind.hpp>
#include <boost/mpl/for_each.hpp>

#include <dStorm/traits/scalar.h>
#include <dStorm/traits/scalar_iterator.h>
#include <dStorm/localization/field_index_enumeration.h>

namespace dStorm {
namespace input {
namespace join {

template <typename Type>
bool iterator<Type,temporal_tag>::InputPart::operator==( const InputPart& o ) const
{ 
    return source.get() == o.source.get() && begin == o.begin && end == o.end;
}

void merge_size( Traits<engine::Image>& onto, const Traits<engine::Image> with, int i )
{
        if ( onto.size != with.size ) {
            std::stringstream ss;
            ss << "Image dimensions for channel 0 (" << onto.size << ") and for channel " << i 
                << " (" << with.size << ") do not agree." << std::endl;
            throw std::runtime_error(ss.str());
        }
}

template <typename Exception>
struct merge_localization_traits {
    typedef Traits<Localization> GenTraits;
    typedef void result_type;

    void operator()( Exception, GenTraits& _onto, const GenTraits& _with ) {}
    template <typename Field>
    void operator()( Field, GenTraits& onto, const GenTraits& with )
    {
        typedef typename boost::fusion::result_of::value_at<Localization, Field >::type::Traits Traits;
        if ( ! Traits::has_range ) return;
        typedef traits::Scalar< Traits > Scalar;

        for ( typename Scalar::Iterator i = Scalar::begin(); i != Scalar::end(); ++i )
        {
            if ( ! i->is_given(with) ) i->is_given(onto) = false;
            if ( ! i->uncertainty_is_given(with) ) i->uncertainty_is_given(onto) = false;

            if ( i->range(onto).first.is_initialized() && i->range(with).first.is_initialized() )
                i->range(onto).first = std::min( *i->range(onto).first, *i->range(with).first );
            else
                i->range(onto).first.reset();

            if ( i->range(onto).second.is_initialized() && i->range(with).second.is_initialized() )
                i->range(onto).second = std::max( *i->range(onto).second, *i->range(with).second );
            else
                i->range(onto).second.reset();
        }
    }
};

void merge_size( Traits<Localization>& onto, const Traits<Localization> with, int )
{
    boost::mpl::for_each< localization::FieldIndices >(
        boost::bind( 
            merge_localization_traits< boost::mpl::int_<Localization::Fields::ImageNumber> >(), 
            _1, boost::ref(onto), boost::ref(with) ) );
    int children_count = std::min( onto.source_traits.size(), with.source_traits.size() );
    onto.source_traits.resize( children_count );
    for ( int i = 0; i < children_count; ++i)
        merge_size( *onto.source_traits[i], *with.source_traits[i], i );
}

template <typename Type>
typename traits_merger<Type>::result_type
merge_traits<Type,temporal_tag>::operator()
    ( const typename traits_merger<Type>::argument_type& images ) const
{
    std::auto_ptr< Traits<Type> > rv( new Traits<Type>(*images[0]) );
    for ( size_t i = 1; i < images.size(); ++i ) {
        merge_size( *rv, *images[i], i );
        const dStorm::traits::ImageNumber::RangeType& range = images[i]->image_number().range();
        if ( rv->image_number().range().second.is_initialized() &&
             range.first.is_initialized() && range.second.is_initialized() )
            *rv->image_number().range().second += (*range.second - *range.first) + 1 * boost::units::camera::frame;
    }
    
    return rv;
}

template <typename Type>
iterator<Type,temporal_tag>::InputPart::InputPart(
    const boost::shared_ptr<Base>& base, const input::Traits<Type>& traits )
: begin( base->begin() ), end( base->end() ), source( base )
{
    const traits::ImageNumber::RangeType range = traits.image_number().range();
    if ( range.first.is_initialized() && range.second.is_initialized() )
        length = *range.second - *range.first;
}

template <typename Type>
iterator<Type,temporal_tag>::iterator( 
    const typename Base::TraitsPtr& traits, const Traits& base_traits, Sources& sources, bool end 
)
: max_frame_count( 0 * boost::units::camera::frame ), current_offset( 0 * boost::units::camera::frame )
{
    if ( ! end ) {
        typename Traits::const_iterator j = base_traits.begin();
        for ( typename Sources::iterator i = sources.begin(); i != sources.end(); ++i )
            bases.push_back( InputPart(*i, **j++) );
        pop_empty_parts();
    }
}

template <typename Type>
void iterator<Type,temporal_tag>::pop_empty_parts() {
    while ( ! bases.empty() && bases.front().begin == bases.front().end ) {
        current_offset += (bases.front().length.is_initialized()) 
                            ? *bases.front().length 
                            : max_frame_count;
        current_offset += 1 * boost::units::camera::frame;
        bases.pop_front();
        max_frame_count = 0 * boost::units::camera::frame;
    }
}

template <typename Type>
void iterator<Type,temporal_tag>::increment() {
    joined.reset();
    ++bases.front().begin;
    pop_empty_parts();
}

template <typename Type>
bool iterator<Type,temporal_tag>::equal( const iterator& o ) const {
    return bases == o.bases;
}

struct frame_number 
: public boost::static_visitor<frame_index&>
{
    frame_index& operator()( engine::Image& a ) const { return a.frame_number(); }
    frame_index& operator()( dStorm::output::LocalizedImage& a ) const { return a.forImage; }
    frame_index& operator()( dStorm::Localization& a ) const { return a.frame_number(); }
    frame_index& operator()( dStorm::localization::EmptyLine& a ) const { return a.number; }
    frame_index& operator()( dStorm::localization::Record& a ) const {
        return boost::apply_visitor(*this, a); }
};

template <typename Type>
Type& iterator<Type,temporal_tag>::dereference() const
{
    if ( ! joined.is_initialized() ) {
        joined = *bases.front().begin;
        max_frame_count = std::max( frame_number()( *joined ), max_frame_count );
        frame_number()( *joined ) += current_offset;
    }
    return *joined;
}

template class iterator< engine::Image, temporal_tag >;
template class iterator< dStorm::Localization, temporal_tag >;
template class iterator< dStorm::localization::Record, temporal_tag >;
template class iterator< dStorm::output::LocalizedImage, temporal_tag >;
template class merge_traits< engine::Image, temporal_tag >;
template class merge_traits< dStorm::Localization, temporal_tag >;
template class merge_traits< dStorm::localization::Record, temporal_tag >;
template class merge_traits< dStorm::output::LocalizedImage, temporal_tag >;

}
}
}
