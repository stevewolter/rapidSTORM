#ifndef DSTORM_DISPLAY_DATASOURCE_H
#define DSTORM_DISPLAY_DATASOURCE_H

#include <dStorm/data-c++/Vector.h>
#include <dStorm/data-c++/VectorList.h>
#include <memory>
#include <vector>

namespace dStorm {
namespace Display {
    struct Color { uint8_t r, g, b; };
    struct ClearChange;
    struct ResizeChange;
    struct ImageChange;
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
    template <>
    class Traits<dStorm::Display::Color> 
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
struct ImageChange {
    /** Row-major array containing new values for all pixels. */
    data_cpp::Vector<Color> pixels;
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

    bool do_resize, do_clear, do_change_image;
    ResizeChange resize_image;
    ClearChange clear_image;
    ImageChange image_change;
    PixelQueue change_pixels;
    data_cpp::Vector<KeyChange> change_key;
    
    Change() { clear(); }
    void clear() { 
        do_resize = false;
        do_clear = false;
        do_change_image = false;
        change_pixels.clear();
        change_key.clear();
    }

};

class DataSource {
    public:
    DataSource() {}
    virtual ~DataSource() {}
    virtual std::auto_ptr<Change> get_changes() = 0;
    virtual void notice_closed_data_window() {}
    /** Only called when notice_drawn_rectangle is set in WindowFlags. */
    virtual void notice_drawn_rectangle
        (int, int, int, int) {}
};

}
}

#endif
