#include <simparm/Eigen_decl.h>
#include <simparm/BoostUnits.h>

#include "ResolutionSetter.h"
#include "debug.h"

#include <boost/lexical_cast.hpp>
#include <dStorm/engine/Image.h>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/input/Link.h>
#include <dStorm/input/MetaInfo.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/localization/Traits.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/input/Source.h>
#include <dStorm/signals/ResolutionChange.h>
#include <dStorm/Localization.h>
#include <dStorm/traits/optics_config.h>
#include <dStorm/units/nanolength.h>
#include <simparm/Eigen.h>
#include <simparm/text_stream/RootNode.h>
#include <simparm/dummy_ui/fwd.h>
#include "dejagnu.h"

namespace dStorm {
namespace input {
namespace resolution {

struct Config : public traits::MultiPlaneConfig {
    Config() : traits::MultiPlaneConfig( traits::PlaneConfig::FitterConfiguration ) {}
};

template <typename ForwardedType>
class Source 
: public input::AdapterSource<ForwardedType>
{
    Config config;

    void modify_traits( input::Traits<engine::ImageStack>& t ) { 
        config.write_traits(t); 
        for (int p = 0; p < t.plane_count(); ++p)
            t.plane(p).create_projection();
    }
    template <typename OtherTypes>
    void modify_traits( input::Traits<OtherTypes>& t ) { 
        config.write_traits(t); 
    }
    void attach_local_ui_( simparm::NodeHandle ) {}

  public:
    Source(
        std::auto_ptr< input::Source<ForwardedType> > backend,
        const Config& config ) 
        : input::AdapterSource<ForwardedType>( backend ), config(config) { 
            simparm::NodeHandle n = simparm::dummy_ui::make_node();
            this->config.attach_ui( n ); 
        }
};

class ChainLink 
: public input::Method<ChainLink>
{
    friend class Check;
    friend class input::Method<ChainLink>;

    Config config;
    void attach_ui( simparm::NodeHandle at ) { config.attach_ui( at ); }
    static std::string getName() { return "Optics"; }

    template <typename Type>
    Source<Type>* make_source( std::auto_ptr< input::Source<Type> > upstream ) { 
        return new resolution::Source<Type>(upstream, config); 
    }
    template <typename Type>
    void update_traits( MetaInfo& i, Traits<Type>& traits ) { 
        i.get_signal< signals::ResolutionChange >()( config.get_resolution() );
        config.set_context( traits );
        config.write_traits(traits); 
    }
    void republish_traits();

  public:
    ChainLink();
    ChainLink(const ChainLink&);
};

ChainLink::ChainLink() 
{
    config.notify_on_any_change( boost::bind( &ChainLink::republish_traits, this ) );
}

ChainLink::ChainLink(const ChainLink& o) 
: input::Method<ChainLink>(o),
  config(o.config)
{
    config.notify_on_any_change( boost::bind( &ChainLink::republish_traits, this ) );
}

void ChainLink::republish_traits()
{
    InputMutexGuard lock( global_mutex() );
    input::Method<ChainLink>::republish_traits();
}

std::auto_ptr<Link> makeLink() {
    DEBUG("Making resolution chain link");
    return std::auto_ptr<Link>( new ChainLink() );
}

template <typename Unit>
bool similar( boost::units::quantity<Unit, float> a, boost::units::quantity<Unit, float> b )
{
    return a.value() > 0.99 * b.value() && a.value() < 1.01 * b.value();
}

bool similar( const dStorm::traits::ImageResolution & a, const dStorm::traits::ImageResolution& b )
{
    return a.unit_symbol == b.unit_symbol && similar(a.value, b.value);
}

struct DummyImageSource : public input::Source<engine::ImageStack>
{
    DummyImageSource() {}
    void attach_ui_( simparm::NodeHandle ) {}
    void dispatch(Messages m) {}
    void set_thread_count(int num_threads) {}
    bool GetNext(int thread, engine::ImageStack* target) OVERRIDE { return false; }
    TraitsPtr get_traits( Wishes ) { 
        return TraitsPtr( 
            new TraitsPtr::element_type(
                image::MetaInfo<2>()
            )
        ); 
    }
    Capabilities capabilities() const { return Capabilities(); }
};

struct MoreSpecialized : public dStorm::input::Link {
    simparm::Object node;

    MoreSpecialized() : node("Downstream", "Downstream") {}
    virtual void traits_changed( TraitsRef r, Link* ) { return update_current_meta_info(r); }

    virtual input::BaseSource* makeSource() { return new DummyImageSource(); }
    virtual Link* clone() const { return new MoreSpecialized(*this); }
    void insert_new_node( std::auto_ptr<dStorm::input::Link>, Place ) {}
    void registerNamedEntries( simparm::NodeHandle ) { }
    std::string name() const { return node.getName(); }
    std::string description() const { return node.getDesc(); }
    void publish_meta_info() {}
};

struct Check {
    typedef dStorm::image::MetaInfo<2>::Resolutions Resolutions;
    bool resolution_close_to( Resolutions r, const Resolutions& t ) {
        if ( ! r[0].is_initialized() || ! r[1].is_initialized() )
            throw std::logic_error("Resolution is not set at all");
        if ( ! similar( *t[0], *r[0] ) || ! similar( *t[1], *r[1] ) ) {
            throw std::logic_error("Resolution is not set correctly");
        } else 
            return true;
    }

    bool trait_resolution_close_to( Resolutions r, boost::shared_ptr<const input::MetaInfo> m ) {
        if ( ! m )
            throw std::logic_error("Meta info is not propagated to less specialized element");
        else if ( ! m->provides<engine::ImageStack>() )
            throw std::logic_error("Image meta info is not propagated to less specialized element");
        else 
            return resolution_close_to( r, m->traits<engine::ImageStack>()->plane(0).image.image_resolutions() );
    }

    int do_check() {
        boost::shared_ptr<simparm::text_stream::RootNode> master( new simparm::text_stream::RootNode() );
        std::auto_ptr<MoreSpecialized> ms( new MoreSpecialized() );
        MoreSpecialized& m(*ms);
        input::resolution::ChainLink l;

        l.insert_new_node( std::auto_ptr<input::Link>(ms), Anywhere );

        dStorm::input::Traits< engine::ImageStack > correct( ( image::MetaInfo<2>() ) );
        l.config.write_traits( correct );

        DEBUG("Publishing image traits");
        input::MetaInfo::Ptr tp( new input::MetaInfo() );
        tp->set_traits( new input::Traits<engine::ImageStack>( (image::MetaInfo<2>()) ) );
        m.traits_changed( tp, NULL );

        DEBUG("Changing context element");
        l.config.attach_ui( master );
        std::stringstream cmd("in Optics in InputLayer0 in PixelSizeInNM in value set 136.875,100");
        master->processCommand(cmd);
        l.config.write_traits( correct );
        DEBUG("Checking if config element change updates traits");
        if ( trait_resolution_close_to(correct.plane(0).image.image_resolutions(), l.current_meta_info()) )
            l.current_meta_info().reset();
        
        DEBUG("Checking if source can be built");
        std::auto_ptr<input::BaseSource> bs( static_cast<input::Link&>(l).make_source() );
        std::auto_ptr< input::Source<engine::ImageStack> > source
            = input::BaseSource::downcast< engine::ImageStack >( bs );
        if ( source.get() == NULL )
            throw std::runtime_error("Source could not be built");

        boost::shared_ptr< const dStorm::input::Traits<engine::ImageStack> > source_traits
            = source->get_traits( dStorm::input::BaseSource::Wishes() );

        resolution_close_to(correct.plane(0).image.image_resolutions(), 
            source_traits->plane(0).image.image_resolutions());
        assert( source_traits->plane(0).optics.transmission_coefficient(0) == 1.0f );
        return 0;
    }
};

void unit_test( TestState& state ) {
    state.testrun( Check().do_check() == 0, "Resolution setter works");
}

}
}
}

