#include <dStorm/helpers/DisplayManager.h>
#include "wxManager.h"
#include <wx/wx.h>

using namespace dStorm::Display;

class TestDataSource : public DataSource {
    std::auto_ptr<Change> get_changes() {
        return std::auto_ptr<Change>(new Change());
    }
};

int main() {
    std::auto_ptr<dStorm::Display::Manager> singleton
        ( new dStorm::Display::wxManager() );
    dStorm::Display::Manager::setSingleton( *singleton );

    TestDataSource source;
    Manager::WindowProperties prop;
    prop.name = "Test name";
    prop.flags.close_window_on_unregister();
    ResizeChange size;
    size.size.x() = 2048 * camera::pixel;
    size.size.y() = 2048 * camera::pixel;
    size.key_size = 256;
    size.pixel_size = 1E5 * camera::pixels_per_meter;
    prop.initial_size = size;

    std::auto_ptr<Manager::WindowHandle> h = 
        Manager::getSingleton().register_data_source( prop, source );

    ::wxMilliSleep( 3000 );

    h.reset( NULL );
    singleton.reset(NULL);

    return 0;
}
