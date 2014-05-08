#include "inputs/join/temporal.h"
#include <deque>
#include "image/MetaInfo.h"
#include <Eigen/Core>
#include "output/LocalizedImage.h"
#include "output/LocalizedImage_traits.h"
#include <boost/variant/apply_visitor.hpp>
#include <boost/bind/bind.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/lexical_cast.hpp>

#include "localization/Fields.h"

namespace dStorm {
namespace temporal_join {
namespace {

template <typename Type>
class Source
: public input::Source<Type>
{
    typedef input::Source<Type> Base;
    typedef std::vector< std::unique_ptr<Base> > Sources;
    typename Base::TraitsPtr traits;

    Sources sources;
    typename Sources::iterator current_source;
    std::vector< boost::shared_ptr< const input::Traits<Type> > > base_traits;
    std::vector< std::unique_ptr<simparm::Object> > connection_nodes;
    frame_index offset, current_source_max;

    void attach_ui_( simparm::NodeHandle n ) { 
        for (size_t i = 0; i < sources.size(); ++i) {
            std::unique_ptr< simparm::Object > object( 
                new simparm::Object("Channel" + boost::lexical_cast<std::string>(i), "") );
            sources[i]->attach_ui( object->attach_ui( n ) ); 
            connection_nodes.push_back( std::move(object) );
        }
    }

    void set_thread_count(int num_threads) OVERRIDE {
        assert(num_threads == 1);
    }

    bool GetNext(int thread, Type* output) OVERRIDE;

  public:
    Source( Sources s ) : sources(std::move(s)), current_source(sources.begin()), offset(0 * camera::frame), current_source_max(0 * camera::frame) {}
    void dispatch(input::BaseSource::Messages m) {
        for (typename Sources::iterator i = sources.begin(); i != sources.end(); ++i)
            (*i)->dispatch(m);
    }

    typename Base::TraitsPtr get_traits( input::BaseSource::Wishes r );

    input::BaseSource::Capabilities capabilities() const {
        input::BaseSource::Capabilities rv;
        rv.set();
        for (typename Sources::const_iterator i = sources.begin(); i != sources.end(); ++i)
            rv = rv.to_ulong() & (*i)->capabilities().to_ulong();
        return rv;
    }
};

struct merge_localization_traits {
    typedef input::Traits<Localization> GenTraits;
    typedef void result_type;

    void operator()( localization::ImageNumber, GenTraits& _onto, const GenTraits& _with ) {}

    template <typename Tag>
    void operator()( Tag tag, GenTraits& onto_traits, const GenTraits& with_traits )
    {
        typedef localization::MetaInfo<Tag> Traits;
        Traits& onto = onto_traits.field(tag);
        const Traits& with = with_traits.field(tag);
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

void merge_size( input::Traits<engine::ImageStack>& onto, const input::Traits<engine::ImageStack> with, int i ) {
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

void merge_size( input::Traits<Localization>& onto, const input::Traits<Localization> with, int ) {
    boost::mpl::for_each< localization::Fields >(boost::bind( 
        merge_localization_traits(), 
        _1, boost::ref(onto), boost::ref(with) ) );
    int children_count = std::min( onto.source_traits.size(), with.source_traits.size() );
    onto.source_traits.resize( children_count );
    for ( int i = 0; i < children_count; ++i)
        merge_size( *onto.source_traits[i], *with.source_traits[i], i );
}

template <typename Type>
typename Source<Type>::Base::TraitsPtr Source<Type>::get_traits( input::BaseSource::Wishes r ) {
    for (typename Sources::iterator i = sources.begin(); i != sources.end(); ++i) {
        base_traits.push_back( (*i)->get_traits(r) );
    }
    traits.reset(merge_traits(base_traits, tag()).release());
    return traits;
}

template <typename Type>
std::unique_ptr< input::Traits<Type> > merge_traits_implementation(
        const std::vector< boost::shared_ptr< const input::Traits<Type> > >& base_traits) {
    std::unique_ptr< input::Traits<Type> > rv( new input::Traits<Type>(*base_traits[0]) );
    for ( size_t i = 1; i < base_traits.size(); ++i ) {
        merge_size( *rv, *base_traits[i], i );
        auto range = base_traits[i]->image_number().range();
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
bool Source<Type>::GetNext(int thread, Type* target) {
    if (current_source == sources.end()) {
        return false;
    }

    if (!(*current_source)->GetNext(thread, target)) {
        const input::Traits<Type>& traits = *base_traits[current_source - sources.begin()];
        offset += traits.image_number().range().second.get_value_or(current_source_max)
            + 1 * camera::frame;
        current_source_max = 0 * camera::frame;
        ++current_source;
        return GetNext(thread, target);
    }

    current_source_max = std::max(target->frame_number(), current_source_max);
    target->set_frame_number( target->frame_number() + offset );
    return true;
}

}

std::unique_ptr< input::Traits<engine::ImageStack> > merge_traits(
        const std::vector< boost::shared_ptr< const input::Traits<engine::ImageStack> > >& traits,
        tag t) {
    return merge_traits_implementation(traits);
}

std::unique_ptr< input::Traits<output::LocalizedImage> > merge_traits(
        const std::vector< boost::shared_ptr< const input::Traits<output::LocalizedImage> > >& traits,
        tag t) {
    return merge_traits_implementation(traits);
}

std::unique_ptr<input::Source<engine::ImageStack>> Create(
        std::vector<std::unique_ptr<input::Source<engine::ImageStack>>> sources) {
    return std::unique_ptr<input::Source<engine::ImageStack>>(new Source<engine::ImageStack>(std::move(sources)));
}

std::unique_ptr<input::Source<output::LocalizedImage>> Create(
        std::vector<std::unique_ptr<input::Source<output::LocalizedImage>>> sources) {
    return std::unique_ptr<input::Source<output::LocalizedImage>>(new Source<output::LocalizedImage>(std::move(sources)));
}

}
}
