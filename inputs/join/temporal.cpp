#include "temporal.hpp"
#include <dStorm/image/MetaInfo.h>
#include <Eigen/Core>
#include <dStorm/output/LocalizedImage.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <boost/variant/apply_visitor.hpp>
#include <boost/bind/bind.hpp>
#include <boost/mpl/for_each.hpp>

#include <dStorm/localization/Fields.h>

using boost::placeholders::_1;

namespace dStorm {
namespace input {
namespace join {

template <typename Type>
bool iterator<Type,temporal_tag>::InputPart::operator==( const InputPart& o ) const
{ 
    return source.get() == o.source.get() && begin == o.begin && end == o.end;
}

void merge_size( Traits<engine::ImageStack>& onto, const Traits<engine::ImageStack> with, int i )
{
    if ( onto.plane_count() != with.plane_count() ) {
            std::stringstream ss;
            ss << "Number of planes for channel 0 (" << onto.plane_count() << ") and for channel " << i 
                << " (" << with.plane_count() << ") do not agree." << std::endl;
            throw std::runtime_error(ss.str());
    }
    for (int i = 0; i < onto.plane_count(); ++i) {
        if ( onto.plane(i).image.size != with.plane(i).image.size ) {
            std::stringstream ss;
            ss << "Image dimensions for channel 0 (" << onto.plane(i).image.size.transpose() << ") and for channel " << i 
                << " (" << with.plane(i).image.size.transpose() << ") do not agree." << std::endl;
            throw std::runtime_error(ss.str());
        }
    }
}

struct merge_localization_traits {
    typedef Traits<Localization> GenTraits;
    typedef void result_type;

    void operator()( localization::ImageNumber, GenTraits& _onto, const GenTraits& _with ) {}

    template <typename Tag>
    void operator()( Tag, GenTraits& onto_traits, const GenTraits& with_traits )
    {
        typedef localization::MetaInfo<Tag> Traits;
        Traits& onto = onto_traits;
        const Traits& with = with_traits;
        if ( ! Traits::has_range ) return;

        if ( ! with.is_given ) {
          onto.is_given = false;
        }

        if ( onto.range().first.is_initialized() && with.range().first.is_initialized() )
            onto.range().first = std::min( *onto.range().first, *with.range().first );
        else
            onto.range().first.reset();

        if ( onto.range().second.is_initialized() && with.range().second.is_initialized() )
            onto.range().second = std::max( *onto.range().second, *with.range().second );
        else
            onto.range().second.reset();
    }
};

void merge_size( Traits<Localization>& onto, const Traits<Localization> with, int )
{
    boost::mpl::for_each< localization::Fields >(boost::bind( 
        merge_localization_traits(), 
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
        auto range = images[i]->image_number().range();
        if ( rv->image_number().range().second.is_initialized() &&
             range.first.is_initialized() && range.second.is_initialized() ) {
            *rv->image_number().range().second += (*range.second - *range.first) + 1 * boost::units::camera::frame;
        } else {
            rv->image_number().range().second.reset();
        }
    }
    
    return rv;
}

template <typename Type>
iterator<Type,temporal_tag>::InputPart::InputPart(
    const boost::shared_ptr<Base>& base, const input::Traits<Type>& traits )
: begin( base->begin() ), end( base->end() ), source( base )
{
    auto range = traits.image_number().range();
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

template <typename Type>
Type& iterator<Type,temporal_tag>::dereference() const
{
    if ( ! joined.is_initialized() ) {
        joined = *bases.front().begin;
        max_frame_count = std::max( joined->frame_number(), max_frame_count );
        joined->set_frame_number( joined->frame_number() + current_offset );
    }
    return *joined;
}

template class iterator< engine::ImageStack, temporal_tag >;
template class iterator< dStorm::output::LocalizedImage, temporal_tag >;
template class merge_traits< engine::ImageStack, temporal_tag >;
template class merge_traits< dStorm::output::LocalizedImage, temporal_tag >;

}
}
}
