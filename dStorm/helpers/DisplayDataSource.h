#ifndef DSTORM_DISPLAY_DATASOURCE_H
#define DSTORM_DISPLAY_DATASOURCE_H

#include <dStorm/data-c++/Vector.h>
#include <dStorm/data-c++/VectorList.h>
#include <memory>
#include <vector>
#include <dStorm/Pixel.h>
#include <dStorm/Image.h>
#include <cs_units/camera/resolution.hpp>
#include <cs_units/camera/length.hpp>

namespace dStorm {
namespace Display {
    typedef dStorm::Pixel Color;
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

typedef dStorm::Image<dStorm::Pixel,2> Image;

struct ClearChange {
    Color background;
};
struct ResizeChange {
    Image::Size size;
    int key_size;
    /* Size of one pixel in meters. */
    typedef boost::units::quantity<cs_units::camera::resolution,float> 
        Resolution;
    Resolution pixel_size; 
};
struct ImageChange {
    Image new_image;
};
struct PixelChange { 
    int x, y; 
    Color color;

    PixelChange(int x, int y) : x(x), y(y) {}
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

    void make_linear_key(Image::PixelPair range);
    template <class PixelType>
    void display_normalized(const dStorm::Image<PixelType,2>&);
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
