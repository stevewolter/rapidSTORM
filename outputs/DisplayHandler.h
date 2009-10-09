#ifndef DSTORM_TRANSMISSIONS_DISPLAY_HANDLER_H
#define DSTORM_TRANSMISSIONS_DISPLAY_HANDLER_H

#include <cc++/thread.h>
#include <memory>
#include <dStorm/Image.h>
#include <data-c++/Vector.h>

namespace dStorm {
    struct Color { uint8_t r, g, b; };
    struct ChangeEvent {
        enum Type { ResizeClear, Pixel, Clear, Key };

        struct ClearChange {
            Color background;
        };
        struct ResizeChange : public ClearChange {
            int width, height;
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
        union AllChanges {
            ResizeChange resize;
            PixelChange pixel;
            ClearChange clear;
            KeyChange key;
        };

        /** Sets interpretation of \c change field. */
        Type type;
        AllChanges change;
    };
}

namespace data_cpp {
    template <>
    class Traits<dStorm::ChangeEvent> : public Traits<int> {};
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
        data_cpp::Vector<ChangeEvent> pending_changes;
      public:
        virtual ~ImageHandle() {}

        ChangeEvent::ResizeChange& resize() {
            pending_changes.clear();
            ChangeEvent *ev = pending_changes.allocate( 1 );
            ev->type = ChangeEvent::ResizeClear;
            return ev->change.resize;
        }
        ChangeEvent::ClearChange& clear() {
            bool restore_resize = (pending_changes.size() != 0
                && pending_changes.front().type==ChangeEvent::ResizeClear);
            pending_changes.clear();
            if ( restore_resize ) pending_changes.commit( 1 );
            ChangeEvent *ev = pending_changes.allocate( 1 );
            ev->type = ChangeEvent::Clear;
            return ev->change.clear;
        }
        ChangeEvent* declare_changes( int number ) { 
            return pending_changes.allocate( number ); 
        }
        ChangeEvent::KeyChange& set_key() {
            ChangeEvent *ev = pending_changes.allocate( 1 );
            ev->type = ChangeEvent::Key;
            return ev->change.key;
        }
        void commit_changes(int number) {
            pending_changes.commit(number); 
        }
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
