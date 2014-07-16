#include <boost/test/unit_test.hpp>

#include <boost/make_shared.hpp>

#include "debug.h"
#include "engine/Image.h"
#include "engine/InputTraits.h"
#include "helpers/make_unique.hpp"
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

class Check {
  public:
    typedef dStorm::image::MetaInfo<2>::Resolutions Resolutions;
    void check_resolution_close_to( Resolutions r, const Resolutions& t ) {
        BOOST_ASSERT(r[0].is_initialized());
        BOOST_ASSERT(r[1].is_initialized());
        BOOST_CHECK_CLOSE(t[0]->value.value(), r[0]->value.value(), 1E-5);
        BOOST_CHECK_CLOSE(t[1]->value.value(), r[1]->value.value(), 1E-5);
    }

    void trait_resolution_close_to( Resolutions r, boost::shared_ptr<const input::Traits<engine::ImageStack>> m ) {
        BOOST_ASSERT(m);
        check_resolution_close_to( r, m->plane(0).image.image_resolutions() );
    }

    void do_check() {
        boost::shared_ptr<simparm::text_stream::RootNode> master( new simparm::text_stream::RootNode() );
        auto testee = create();
        int trigger_count = 0;
        testee->attach_ui(master, [&]{ ++trigger_count; });

        BOOST_CHECK_EQUAL(0, trigger_count);

        Resolutions resolutions;
        resolutions[0] = traits::ImageResolution(107.0f * si::nanometre / camera::pixel);
        resolutions[1] = resolutions[0];

        auto upstream = boost::make_shared<input::Traits<engine::ImageStack>>(image::MetaInfo<2>());
        trait_resolution_close_to(resolutions, testee->make_meta_info(upstream));

        std::stringstream cmd("in Optics in InputLayer0 in PixelSizeInNM in value set 136.875,100");
        master->processCommand(cmd);
        BOOST_CHECK_EQUAL(2, trigger_count);
        resolutions[0] = traits::ImageResolution(136.875f * si::nanometre / camera::pixel);
        resolutions[1] = traits::ImageResolution(100.000f * si::nanometre / camera::pixel);
        trait_resolution_close_to(resolutions, testee->make_meta_info(upstream));
        
        auto source = testee->make_source(make_unique<DummyImageSource>());
        auto source_traits = source->get_traits();
        trait_resolution_close_to(resolutions, source_traits);
        BOOST_CHECK_CLOSE(source_traits->plane(0).optics.transmission_coefficient(0), 1.0f, 1E-7 );
    }
};

void unit_test() {
    Check().do_check();
}

}
}
}
