#ifndef SIMPARM_WX_UI_IMAGE_WINDOW_HANDLE_H
#define SIMPARM_WX_UI_IMAGE_WINDOW_HANDLE_H

#include "display/Manager.h"
#include "simparm/ProtocolNode.h"

namespace simparm {
namespace wx_ui {

namespace image_window {

struct Handle
: public dStorm::display::WindowHandle
{
    boost::shared_ptr< SharedDataSource > data_source;
    boost::shared_ptr< Window* > associated_window;

    Handle( dStorm::display::DataSource&, const ProtocolNode& );
    ~Handle();

    void store_current_display( dStorm::display::SaveRequest );
};

}
}
}

#endif
