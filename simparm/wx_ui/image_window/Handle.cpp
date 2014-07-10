#include "simparm/wx_ui/image_window/ImageWindow.h"
#include "simparm/wx_ui/gui_thread.h"
#include "display/SharedDataSource.h"
#include "display/store_image.h"
#include "simparm/wx_ui/image_window/Handle.h"
#include <boost/bind/bind.hpp>

namespace simparm {
namespace wx_ui {
namespace image_window {

using dStorm::display::SharedDataSource;

Handle::Handle(dStorm::display::DataSource& m, const ProtocolNode& n)
: data_source( new SharedDataSource(m,n) ), associated_window( new Window*(NULL) )
{
}

Handle::~Handle() {
    data_source->disconnect();
}

static void create_window( 
    boost::shared_ptr<Window*> window_handle_store,
    dStorm::display::WindowProperties properties,
    boost::shared_ptr<SharedDataSource> data_source ) 
{
    *window_handle_store = new Window( properties, data_source );
}

std::auto_ptr< dStorm::display::WindowHandle >
    create( const dStorm::display::WindowProperties& props, dStorm::display::DataSource& handler, const ProtocolNode& protocol )
{
    std::auto_ptr<Handle> 
            handle(new Handle(handler, ProtocolNode( protocol, props.name ) ) );

    run_in_GUI_thread( 
        boost::bind(
            &create_window, 
            handle->associated_window,
            props,
            handle->data_source ) );

    return std::auto_ptr< dStorm::display::WindowHandle >( handle.release() );
}

static void fetch_state( boost::shared_ptr<Window*> handle, const dStorm::display::SaveRequest& r ) {
    try {
        Window& window = **handle;
        auto c = window.getState();
        if ( r.manipulator ) r.manipulator(*c);
        dStorm::display::store_image( r.filename, *c );
    } catch (const std::runtime_error& e) {
        std::cerr << "Unable to save image: " << e.what() << std::endl;
    } 
}

void Handle::store_current_display( dStorm::display::SaveRequest s )
{
    run_in_GUI_thread( boost::bind( &fetch_state, associated_window, s ) );
}


}
}
}
