#ifndef DSTORM_TRANSMISSIONS_DISPLAY_HANDLER_H
#define DSTORM_TRANSMISSIONS_DISPLAY_HANDLER_H

#include <cc++/thread.h>
#include <memory>
#include <dStorm/engine/Image.h>
#include <data-c++/Vector.h>

namespace dStorm {
    struct Pixel { 
        int x, y; uint8_t r, g, b; 
        Pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
            : x(x), y(y), r(r), g(g), b(b) {}
    };
}

namespace data_cpp {
    template <>
    class Traits<dStorm::Pixel> : public Traits<int> {};
}

namespace dStorm {

class DisplayHandler : public ost::Thread {
  public:
    static DisplayHandler& getSingleton();

    class ImageHandle;
    class ViewportHandler {
      public:
        ost::Mutex& mutex;

        ViewportHandler( ost::Mutex& access_mutex )
            : mutex( access_mutex ) {}
        virtual ~ViewportHandler() {}
        virtual void clean() = 0;
        virtual std::auto_ptr<ImageHandle> detach_from_window() = 0;
    };
    class ImageHandle {
      protected:
        data_cpp::Vector<Pixel> pending_changes;

      public:
        virtual ~ImageHandle() {}
        virtual void setSize(int w, int h) = 0;
        virtual void 
            drawPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
                new(pending_changes.allocate()) Pixel( x, y, r, g, b );
                pending_changes.commit();
            }
        virtual void clear( uint8_t r, uint8_t g, uint8_t b ) = 0;
    };
    virtual std::auto_ptr<ImageHandle> makeImageWindow
        (const std::string& name, ViewportHandler& handler) = 0;
    virtual void returnImageWindow
        ( std::auto_ptr<ImageHandle> handle, bool close_immediately ) = 0;

  protected:
    DisplayHandler() : ost::Thread("Display handler") {}
    DisplayHandler(const DisplayHandler&);
    virtual ~DisplayHandler() {}
    DisplayHandler& operator=(const DisplayHandler&);
};

}

#endif
