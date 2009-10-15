#ifndef DSTORM_TRANSMISSIONS_DISPLAY_HANDLER_H
#define DSTORM_TRANSMISSIONS_DISPLAY_HANDLER_H

#include <cc++/thread.h>
#include <memory>
#include <dStorm/Image.h>
#include <data-c++/Vector.h>

namespace dStorm {

struct Color { uint8_t r, g, b; };

class DisplayHandler : public ost::Thread {
  public:
    static DisplayHandler& getSingleton();

    struct ClearChange {
        Color background;
    };
    struct ResizeChange : public ClearChange {
        int width, height;
        int key_size;
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

    struct Change;

    class ViewportHandler {
      public:
        ViewportHandler() {}
        virtual ~ViewportHandler() {}
        virtual std::auto_ptr<Change> get_changes() = 0;
    };
    typedef int WindowID;
    virtual WindowID make_image_window
        (const std::string& name, ViewportHandler& handler,
            const ResizeChange& initial_size) = 0;
    virtual void notify_of_vanished_data_source
        ( WindowID id, bool close_window_immediately ) = 0;
    virtual void close() = 0;

  protected:
    DisplayHandler() : ost::Thread("Display handler") {}
    DisplayHandler(const DisplayHandler&);
    virtual ~DisplayHandler() {}
    DisplayHandler& operator=(const DisplayHandler&);
};

}

namespace data_cpp {
    template <>
    class Traits<dStorm::DisplayHandler::PixelChange> : public Traits<int> {};
    template <>
    class Traits<dStorm::DisplayHandler::KeyChange> : public Traits<int> {};
}

namespace dStorm {

struct DisplayHandler::Change {
    bool do_resize;
    ResizeChange resize_image;
    bool do_clear;
    ClearChange clear_image;
    data_cpp::Vector<PixelChange> change_pixels;
    data_cpp::Vector<KeyChange> change_key;
    
    Change(int pixel_buffer_size, int key_buffer_size) 
        : change_pixels(pixel_buffer_size, 0, PixelChange()),
          change_key(key_buffer_size, 0, KeyChange())
          { clear(); }
    void clear() { 
        do_resize = false;
        do_clear = false;
        change_pixels.clear();
        change_key.clear();
    }

    std::auto_ptr<Change> empty_change_with_same_allocated_size() {
        return std::auto_ptr<Change>( 
            new Change( change_pixels.capacity(), change_key.capacity()));
    }
};

}

#endif
