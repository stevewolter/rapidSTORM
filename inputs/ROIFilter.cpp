#include "debug.h"
#include "ROIFilter.h"

#include <simparm/BoostUnits.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Entry_Impl.hh>
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
    typedef typename input::Source<Ty>::iterator base_iterator;

    struct _iterator;

    inline bool is_in_range(const Ty& t) const;
    void attach_local_ui_( simparm::NodeHandle ) {}

  public:
    Source( std::auto_ptr< input::Source<Ty> > upstream,
            frame_index from, boost::optional<frame_index> to)
        : input::AdapterSource<Ty>(upstream), from(from), to(to) {}
    Source* clone() const { return new Source(*this); }

    base_iterator begin();
    base_iterator end();
    void modify_traits( input::Traits<Ty>& p)
    {
        frame_index from = std::max( this->from, *p.image_number().range().first );
        if ( p.image_number().range().second.is_initialized() && to.is_initialized() ) {
            p.image_number().range().second = std::min(*to, *p.image_number().range().second);
        } else if ( to.is_initialized() ) {
            p.image_number().range().second = *to;
        }
        p.image_number().range().first = from;
        DEBUG("First frame of traits is " << *p.image_number().range().first << ", last frame set is " << p.image_number().range().second.is_initialized());
    }
};

template <typename Ty>
class Source<Ty>::_iterator 
  : public boost::iterator_adaptor< 
        Source<Ty>::_iterator,
        typename input::Source<Ty>::iterator >
{
    const Source<Ty>& s;
    typedef typename input::Source<Ty>::iterator Base;
    const Base end;
    mutable Ty i;

    friend class boost::iterator_core_access;
    void increment() { 
        ++this->base_reference(); 
        if ( this->base() == end )
            return;
        else if ( s.is_in_range(*this->base()) )
            i = *this->base();
        else
            this->base_reference() = end;
    }

    Ty& dereference() const { return i; }
    
  public:
    explicit _iterator(const Source<Ty>& s, const Base& from, const Base& end)
      : _iterator::iterator_adaptor_(from), s(s), end(end) 
    {
        while ( this->base() != end && ! s.is_in_range(*this->base()) )
            ++this->base_reference();
        if ( this->base() != end ) i = *this->base();
    }
};

template <class Ty>
bool Source<Ty>::is_in_range(const Ty& t) const
{
    frame_index f = ImageNumber()(t);
    bool rv = f >= from && (!to.is_initialized() || f <= *to);
    DEBUG("ROI filter returns " << rv << " for " << f);
    return rv;
}

template <typename Ty>
typename Source<Ty>::base_iterator
Source<Ty>::begin() { 
    return typename Source<Ty>::base_iterator( 
        _iterator( *this, this->base().begin(), this->base().end() ) ); 
}

template <typename Ty>
typename Source<Ty>::base_iterator
Source<Ty>::end() 
    { return typename Source<Ty>::base_iterator( 
        _iterator( *this, this->base().end(), this->base().end() ) ); }

class ChainLink 
: public input::Method<ChainLink>
{
    friend class input::Method<ChainLink>;

    Config config;
    simparm::BaseAttribute::ConnectionStore listening[2];

    typedef Localization::ImageNumber::Traits TemporalTraits;

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
  first_frame("FirstImage", "First image to load"),
  last_frame( "LastImage", "Last image to load" )
{
    first_frame.userLevel = simparm::Object::Intermediate;
    last_frame.userLevel = simparm::Object::Intermediate;
}

std::auto_ptr<input::Link> make_link() {
    return std::auto_ptr<input::Link>( new ChainLink() );
}

}
}
