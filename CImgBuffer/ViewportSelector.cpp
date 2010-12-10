#define CIMGBUFFER_ANDORCAMERA_VIEWPORTSELECTOR_CPP
#include "ViewportSelector.h"

using namespace std;
using namespace AndorCamera;
using namespace CImgBuffer;
using namespace AndorDirect;
using namespace cimg_library;

#define CHECK(x) checkAndorCode( x, __LINE__ )
#define DISP_RESIZE ((detWidth > CImgDisplay::screen_dimx() || \
                      detHeight > CImgDisplay::screen_dimy()) ? 2 : 1)

namespace CImgBuffer {
namespace AndorDirect {

class ImageAcquirer : public ost::Thread {
  private:
    ViewportSelector &vps;
  public:
    ImageAcquirer(ViewportSelector &vps) throw() 
        : ost::Thread("Image acquirer"), vps(vps) {}
    void run() throw() { vps.acquire(); }
};

static const int colorChannels = 3;

CImgBuffer::AndorDirect::ViewportSelector::ViewportSelector(Config& c) throw ()
: ost::Thread("Viewport Selector"),
  left(c.left), right(c.right), 
  top(c.top), bottom(c.bottom),
  saveTo(c.imageFile),
  closer(c.stopAim),
  pause(c.pause),
  save(c.save),
  needMoreImages(true),
  detWidth(c.right.max()+1), 
  detHeight(c.bottom.max()+1),
  mapFac(DISP_RESIZE),
  display(detWidth/mapFac, detHeight/mapFac, 
          "Camera calibration", 1),
  tempImage(detWidth, detHeight, 
            1, colorChannels),
  paused(false)
{
    connect(closer);
    connect(pause);
    connect(save);
    detach();
}

CImgBuffer::AndorDirect::ViewportSelector::~ViewportSelector() throw() {
    enterMutex();
    display.close();
    leaveMutex();
}

void ViewportSelector::run() throw()
{
    CImgDisplay &d = display;
    haveMoreImages= true;
    imageAcquirer.reset( new ImageAcquirer(*this) );
    imageAcquirer->start();
    /* Last valid mouse position in X/Y coordinates. */
    unsigned int lastMX = 0, lastMY = 0;

    enterMutex();
    while ( haveMoreImages && ! d.is_closed ) {
        bool move = false;
        if (d.mouse_x >= 0) lastMX = d.mouse_x;
        if (d.mouse_y >= 0) lastMY = d.mouse_y;
        if (d.is_event) {
            /* move is set to true if any mouse button was pressed */
            for ( int i = 0; i < 512; i++)
                if (d.buttons[i] == 1) {
                    move = true;
                    break;
                }
        }
        d.flush();
        leaveMutex();

        if (move) {
            /* Determine which corner of the rectangle is closest. */
            int mx = lastMX * mapFac, my = lastMY * mapFac;
            int middleX = (right() + left()) / 2;
            int middleY = (top() + bottom()) / 2;
            /* Set closest corner to last known mouse position. */
            if (mx < middleX) left = mx; else right = mx;
            if (my < middleY) top = my; else bottom = my;
        }
        cimg::wait(30);
        enterMutex();
    }
    needMoreImages = false;
    leaveMutex();

    imageAcquirer->join();
    closer.trigger();
}

void ViewportSelector::acquire() throw() 
{
    /* Allocate buffer for data storage */
    unsigned long sz = detWidth * detHeight;
    uint16_t *data = NULL;
    data = (uint16_t*)calloc(sz, sizeof(uint16_t));
    if (data == NULL) { 
        cerr << "Could not allocate data buffer" << endl; 
        return;
    }
        
    /* Start acquisition with unlimited length. Acquisition is stopped
     * by needMoreImages variable. */
    Acquisition acq( 0, detWidth-1, 0, detHeight-1, 
                     numeric_limits<unsigned long>::max() );
    try {
        acq.start();
    } catch (const std::exception & e) {
        cerr << "Could not start image acquisition: " << e.what() << endl;
        free(data);
        haveMoreImages = false;
        return;
    }

    while ( needMoreImages ) {
        long index;
        /* Initialize first data elements. Was used for debugging and
         * am afraid code will break if removed. */
        data[0] = data[1] = data[2] = 0;
        /* Acquire image */
        try {
            index = acq.getNextImage(data);
        } catch (const std::exception &e) {
            cerr << "Image acquisition failed:" << e.what() << endl;
            break;
        }
        if (index == -1) break;
        if (paused == true) continue;

        /* Copy the recorded signal into every color channel. */
        for (int i = 0; i < colorChannels; i++)
            memcpy( tempImage.ptr(0,0,0,i), data, sz * sizeof(uint16_t) );
        int max, min = tempImage.minmax(max);

        /* Find coordinates close to the true border which are displayable in the
         * reduced view. */
        enterMutex();
        int l = (left() / mapFac) * mapFac, 
            r = (right() / mapFac) * mapFac,
            t = (top() / mapFac) * mapFac,
            b = (bottom() / mapFac) * mapFac;
        leaveMutex();

        /* Draw the red rectangle that indicates the current acquisition borders */
        for (int v = 0; v < colorChannels; v++) {
            for (int x = l; x <= r; x++) {
                tempImage(x, t, 0, v) = (v) ? min : max;
                tempImage(x, b, 0, v) = (v) ? min : max;
            }
            for (int y = t; y <= b; y++) {
                tempImage(l, y, 0, v) = (v) ? min : max;
                tempImage(r, y, 0, v) = (v) ? min : max;
            }
        }

        enterMutex();
        display << tempImage;
        leaveMutex();
    }
    free(data);
    haveMoreImages = false;
}

void CImgBuffer::AndorDirect::ViewportSelector::gotEvent
    (const ConfigEntry &src) 
    throw() 
{ 
    if (&src == &closer && closer.triggered()) {
        enterMutex();
        display.close();
        leaveMutex();
    } else if (&src == &pause && pause.triggered()) {
        pause.untrigger();
        paused = !paused;
    } else if (&src == &save && save.triggered()) {
        save.untrigger();
        if ( saveTo ) {
            try {
                CImg<uint8_t> brokenDown = tempImage.get_normalize(0,255);
                brokenDown.save( saveTo().c_str() );
            } catch (const cimg_library::CImgException &e) {
                cerr << "CImg: " << e.message << endl;
            } catch (const std::exception &e) {
                cerr << "CImg: " << e.what() << endl;
            }
        }
    }
}

}
}
