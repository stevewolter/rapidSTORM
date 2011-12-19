#include "debug.h"
#include "ROIFilter.h"
#include <simparm/Entry_Impl.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <dStorm/input/Method.hpp>
#include <dStorm/input/LocalizationTraits.h>
#include <dStorm/ImageTraits.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <boost/lexical_cast.hpp>
#include <dStorm/input/InputMutex.h>

namespace dStorm {

namespace ROIFilter {

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
    void update_traits( input::chain::MetaInfo&, input::Traits<engine::Image>& traits ) {
        set_temporal_ROI( traits.image_number() );
        if ( config.which_plane() != -1 ) {
            std::swap( traits.planes[0], traits.planes[ config.which_plane() ] );
            traits.planes.resize(1);
        }
    }
    template <typename Type>
    void update_traits( input::chain::MetaInfo&, input::Traits<Type>& traits ) 
        { set_temporal_ROI( traits.image_number() ); }

    void notice_traits( const input::chain::MetaInfo&, const input::Traits<engine::Image>& t ) {
        for (int i = config.which_plane.numChoices()-1; i < t.plane_count(); ++i) {
            std::string id = boost::lexical_cast<std::string>(i);
            config.which_plane.addChoice( i, "Plane" + id, "Plane " + id );
        }
        for (int i = t.plane_count(); i < config.which_plane.numChoices()-1; ++i)
            config.which_plane.removeChoice( i );
    }
    template <typename Type>
    void notice_traits( const input::chain::MetaInfo&, const input::Traits<Type>& ) {}

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

std::auto_ptr<input::chain::Link> makeFilter() 
{
    return std::auto_ptr<input::chain::Link> (new ChainLink());
}

void ChainLink::operator()( const simparm::Event& ) {
    ost::MutexLock lock( input::global_mutex() );
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


}
}
