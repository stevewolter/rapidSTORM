#ifndef DSTORM_VIEWPORT_H
#define DSTORM_VIEWPORT_H

#include <CImg.h>
#include <dStorm/transmissions/HighDepthImage.h>
#include <iostream>
#include <cassert>
#include <cc++/thread.h>
#include <limits>

namespace dStorm {
    class DummyViewportListener {
        void setSize(int, int, int) {}
        void updatePixel(int, int, SmoothedPixel, SmoothedPixel) {}
        void clean() {}
        void clear() {}

        bool notify_of_all_moved_pixels() const { return false; }
        void setOffset(int, int) {}
    };

    template <typename Listener>
    struct ViewportPublisher;

    template <>
    class ViewportPublisher<DummyViewportListener>
    {
        DummyViewportListener dummy;
      public:
        inline DummyViewportListener* operator->()
            { return &dummy; }
        inline void setListener(DummyViewportListener*)
 { assert( false ); }
    };

    template <typename Listener>
    class ViewportPublisher
    {
        Listener *fwd;
      public:
        inline void setListener(Listener* target)
            { fwd = target; }
        inline Listener* operator->() { return fwd; }
    };

    template <
        typename UnrestrictedListener = DummyDiscretizationListener,
        typename ViewportListener = DummyViewportListener
    >
    class ViewportImage {
      public:
        ViewportImage()
 
        : x0(-1), x1(-2), y0(-1), y1(-2), viewport_moved(false)
        {
        }

        void setSize(int w, int h, int depth) { 
            forward->setSize(w, h, depth);
            baseWidth = w; baseHeight = h; 
            this->depth = depth;
            buffer.resize(w, h, 1, 1, -1);
            buffer.fill( 0 );
            x1 = std::min<int>(w-1,x1);
            y1 = std::min<int>(h-1,y1);
            if ( x1 > 0 ) x0 = std::min<int>(x0,x1);
            if ( y1 > 0 ) y0 = std::min<int>(y0,y1);
            if ( x0 >= 0 && x1 >= 0 && y0 >= 0 && y1 >= 0 )
                setViewport(x0, x1, y0, y1);

            PROGRESS("Setting viewport to " << x0 <<"-"<< x1 << "-" << y0<< "-" << y1);
        }

        void setViewport(int x0, int x1, int y0, int y1) {
            PROGRESS("Setting viewport to " << x0 <<"-"<< x1 << "-" << y0<< "-" << y1);
            this->x0 = x0; this->x1 = x1; this->y0 = y0; this->y1 = y1; 
            viewport_moved = true;

            viewport_buffer.resize( width(), height(), 1, 1, -1 );
            viewported->setSize( width(), height(), depth );

            viewported->setOffset(x0, y0);
        }

        inline void setPixel(int x, int y, SmoothedPixel oldVal,
                                SmoothedPixel val) 
        {
            assert( x >= x0 && y >= y0 && 
                    x-x0 < int(viewport_buffer.width) &&
                    y-y0 < int(viewport_buffer.height) );

            viewport_buffer(x-x0, y-y0) = val;
            viewported->updatePixel( x-x0, y-y0, oldVal, val );
        }
        void updatePixel(int x, int y, SmoothedPixel old,
                         SmoothedPixel val) 
        {
            buffer( x, y ) = val;
            if (x0 <= x && x <= x1 && y0 <= y && y <= y1)
                setPixel(x,y, old, val);
        }

        void clean()
        {
            forward->clean();
            if (x0>x1 || y0>y1) return;
            if ( viewport_moved && buffer.size() > 0 ) {
                for (int x = x0; x <= x1; x++)
                  for (int y = y0; y <= y1; y++) {
                    assert( x-x0 >= 0 && y-y0 >= 0 && 
                            x-x0 < int(viewport_buffer.width) &&
                            y-y0 < int(viewport_buffer.height) );
                    if ( viewported->notify_of_all_moved_pixels() 
                         || viewport_buffer(x-x0, y-y0) != buffer(x,y) )
                        setPixel(x,y, viewport_buffer(x-x0,y-y0),
                                      buffer(x,y) );
                  }
                viewport_moved = false;
            }
            viewported->clean();
        }

        void clear() {
            forward->clear();
            buffer.fill( 0 );
            viewport_buffer.fill( 0 );
            viewported->clear();
        }

        unsigned int leftBorder() const { return x0; }
        unsigned int rightBorder() const { return x1; }
        unsigned int topBorder() const { return y0; }
        unsigned int bottomBorder() const { return y1; }
        unsigned int width() const { return x1-x0+1; }
        unsigned int height() const { return y1-y0+1; }

        void shift(float xpages, float ypages) {
            int nx0 = int(width() * xpages)+x0,
                ny0 = int(height() * ypages)+y0;
            nx0 = std::max<int>(0, 
                    std::min<int>( baseWidth-width(), nx0 ));
            ny0 = std::max<int>(0, 
                    std::min<int>( baseHeight-height(), ny0 ));

            if (x0 != nx0 || y0 != ny0 ) {
                x1 = x1+ (nx0-x0);
                x0 = nx0;
                y1 = y1+ (ny0-y0);
                y0 = ny0;
                viewport_moved = true;
                PROGRESS("Setting viewport to " << x0 <<"-"<< x1 << "-" << y0<< "-" << y1);
            }
        }

        void setUnrestrictedListener(UnrestrictedListener *h) 
            { forward.setListener(h); }
        void setViewportListener(ViewportListener *h) 
            { viewported.setListener(h); }

        const SmoothedImage& full_size_image() const
            { return buffer; }

      private:
        DiscretizationPublisher<UnrestrictedListener> forward;
        ViewportPublisher<ViewportListener> viewported;

        SmoothedImage buffer, viewport_buffer;

        int x0, x1, y0, y1;
        int baseWidth, baseHeight;
        int depth;
        bool viewport_moved;
    };
};

#endif
