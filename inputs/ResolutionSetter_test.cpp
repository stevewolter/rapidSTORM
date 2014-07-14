#include <boost/test/unit_test.hpp>

#include "debug.h"
#include "engine/Image.h"
#include "engine/InputTraits.h"
#include "input/Link.h"
#include "input/MetaInfo.h"
#include "input/Source.h"
#include "inputs/ResolutionSetter.h"
#include "simparm/text_stream/RootNode.h"

namespace dStorm {
namespace input {
namespace resolution {

struct DummyImageSource : public input::Source<engine::ImageStack>
{
    DummyImageSource() {}
    void attach_ui_( simparm::NodeHandle ) {}
    void dispatch(Messages m) {}
    void set_thread_count(int num_threads) {}
    bool GetNext(int thread, engine::ImageStack* target) OVERRIDE { return false; }
    TraitsPtr get_traits() { 
        return TraitsPtr( 
            new TraitsPtr::element_type(
                image::MetaInfo<2>()
            )
        ); 
    }
};

struct MoreSpecialized : public dStorm::input::Link {
    simparm::Object node;

    MoreSpecialized() : node("Downstream", "Downstream") {}
    virtual void traits_changed( TraitsRef r, Link* ) { return update_current_meta_info(r); }

    virtual input::BaseSource* makeSource() { return new DummyImageSource(); }
    virtual Link* clone() const { return new MoreSpecialized(*this); }
    void insert_new_node( std::auto_ptr<dStorm::input::Link> ) {}
    void registerNamedEntries( simparm::NodeHandle ) { }
    std::string name() const { return node.getName(); }
    std::string description() const { return node.getDesc(); }
    void publish_meta_info() {}
};

class Check {
  public:
    typedef dStorm::image::MetaInfo<2>::Resolutions Resolutions;
    void check_resolution_close_to( Resolutions r, const Resolutions& t ) {
        BOOST_ASSERT(r[0].is_initialized());
        BOOST_ASSERT(r[1].is_initialized());
        BOOST_CHECK_CLOSE(t[0]->value.value(), r[0]->value.value(), 1E-5);
        BOOST_CHECK_CLOSE(t[1]->value.value(), r[1]->value.value(), 1E-5);
    }

    void trait_resolution_close_to( Resolutions r, boost::shared_ptr<const input::MetaInfo> m ) {
        BOOST_ASSERT(m);
        BOOST_ASSERT(m->provides<engine::ImageStack>());
        check_resolution_close_to( r, m->traits<engine::ImageStack>()->plane(0).image.image_resolutions() );
    }

    void do_check() {
        boost::shared_ptr<simparm::text_stream::RootNode> master( new simparm::text_stream::RootNode() );
        std::auto_ptr<MoreSpecialized> ms( new MoreSpecialized() );
        MoreSpecialized& m(*ms);
        std::auto_ptr<Link> l(makeLink());

        l->insert_new_node( std::auto_ptr<input::Link>(ms) );
        l->publish_meta_info();

        dStorm::input::Traits< engine::ImageStack > correct(
                *l->current_meta_info()->traits<engine::ImageStack>());

        DEBUG("Publishing image traits");
        input::MetaInfo::Ptr tp( new input::MetaInfo() );
        tp->set_traits( new input::Traits<engine::ImageStack>( (image::MetaInfo<2>()) ) );
        m.traits_changed( tp, NULL );

        DEBUG("Changing context element");
        l->registerNamedEntries( master );
        std::stringstream cmd("in Optics in InputLayer0 in PixelSizeInNM in value set 136.875,100");
        master->processCommand(cmd);
        DEBUG("Checking if config element change updates traits");
        trait_resolution_close_to(correct.plane(0).image.image_resolutions(), l->current_meta_info());
        l->current_meta_info().reset();
        
        DEBUG("Checking if source can be built");
        std::auto_ptr<input::BaseSource> bs( l->make_source() );
        std::auto_ptr< input::Source<engine::ImageStack> > source
            = input::BaseSource::downcast< engine::ImageStack >( bs );
        if ( source.get() == NULL )
            throw std::runtime_error("Source could not be built");

        boost::shared_ptr< const dStorm::input::Traits<engine::ImageStack> > source_traits
            = source->get_traits();

        check_resolution_close_to(correct.plane(0).image.image_resolutions(), 
            source_traits->plane(0).image.image_resolutions());
        BOOST_CHECK_CLOSE(source_traits->plane(0).optics.transmission_coefficient(0), 1.0f, 1E-7 );
    }
};

void unit_test() {
    Check().do_check();
}

}
}
}
