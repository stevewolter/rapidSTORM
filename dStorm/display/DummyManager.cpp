#include "DummyManager.h"

namespace dStorm {
namespace display {

struct DummyManager::Handle : public WindowHandle {
    WindowProperties properties;
    DataSource* data_source;
    void store_current_display( SaveRequest ) {}
};

std::auto_ptr<Manager::WindowHandle> DummyManager::register_data_source_impl(
    const WindowProperties& properties,
    DataSource& handler)
{

    std::auto_ptr<Handle> handle;
    handle->properties = properties;
    handle->data_source = &handler;
    open_windows.push_back( handle.get() );
    return std::auto_ptr<WindowHandle>(handle.release());
}

void DummyManager::store_image_impl( std::string filename, const Change& image ) {
    this->image = image;
}

DummyManager::~DummyManager() {}

std::auto_ptr< Manager > make_dummy_manager() {
    return std::auto_ptr< Manager >( new DummyManager() );
}

}
}
