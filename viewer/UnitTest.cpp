#include "fwd.h"
#include "dejagnu.h"
#include "test-plugin/xenophon.h"
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

    std::auto_ptr<output::OutputSource> viewer_source = make_output_source();
    viewer_source->set_output_file_basename( output::Basename("foo") );
    std::auto_ptr<output::Output> viewer = viewer_source->make_output();
    output::Output::Announcement announcement( traits, manager );
    viewer->announceStormSize( announcement );
    viewer->announce_run( output::Output::RunAnnouncement() );
    for (int i = 0; i < 3; ++i) {
        std::for_each( data.begin(), data.end(),
            boost::bind( &output::Output::receiveLocalizations, 
                         viewer.get(), _1 ) );
    }
    viewer->store_results();

    const display::Change& image = manager.get_stored_image();
    state( image.do_resize, "Size of image is given" );
    state( image.do_clear, "Size of image is given" );
    state( image.do_change_image, "Image has pending change requests" );
}

}
}
