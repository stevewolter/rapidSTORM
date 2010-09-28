#include "DisplayManager.h"
#include "DisplayDataSource.h"
#include <dStorm/Image_impl.h>

namespace dStorm {
namespace Display {

static Manager* m;

void Manager::setSingleton(Manager& manager) {
    m = &manager;
}

Manager& Manager::getSingleton() {
    return *m;
}

void Change::make_linear_key(Image::PixelPair range) {
    if ( changed_keys.empty() ) changed_keys.push_back( data_cpp::Vector<KeyChange>() );
    Display::KeyChange *v = changed_keys.front().allocate( 256 );
    for (int i = 0; i <= 255; i++) {
        v[i].index = i;
        v[i].color = i;
        v[i].value = range.first + 
            Pixel(i * (uint8_t(range.second - range.first) / 255.0));
    }
    changed_keys.front().commit(256);
}

}
template class Image<dStorm::Pixel,2>;
}

