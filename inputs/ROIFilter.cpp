#include "debug.h"
#include "ROIFilter.h"

#include <simparm/BoostUnits.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Entry_Impl.hh>
#include <simparm/Structure.hh>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/units/io.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>
#include <dStorm/image/extend.h>
#include <dStorm/Image.h>
#include <dStorm/image/slice.h>
#include <dStorm/ImageTraits.h>
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

struct Config : public simparm::Object {
    IntFrameEntry first_frame;
    simparm::Entry< boost::optional< frame_index > > last_frame;
    simparm::ChoiceEntry which_plane;

    Config();
    void registerNamedEntries() { 
        push_back( first_frame ); 
        push_back( last_frame ); 
        push_back( which_plane );
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
    const boost::optional<int> plane;
    typedef typename input::Source<Ty>::iterator base_iterator;

    struct _iterator;

    inline bool is_in_range(const Ty& t) const;

    void reduce_planes( input::Traits<engine::Image>& t ) {
        if ( plane.is_initialized() ) {
            std::swap(t.planes[0], t.planes[*plane]);
            t.planes.resize(1);
            t.size.z() = 1 * camera::pixel;
        }
    }
    template <class Other>
    void reduce_planes( input::Traits<Other>& ) {}

  public:
    Source( std::auto_ptr< input::Source<Ty> > upstream,
            frame_index from, boost::optional<frame_index> to, boost::optional<int> plane)
        : input::AdapterSource<Ty>(upstream), from(from), to(to),
          plane(plane) {}
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
        reduce_planes(p);
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
            select_plane();
        else
            this->base_reference() = end;
    }

    void select_plane();

    Ty& dereference() const { return i; }
    
  public:
    explicit _iterator(const Source<Ty>& s, const Base& from, const Base& end)
      : _iterator::iterator_adaptor_(from), s(s), end(end) 
    {
        while ( this->base() != end && ! s.is_in_range(*this->base()) )
            ++this->base_reference();
        if ( this->base() != end ) select_plane();
    }
};

template <>
void Source< dStorm::engine::Image >::_iterator::select_plane()
{
    if ( s.plane.is_initialized() ) {
        dStorm::Image< unsigned short, 2 > slice
            = base()->slice(2, *s.plane * camera::pixel );
        i = extend( slice, engine::Image() );
    } else {
        i = *this->base();
    }
}

template <class Ty>
void Source<Ty>::_iterator::select_plane() { i = *this->base(); }

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
: public input::Method<ChainLink>, public simparm::Listener
{
    friend class input::Method<ChainLink>;

    simparm::Structure<Config> config;
    simparm::Structure<Config>& get_config() { return config; }
    void operator()( const simparm::Event& );

    typedef Localization::ImageNumber::Traits TemporalTraits;

    void set_temporal_ROI( TemporalTraits& t ) {
        t.range().first = config.first_frame();
        if ( config.last_frame().is_initialized() )
            t.range().second = config.last_frame();
    }
    void update_traits( input::MetaInfo&, input::Traits<engine::Image>& traits ) {
        set_temporal_ROI( traits.image_number() );
        if ( config.which_plane() != -1 ) {
            std::swap( traits.planes[0], traits.planes[ config.which_plane() ] );
            traits.planes.resize(1);
        }
    }
    template <typename Type>
    void update_traits( input::MetaInfo&, input::Traits<Type>& traits ) 
        { set_temporal_ROI( traits.image_number() ); }

    void notice_traits( const input::MetaInfo&, const input::Traits<engine::Image>& t ) {
        for (int i = config.which_plane.numChoices()-1; i < t.plane_count(); ++i) {
            std::string id = boost::lexical_cast<std::string>(i);
            config.which_plane.addChoice( i, "Plane" + id, "Plane " + id );
        }
        for (int i = t.plane_count(); i < config.which_plane.numChoices()-1; ++i)
            config.which_plane.removeChoice( i );
    }
    template <typename Type>
    void notice_traits( const input::MetaInfo&, const input::Traits<Type>& ) {}

    template <typename Type>
    input::Source<Type>* make_source( std::auto_ptr< input::Source<Type> > p ) {
        boost::optional<int> plane;
        if ( config.which_plane() != -1 ) plane = config.which_plane();
        if ( config.first_frame() > 0 * camera::frame 
            || config.last_frame().is_initialized()
            || plane.is_initialized() )
        {
            return new Source<Type>( p, config.first_frame(), config.last_frame(), 
                               plane );
        } else
            return p.release();
    }

  public:
    ChainLink();
    ChainLink(const ChainLink&);
    simparm::Node& getNode() { return config; }
};

Config::Config() 
: simparm::Object("ROIFilter", "Image selection filter"),
  first_frame("FirstImage", "First image to load"),
  last_frame( "LastImage", "Last image to load" ),
  which_plane( "OnlyPlane", "Process only given plane" )
{
    which_plane.addChoice(-1, "AllPlanes", "All planes");
    first_frame.userLevel = simparm::Object::Intermediate;
    last_frame.userLevel = simparm::Object::Intermediate;
    which_plane.userLevel = simparm::Object::Expert;
}

void ChainLink::operator()( const simparm::Event& ) {
    input::InputMutexGuard lock( input::global_mutex() );
    republish_traits();
}

ChainLink::ChainLink()
: simparm::Listener( simparm::Event::ValueChanged )
{
    receive_changes_from( config.which_plane.value );
}

ChainLink::ChainLink(const ChainLink& o)
: input::Method<ChainLink>(o), simparm::Listener( simparm::Event::ValueChanged ),
  config(o.config)
{
    receive_changes_from( config.which_plane.value );
}

std::auto_ptr<input::Link> make_link() {
    return std::auto_ptr<input::Link>( new ChainLink() );
}

}
}
