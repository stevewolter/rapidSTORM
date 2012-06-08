#ifndef SIMPARM_WX_UI_IMAGE_WINDOW_H
#define SIMPARM_WX_UI_IMAGE_WINDOW_H

#include <memory>
#include <dStorm/display/Manager.h>

namespace simparm {
namespace wx_ui {
namespace image_window {

std::auto_ptr< dStorm::display::WindowHandle >
    create( const dStorm::display::WindowProperties&, dStorm::display::DataSource& );

}
}
}

#endif
