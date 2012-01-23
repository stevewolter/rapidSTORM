#include "Manager.h"
#include "DataSource.h"
#include <dStorm/Image_impl.h>
#include <limits>

namespace dStorm {
namespace display {

void DataSource::notice_user_key_limits(int, bool, std::string) {}

static Manager* m;

void Manager::setSingleton(Manager& manager) {
    m = &manager;
}

Manager& Manager::getSingleton() {
    return *m;
}

std::auto_ptr<Manager::WindowHandle> Manager::register_data_source
    (const WindowProperties& properties,
        DataSource& handler)
{
    return register_data_source_impl( properties, handler );
}

void Manager::store_image( std::string filename, const Change& image )
{
    assert( this );
    store_image_impl( filename, image );
}

void Change::make_linear_key(Image::PixelPair range) {
    if ( changed_keys.empty() ) changed_keys.push_back( std::vector<KeyChange>() );
    changed_keys.front().reserve( 256 );
    for (int i = 0; i <= 255; i++)
        changed_keys.front().push_back( KeyChange(
            i, Color(i),
            range.first + Pixel(i * (uint8_t(range.second - range.first) / 255.0)) ) );
}

void DataSource::look_up_key_values( const PixelInfo& info, std::vector<float>& targets )
{
    for (std::vector<float>::iterator 
         i = targets.begin(); i != targets.end(); ++i)
    {
        *i = std::numeric_limits<float>::quiet_NaN();
    }
}

void Manager::WindowHandle::store_current_display( 
    std::string filename, 
    SaveRequest::ImageManipulator manip )
{
    SaveRequest r;
    r.filename = filename;
    r.manipulator = manip;
    store_current_display(r);
}

}
}

