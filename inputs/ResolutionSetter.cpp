#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>

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
#include <dStorm/traits/resolution_config.h>
#include <dStorm/units/nanolength.h>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Eigen.hh>
#include <simparm/Structure.hh>
#include <simparm/TreeCallback.hh>
#include "dejagnu.h"

namespace dStorm {
namespace input {
namespace resolution {

class Config : public traits::resolution::Config {};

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
  public:
    Source(
        std::auto_ptr< input::Source<ForwardedType> > backend,
        const Config& config ) 
        : input::AdapterSource<ForwardedType>( backend ), config(config) {}
};

class ChainLink 
: public input::Method<ChainLink>, public simparm::TreeListener 
{
    friend class Check;
    friend class input::Method<ChainLink>;

    simparm::Structure<Config> config;

    void operator()(const simparm::Event&);

    template <typename Type>
    Source<Type>* make_source( std::auto_ptr< input::Source<Type> > upstream ) { 
        return new resolution::Source<Type>(upstream, config); 
    }
    template <typename Type>
    void update_traits( MetaInfo& i, Traits<Type>& traits ) { 
        i.get_signal< signals::ResolutionChange >()( config.get_resolution() );
        config.read_plane_count( traits );
        config.write_traits(traits); 
    }

  public:
    ChainLink();
    ChainLink(const ChainLink&);
    simparm::Node& getNode() { return config; }
};

ChainLink::ChainLink() 
{
    DEBUG("Making ResolutionSetter chain link");
    receive_changes_from_subtree( config );
}

ChainLink::ChainLink(const ChainLink& o) 
: input::Method<ChainLink>(o),
  config(o.config)
{
    receive_changes_from_subtree( config );
}

void ChainLink::operator()(const simparm::Event& e)
{
    if ( e.cause == simparm::Event::ValueChanged) {
	InputMutexGuard lock( global_mutex() );
        republish_traits();
    } else 
	TreeListener::add_new_children(e);
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
    simparm::Object foo;
    DummyImageSource() : foo("Foo", "Foo") {}
    simparm::Node& node() { return foo; }
    typedef Source<engine::ImageStack>::iterator iterator;
    void dispatch(Messages m) {}
    iterator begin() { return iterator(); }
    iterator end() { return iterator(); }
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
    void registerNamedEntries( simparm::Node& ) { }
    std::string name() const { return node.getName(); }
    std::string description() const { return node.getDesc(); }
    void publish_meta_info() {}
};

struct Check {
    typedef dStorm::image::MetaInfo<2>::Resolutions Resolutions;
    bool resolution_close_to( Resolutions r, const Resolutions& t ) {
        if ( ! r[0].is_initialized() || ! r[1].is_initialized() )
            throw std::logic_error("Resolution is not set at all");
        if ( ! similar( *t[0], *r[0] ) || ! similar( *t[1], *r[1] ) )
            throw std::logic_error("Resolution is not set correctly");
        else 
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
        std::stringstream cmd("set 136.875,100");
        l.config["Optics"]["InputLayer0"]["PixelSizeInNM"]["value"].processCommand(cmd);
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

