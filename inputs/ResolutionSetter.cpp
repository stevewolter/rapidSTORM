#include "debug.h"
#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include "ResolutionSetter.h"
#include <dStorm/input/Source_impl.h>
#include <dStorm/engine/Image.h>
#include <dStorm/Localization.h>
#include <dStorm/input/LocalizationTraits.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/ImageTraits_impl.h>
#include <boost/lexical_cast.hpp>
#include <boost/lexical_cast.hpp>
#include <simparm/ChoiceEntry_Impl.hh>
#include <dStorm/input/ResolutionChange.h>
#include <dStorm/input/Method.hpp>
#include "dejagnu.h"

namespace dStorm {
namespace input {
namespace resolution {

class ChainLink 
: public input::Method<ChainLink>, public simparm::TreeListener 
{
    friend class Check;
    friend class input::Method<ChainLink>;

    simparm::Structure<SourceConfig> config;
    simparm::Structure<SourceConfig>& get_config() { return config; }

    class TraitMaker;

    void operator()(const simparm::Event&);

    template <typename Type>
    Source<Type>* make_source( std::auto_ptr< input::Source<Type> > upstream ) { 
        return new resolution::Source<Type>(upstream, config); 
    }
    template <typename Type>
    void update_traits( chain::MetaInfo& i, Traits<Type>& traits ) { 
        i.get_signal< ResolutionChange >()( config.get_resolution() );
        config.read_traits( traits );
        config.set_traits(traits); 
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
	ost::MutexLock lock( global_mutex() );
        republish_traits();
    } else 
	TreeListener::add_new_children(e);
}

std::auto_ptr<chain::Link> makeLink() {
    DEBUG("Making resolution chain link");
    return std::auto_ptr<chain::Link>( new ChainLink() );
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

struct DummyImageSource : public input::Source<engine::Image>
{
    simparm::Object foo;
    DummyImageSource() : foo("Foo", "Foo") {}
    simparm::Node& node() { return foo; }
    typedef Source<engine::Image>::iterator iterator;
    void dispatch(Messages m) {}
    iterator begin() { return iterator(); }
    iterator end() { return iterator(); }
    TraitsPtr get_traits( Wishes ) { return TraitsPtr( new TraitsPtr::element_type()); }
    Capabilities capabilities() const { return Capabilities(); }
};

struct MoreSpecialized : public dStorm::input::chain::Link {
    simparm::Object node;

    MoreSpecialized() : node("Downstream", "Downstream") {}
    virtual AtEnd traits_changed( TraitsRef r, Link* ) { return notify_of_trait_change(r); }

    virtual input::BaseSource* makeSource() { return new DummyImageSource(); }
    virtual Link* clone() const { return new MoreSpecialized(*this); }
    void insert_new_node( std::auto_ptr<dStorm::input::chain::Link>, Place ) {}
    void registerNamedEntries( simparm::Node& ) { }
    std::string name() const { return node.getName(); }
    std::string description() const { return node.getDesc(); }
};

struct LessSpecialized : public dStorm::input::chain::Forwarder {
    simparm::Object node;
    TraitsRef declared_traits;

    LessSpecialized() : node("Upstream", "Upstream") {}
    virtual AtEnd traits_changed( TraitsRef r, Link* ) { declared_traits = r; return AtEnd(); }

    virtual input::BaseSource* makeSource() { return Forwarder::makeSource(); }
    virtual LessSpecialized* clone() const { return new LessSpecialized(*this); }
    void registerNamedEntries( simparm::Node& ) { }
    std::string name() const { return node.getName(); }
    std::string description() const { return node.getDesc(); }
};

struct Check {
    typedef dStorm::traits::Optics<2>::Resolutions Resolutions;
    bool resolution_close_to( Resolutions r, const Resolutions& t ) {
        if ( ! r[0].is_initialized() || ! r[1].is_initialized() )
            throw std::logic_error("Resolution is not set at all");
        if ( ! similar( *t[0], *r[0] ) || ! similar( *t[1], *r[1] ) )
            throw std::logic_error("Resolution is not set correctly");
        else 
            return true;
    }

    bool trait_resolution_close_to( Resolutions r, boost::shared_ptr<const input::chain::MetaInfo> m ) {
        if ( ! m )
            throw std::logic_error("Meta info is not propagated to less specialized element");
        else if ( ! m->provides<engine::Image>() )
            throw std::logic_error("Image meta info is not propagated to less specialized element");
        else 
            return resolution_close_to( r, m->traits<dStorm::engine::Image>()->plane(0).image_resolutions() );
    }

    int do_check() {
        MoreSpecialized m;
        input::resolution::ChainLink l;
        LessSpecialized s;

        s.set_more_specialized_link_element( &l );
        l.set_more_specialized_link_element( &m );

        dStorm::input::Traits< dStorm::engine::Image > correct;
        l.config.set_traits( correct );

        DEBUG("Publishing image traits");
        input::chain::MetaInfo::Ptr tp( new input::chain::MetaInfo() );
        tp->set_traits( new input::Traits<engine::Image>() );
        m.traits_changed( tp, NULL );

        DEBUG("Changing context element");
        std::stringstream cmd("set 136.875,100");
        l.config["Optics"]["InputLayer0"]["PixelSizeInNM"]["value"].processCommand(cmd);
        l.config.set_traits( correct );
        DEBUG("Checking if config element change updates traits");
        if ( trait_resolution_close_to(correct.plane(0).image_resolutions(), s.declared_traits) )
            s.declared_traits.reset();
        
        DEBUG("Checking if source can be built");
        std::auto_ptr<input::BaseSource> bs( s.makeSource() );
        std::auto_ptr< input::Source<engine::Image> > source
            = input::BaseSource::downcast< engine::Image >( bs );
        if ( source.get() == NULL )
            throw std::runtime_error("Source could not be built");

        boost::shared_ptr< const dStorm::input::Traits<engine::Image> > source_traits
            = source->get_traits( dStorm::input::BaseSource::Wishes() );

        resolution_close_to(correct.plane(0).image_resolutions(), 
            source_traits->plane(0).image_resolutions());
        assert( source_traits->plane(0).transmission_coefficient(0) == 1.0f );
        return 0;
    }
};

void unit_test( TestState& state ) {
    state.testrun( Check().do_check() == 0, "Resolution setter works");
}

}
}
}

