#include "inputs/ROIFilter.h"

#include <simparm/BoostUnits.h>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/units/io.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>
#include <dStorm/image/extend.h>
#include <dStorm/Image.h>
#include <dStorm/image/slice.h>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/input/MetaInfo.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/localization/Traits.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/output/LocalizedImage.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <dStorm/UnitEntries/FrameEntry.h>
#include <dStorm/units/frame_count.h>

namespace dStorm {
namespace ROIFilter {

struct Config {
    simparm::Object name_object;
    IntFrameEntry first_frame;
    simparm::Entry< boost::optional< frame_index > > last_frame;

    Config();
    void attach_ui( simparm::NodeHandle at ) {
        simparm::NodeHandle r = name_object.attach_ui(at);
        first_frame.attach_ui( r ); 
        last_frame.attach_ui( r ); 
    }
};

struct ImageNumber
: public boost::static_visitor<frame_index>
{
    template <typename Type>
    frame_index operator()( const Type& e )
        { return e.frame_number(); }
};

template <typename Ty>
class Source
: public input::AdapterSource<Ty>
{
    const frame_index from;
    const boost::optional<frame_index> to;
    bool saw_last_image;

    inline bool is_in_range(const Ty& t) const;
    void attach_local_ui_( simparm::NodeHandle ) {}
    bool GetNext(int thread, Ty* target) OVERRIDE;

  public:
    Source( std::auto_ptr< input::Source<Ty> > upstream,
            frame_index from, boost::optional<frame_index> to)
        : input::AdapterSource<Ty>(upstream), from(from), to(to), saw_last_image(false) {}
    Source* clone() const { return new Source(*this); }

    void modify_traits( input::Traits<Ty>& p)
    {
        frame_index from = std::max( this->from, *p.image_number().range().first );
        if ( p.image_number().range().second.is_initialized() && to.is_initialized() ) {
            p.image_number().range().second = std::min(*to, *p.image_number().range().second);
        } else if ( to.is_initialized() ) {
            p.image_number().range().second = *to;
        }
        p.image_number().range().first = from;
    }
};

template <typename Ty>
bool Source<Ty>::GetNext(int thread, Ty* target) {
    if (saw_last_image) {
        return false;
    }

    while (true) {
        if (!input::AdapterSource<Ty>::GetNext(thread, target)) {
            return false;
        }

        if (to && ImageNumber()(*target) == *to) {
            saw_last_image = true;
        }

        if (is_in_range(*target)) {
            return true;
        }
    }
}

template <class Ty>
bool Source<Ty>::is_in_range(const Ty& t) const
{
    frame_index f = ImageNumber()(t);
    bool rv = f >= from && (!to.is_initialized() || f <= *to);
    return rv;
}

class ChainLink 
: public input::Method<ChainLink>
{
    friend class input::Method<ChainLink>;

    Config config;
    simparm::BaseAttribute::ConnectionStore listening[2];

    typedef localization::MetaInfo<localization::ImageNumber> TemporalTraits;

    void set_temporal_ROI( TemporalTraits& t ) {
        t.range().first = config.first_frame();
        if ( config.last_frame().is_initialized() )
            t.range().second = config.last_frame();
    }
    void update_traits( input::MetaInfo&, input::Traits<engine::ImageStack>& traits ) {
        set_temporal_ROI( traits.image_number() );
    }
    template <typename Type>
    void update_traits( input::MetaInfo&, input::Traits<Type>& traits ) 
        { set_temporal_ROI( traits.image_number() ); }

    void notice_traits( const input::MetaInfo&, const input::Traits<engine::ImageStack>& t ) {
    }
    template <typename Type>
    void notice_traits( const input::MetaInfo&, const input::Traits<Type>& ) {}

    template <typename Type>
    input::Source<Type>* make_source( std::auto_ptr< input::Source<Type> > p ) {
        if ( config.first_frame() > 0 * camera::frame 
            || config.last_frame().is_initialized() )
        {
            return new Source<Type>( p, config.first_frame(), config.last_frame() );
        } else
            return p.release();
    }

  public:
    static std::string getName() { return "ROIFilter"; }
    void attach_ui( simparm::NodeHandle at ) { 
        listening[0] = config.first_frame.value.notify_on_value_change( 
            boost::bind( &input::Method<ChainLink>::republish_traits_locked, this ) );
        listening[1] = config.last_frame.value.notify_on_value_change( 
            boost::bind( &input::Method<ChainLink>::republish_traits_locked, this ) );
        config.attach_ui( at ); 
    }
};

Config::Config() 
: name_object(ChainLink::getName(), "Image selection filter"),
  first_frame("FirstImage", 0 * camera::frame),
  last_frame( "LastImage", boost::optional< frame_index >() )
{
    first_frame.set_user_level( simparm::Intermediate );
    last_frame.set_user_level( simparm::Intermediate );
}

std::auto_ptr<input::Link> make_link() {
    return std::auto_ptr<input::Link>( new ChainLink() );
}

}
}
