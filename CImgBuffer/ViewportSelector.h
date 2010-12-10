#ifndef ANDORDIRECT_VIEWPORTSELECTOR_H
#define ANDORDIRECT_VIEWPORTSELECTOR_H

#include <cc++/thread.h>
#include <ConfigEntry.hh>
#include <ConfigEventHandler.hh>
#include <map>
#include <CImg.h>
#include <AndorCamera/Control.h>
#include "AndorDirect.h"
#include <stdint.h>

namespace CImgBuffer {
    namespace AndorDirect {
        /** The ViewportSelector provides a window in which ConfigEntry 
         *  elements defining the acquisition rectangle can be displayed
         *  and configured interactively. */
        class ViewportSelector : private ConfigEventHandler, 
                                 private ost::Mutex,
                                 private ost::Thread
        {
          private:
            void gotEvent(const ConfigEntry &source) throw();

            AndorCamera::ControlReference control;
            /** Reference to the config element to be configured. */
            ConfigEntryUnsignedLong &left, &right, &top, &bottom;
            /** Reference to a config element that controls snapshot saving. */
            ConfigEntryFile &saveTo;
            ConfigEntryTrigger &closer, &pause, &save;
            /** This flag is set when the acquisition subthread should stop. */
            bool needMoreImages;
            /** This flag is set while the acquisition subthread is running. */
            bool haveMoreImages;
            /** Width and height of the detector CCD. */
            int detWidth, detHeight;
            /** Scaling factor for the window size. If set to value greater than
             *  1, indicates that window should be shrunk by that factor. */
            int mapFac;
            cimg_library::CImgDisplay display;
            /** Buffer image for acquisition. Made class member to allow 
             *  saving to file. */
            cimg_library::CImg<uint16_t> tempImage;
            bool paused;

            /** Called when subthread is started */
            virtual void run() throw();
            friend class ImageAcquirer;
            std::auto_ptr<ost::Thread> imageAcquirer;
            void acquire() throw();

          public:
            /** Constructor and only public interface.
             *  This is a fire-and-forget class: The constructor starts a
             *  subthread that will open the acquisition and update the
             *  display window, and then return control. */
            ViewportSelector (Config& config) throw();
            /** Destructor, will join the subthread and close the display
             *  window. */
            virtual ~ViewportSelector() throw();
        };
    }
}
#endif
