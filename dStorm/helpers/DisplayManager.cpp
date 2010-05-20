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
    Display::KeyChange *v = change_key.allocate( 256 );
    for (int i = 0; i <= 255; i++) {
        v[i].index = i;
        v[i].color = i;
        v[i].value = range.first + 
            Pixel(i * (uint8_t(range.second - range.first) / 255.0));
    }
    change_key.commit(256);
}

}
template class Image<dStorm::Pixel,2>;
}

