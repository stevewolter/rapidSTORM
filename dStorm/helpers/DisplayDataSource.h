#ifndef DSTORM_DISPLAY_DATASOURCE_H
#define DSTORM_DISPLAY_DATASOURCE_H

#include <dStorm/data-c++/Vector.h>
#include <dStorm/data-c++/VectorList.h>
#include <memory>

namespace dStorm {
namespace Display {
    struct Color { uint8_t r, g, b; };
    struct ClearChange;
    struct ResizeChange;
    struct PixelChange;
    struct KeyChange;
}
}

namespace data_cpp {
    template <>
    class Traits<dStorm::Display::PixelChange> 
        : public Traits<int> {};
    template <>
    class Traits<dStorm::Display::KeyChange> 
        : public Traits<int> {};
}

namespace dStorm {
namespace Display {

struct ClearChange {
    Color background;
};
struct ResizeChange {
    int width, height;
    int key_size;
    /* Size of one pixel in meters. */
    float pixel_size; 
};
struct PixelChange { 
    int x, y; 
    Color color;
};
struct KeyChange {
    int index;
    Color color;
    float value;
};

struct Change {
    typedef data_cpp::VectorList<PixelChange> PixelQueue;

    bool do_resize, do_clear;
    ResizeChange resize_image;
    ClearChange clear_image;
    PixelQueue change_pixels;
    data_cpp::Vector<KeyChange> change_key;
    
    Change() { clear(); }
    void clear() { 
        do_resize = false;
        do_clear = false;
        change_pixels.clear();
        change_key.clear();
    }

};

class DataSource {
    public:
    DataSource() {}
    virtual ~DataSource() {}
    virtual std::auto_ptr<Change> get_changes() = 0;
};

}
}

#endif
