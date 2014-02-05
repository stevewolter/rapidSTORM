#ifndef DSTORM_DISPLAY_DATASOURCE_H
#define DSTORM_DISPLAY_DATASOURCE_H

#include "dStorm/display/fwd.h"

#include <memory>
#include <vector>
#include <list>
#include <dStorm/Pixel.h>
#include "dStorm/image/Image.h"
#include <boost/units/systems/camera/resolution.hpp>
#include <boost/units/systems/camera/length.hpp>
#include <dStorm/image/MetaInfo.h>
#include <dStorm/image/constructors.h>

namespace dStorm {
namespace display {

typedef dStorm::Image<dStorm::Pixel,3> Image;

struct ClearChange {
    Color background;
};
struct KeyDeclaration {
    std::string unit, description;
    int size;
    bool can_set_lower_limit, can_set_upper_limit;
    std::string lower_limit, upper_limit;

    KeyDeclaration(std::string unit, std::string description, int size)
        : unit(unit), description(description), size(size),
          can_set_lower_limit(false), can_set_upper_limit(false) {}
};

struct ResizeChange {
    Image::Size size;
    std::vector<KeyDeclaration> keys;
    /* Size of one pixel in meters. */
    typedef dStorm::traits::ImageResolution Resolution;
    Resolution pixel_sizes[Image::Dim]; 

    void set_size( const ImageTypes<3>::Size& sz ) { size = sz; }
    void set_size( const ImageTypes<2>::Size& sz ) 
        { size.fill( 1 * camera::pixel ); size.head<2>() = sz; }
};
struct ImageChange {
    Image new_image;
};
struct PixelChange : public Image::Position { 
    Color color;

    PixelChange(Image::Position p) : Image::Position(p) {}
    PixelChange(ImageTypes<2>::Position p) {
        Image::Position::fill(0);
        Image::Position::head<2>() = p;
    }
};
struct KeyChange {
    int index;
    Color color;
    float value;

    /** Default constructor leaves values uninitialized */
    KeyChange() {}
    KeyChange( int index, Color color, float value )
        : index(index), color(color), value(value) {}
    
    static std::vector<KeyChange> make_linear_key( std::pair<float,float> range );
};

struct Change {
    typedef std::vector<PixelChange> PixelQueue;
    typedef std::vector< std::vector<KeyChange> > Keys;

    bool do_resize, do_clear, do_change_image;
    ResizeChange resize_image;
    ClearChange clear_image;
    ImageChange image_change;
    PixelQueue change_pixels;
    Keys changed_keys;
    
    Change(int key_count) : changed_keys(key_count) { clear(); }
    void clear() { 
        do_resize = false;
        do_clear = false;
        do_change_image = false;
        change_pixels.clear();
        for (unsigned int i = 0; i < changed_keys.size(); ++i)
            changed_keys[i].clear();
    }

    void make_linear_key(Image::PixelPair range);
};

struct DataSource {
    DataSource() {}
    virtual ~DataSource() {}
    virtual std::auto_ptr<Change> get_changes() = 0;
    virtual void notice_closed_data_window() {}
    /** Only called when notice_drawn_rectangle is set in WindowFlags. */
    virtual void notice_drawn_rectangle
        (int, int, int, int) {}
    virtual void notice_user_key_limits(int key_index, bool lower, std::string input);

    struct PixelInfo : public Image::Position {
        const Color& pixel;
        PixelInfo( Image::Position pos, const Color& p) 
            : Image::Position(pos), pixel(p) {}
    };
    /** This method should fill the fields in the vector \c targets, which has
     *  one pre-allocated field for each declared key, with the value displayed
     *  in the pixel defined by the \c info structure. If the value is not known
     *  any more, the vector field should be set to NaN. */
    virtual void look_up_key_values( const PixelInfo& info,
                                     std::vector<float>& targets );
};

}
}

#endif
