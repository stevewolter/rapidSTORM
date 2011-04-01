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
    dStorm::Display::Manager::setSingleton
        ( dStorm::Display::wxManager::getSingleton() );

    TestDataSource source;
    Manager::WindowProperties prop;
    prop.name = "Test name";
    prop.flags.close_window_on_unregister();
    ResizeChange size;
    size.width = 2048;
    size.height = 2048;
    size.key_size = 256;
    size.pixel_size = 0.1;
    prop.initial_size = size;

    std::auto_ptr<Manager::WindowHandle> h = 
        Manager::getSingleton().register_data_source( prop, source );

    ::wxMilliSleep( 3000 );

    h.reset( NULL );
    //dStorm::Display::wxManager::destroySingleton();

    return 0;
}
