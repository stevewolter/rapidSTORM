#include "debug.h"
#include "ResolutionSetter.h"
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/Context_impl.h>
#include <dStorm/engine/Image.h>
#include <dStorm/ImageTraits.h>
#include <dStorm/ImageTraits_impl.h>
#include <dStorm/input/Source_impl.h>
#include <dStorm/helpers/thread.h>

namespace dStorm {
namespace input {
namespace Resolution {

template <typename Unit>
bool similar( boost::units::quantity<Unit, float> a, boost::units::quantity<Unit, float> b )
{
    return a.value() > 0.99 * b.value() && a.value() < 1.01 * b.value();
}

bool similar( const dStorm::traits::ImageResolution & a, const dStorm::traits::ImageResolution& b )
{
    return a.unit_symbol == b.unit_symbol && similar(a.value, b.value);
}

struct DummyImageSource : simparm::Object, public Source<engine::Image>
{
    typedef Source<engine::Image>::iterator iterator;
    DummyImageSource() : simparm::Object("DummySource", "DummySource"), Source<engine::Image>(*this, Flags()) {}
    void dispatch(Messages m) {}
    iterator begin() { return iterator(); }
    iterator end() { return iterator(); }
    TraitsPtr get_traits() { return TraitsPtr( new TraitsPtr::element_type()); }
};

struct MoreSpecialized : public dStorm::input::chain::Link {
    simparm::Object node;
    ContextRef declared_context;

    MoreSpecialized() : node("Downstream", "Downstream") {}
    virtual AtEnd traits_changed( TraitsRef r, Link* ) { return notify_of_trait_change(r); }
    virtual AtEnd context_changed( ContextRef c, Link* ) { declared_context = c; return AtEnd(); }

    virtual BaseSource* makeSource() { return new DummyImageSource(); }
    virtual Link* clone() const { return new MoreSpecialized(*this); }
    virtual simparm::Node& getNode() { return node; }
    void insert_new_node( std::auto_ptr<dStorm::input::chain::Link> ) {}
};

struct LessSpecialized : public dStorm::input::chain::Forwarder {
    simparm::Object node;
    TraitsRef declared_traits;

    LessSpecialized() : node("Upstream", "Upstream") {}
    virtual AtEnd traits_changed( TraitsRef r, Link* ) { declared_traits = r; return AtEnd(); }
    virtual AtEnd context_changed( ContextRef c, Link* ) { return notify_of_context_change(c); }

    virtual BaseSource* makeSource() { return Forwarder::makeSource(); }
    virtual LessSpecialized* clone() const { return new LessSpecialized(*this); }
    virtual simparm::Node& getNode() { return node; }
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

    bool context_resolution_close_to( Resolutions r, boost::shared_ptr<const chain::Context> c ) {
        if ( ! c )
            throw std::logic_error("Context is not propagated to less specialized element");
        if ( ! c->has_info_for<dStorm::engine::Image>() )
            throw std::logic_error("Image context is not propagated to more specialized element");
        else 
            return resolution_close_to( r, c->get_info_for<dStorm::engine::Image>().plane(0).image_resolutions() );
    }

    bool trait_resolution_close_to( Resolutions r, boost::shared_ptr<const chain::MetaInfo> m ) {
        if ( ! m )
            throw std::logic_error("Meta info is not propagated to less specialized element");
        else if ( ! m->provides<engine::Image>() )
            throw std::logic_error("Image meta info is not propagated to less specialized element");
        else 
            return resolution_close_to( r, m->traits<dStorm::engine::Image>()->plane(0).image_resolutions() );
    }

    int do_check() {
        MoreSpecialized m;
        ChainLink l;
        LessSpecialized s;

        s.set_more_specialized_link_element( &l );
        l.set_more_specialized_link_element( &m );

        dStorm::input::Traits< dStorm::engine::Image > correct;
        l.config.set_traits( correct );

        DEBUG("Publishing empty context");
        chain::Context::Ptr r( new input::chain::Context() );
        s.context_changed(r, NULL);
        m.declared_context.reset();

        DEBUG("Publishing image context");
        r.reset( new input::chain::Context() );
        r->more_infos.push_back( new input::Traits<dStorm::engine::Image>() );
        s.context_changed(r, NULL);
        if ( context_resolution_close_to(correct.plane(0).image_resolutions(), m.declared_context) )
            m.declared_context.reset();

        DEBUG("Publishing image traits");
        chain::MetaInfo::Ptr tp( new input::chain::MetaInfo() );
        tp->set_traits( new Traits<engine::Image>() );
        m.traits_changed( tp, NULL );

        DEBUG("Changing context element");
        std::stringstream cmd("set 136.875,100");
        l.config.cuboid_config["InputLayer0"]["PixelSizeInNM"]["value"].processCommand(cmd);
        l.config.set_traits( correct );
        DEBUG("Checking if config element change updates context");
        if ( context_resolution_close_to(correct.plane(0).image_resolutions(), m.declared_context) )
            m.declared_context.reset();
        DEBUG("Checking if config element change updates traits");
        if ( trait_resolution_close_to(correct.plane(0).image_resolutions(), s.declared_traits) )
            s.declared_traits.reset();
        
        DEBUG("Checking if source can be built");
        std::auto_ptr<BaseSource> bs( s.makeSource() );
        std::auto_ptr< input::Source<engine::Image> > source
            = BaseSource::downcast< engine::Image >( bs );
        if ( source.get() == NULL )
            throw std::runtime_error("Source could not be built");

        resolution_close_to(correct.plane(0).image_resolutions(), source->get_traits()->plane(0).image_resolutions());
        return 0;
    }
};

}
}
}

int main() {
    ost::DebugStream::set( std::cerr );
    return dStorm::input::Resolution::Check().do_check();
}
