#ifndef ANDORDIRECT_VIEWPORTSELECTOR_H
#define ANDORDIRECT_VIEWPORTSELECTOR_H

#ifdef HAVE_LIBATMCD32D

#include <dStorm/helpers/thread.h>
#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <simparm/FileEntry.hh>
#include <simparm/TriggerEntry.hh>
#include <simparm/Set.hh>
#include <map>
#include <CImg.h>
#include <AndorCamera/CameraReference.h>
#include "AndorDirect.h"

namespace AndorCamera { class ImageReadout; class Acquisition; }

namespace dStorm {
namespace AndorDirect {

        /** The ViewportSelector provides a window in which Entry 
         *  elements defining the acquisition rectangle can be displayed
         *  and configured interactively. */
        class ViewportSelector : public simparm::Set,
                                 private simparm::Node::Callback, 
                                 private ost::Mutex,
                                 public ost::Thread
        {
          public:
            class Config;

          private:
            AndorCamera::CameraReference cam;

            void operator()(Node &, Cause, Node *);

            /** Reference to the config element to be configured. */
            simparm::UnsignedLongEntry &left, &right, &top, &bottom;
            Config& config;

            /** This flag is set when the acquisition subthread should
             *  stop. */
            bool needMoreImages;
            /** This flag is set while the acquisition subthread is
             *  running. */
            bool haveMoreImages;
            /** Is set to true when the viewport window should be closed. */
            bool close_viewer;
            /** Width and height of the detector CCD. */
            int detWidth, detHeight;
            /** Scaling factor for the window size. If set to value
             *  greater than 1, indicates that window should be shrunk
             *  by that factor. */
            int mapFac;
            std::auto_ptr<cimg_library::CImgDisplay> display;
            ost::Condition display_was_initialized;
            /** Buffer image for acquisition. Made class member to allow 
             *  saving to file. */
            cimg_library::CImg<CameraPixel> tempImage;
            bool paused;

            /** Called when subthread is started */
            virtual void run() throw();
            friend class ImageAcquirer;
            std::auto_ptr<ost::Thread> imageAcquirer;

            void acquire();
            void configure_camera(AndorCamera::Acquisition&);
            void initialize_display();
            void draw_image(const CameraPixel *data);

          public:
            /** Constructor and only public interface.
             *  This is a fire-and-forget class: The constructor starts a
             *  subthread that will open the acquisition and update the
             *  display window, and then return control. */
            ViewportSelector (
                const AndorCamera::CameraReference& cam,
                AndorCamera::ImageReadout& readout,
                Config &config);
            /** Destructor, will join the subthread and close the display
             *  window. */
            virtual ~ViewportSelector();

            void terminate();
        };

        /** Configuration items for the viewport selection window that
         *  opens when the "aim" button is pressed. */
        class ViewportSelector::Config 
        : public simparm::Object, public simparm::Node::Callback 
        {
            AndorCamera::CameraReference cam;

            void registerNamedEntries();

            void startAiming();
            void stopAiming();

          public:
            simparm::TriggerEntry      aim;
            simparm::Object            statusBox;
            simparm::StringEntry       dynamic_range;
            simparm::TriggerEntry      stopAim;
            simparm::TriggerEntry      pause;
            simparm::FileEntry         imageFile;
            simparm::TriggerEntry      save;

            Config(const AndorCamera::CameraReference& cam);
            Config(const Config &c);
            ~Config();
            Config* clone();

            Config& operator=(const Config&);

            void operator()(Node &src, Cause c, Node*);

          private:
            friend class ViewportSelector;
            void viewport_selector_finished();
            ost::Mutex activeSelectorMutex;
            ost::Condition noActiveSelector;
            ViewportSelector *activeSelector;
        };
}
}

#endif
#endif
