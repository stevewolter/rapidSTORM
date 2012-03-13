#include "fwd.h"
#include "dejagnu.h"
#include "test-plugin/xenophon.h"
#include "test-plugin/cpu_time.h"
#include <dStorm/display/DummyManager.h>
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputSource.h>
#include <boost/bind/bind.hpp>

namespace dStorm {
namespace viewer {

void unit_test( TestState& state ) {
    display::DummyManager manager;

    input::Traits< output::LocalizedImage > traits = test::xenophon_traits();
    std::vector< output::LocalizedImage > data = test::xenophon();

    boost::optional<double> tick = get_cpu_time();
    std::auto_ptr<output::OutputSource> viewer_source = make_output_source();
    viewer_source->set_output_file_basename( output::Basename("foo") );
    std::auto_ptr<output::Output> viewer = viewer_source->make_output();
    output::Output::Announcement announcement( traits, manager );
    viewer->announceStormSize( announcement );
    viewer->announce_run( output::Output::RunAnnouncement() );
    for (int i = 0; i < 10; ++i) {
        std::for_each( data.begin(), data.end(),
            boost::bind( &output::Output::receiveLocalizations, 
                         viewer.get(), _1 ) );
    }
    viewer->store_results(true);
    boost::optional<double> tock = get_cpu_time();

    const display::Change& image = manager.get_stored_image();
    state( image.do_resize, "Size of image is given" );
    state( image.resize_image.size.x() == 1000 * camera::pixel, "Image has correct width" );
    state( image.resize_image.size.y() == 500 * camera::pixel, "Image has correct height" );
    state( image.resize_image.size.z() == 1 * camera::pixel, "Image has correct depth" );
    state( image.do_clear, "Background color of image is given" );
#if 0
    state( ! tick || ! tock || (*tock - *tick) < 0.1, "Image creation is sufficiently fast" );
#endif
}

}
}
