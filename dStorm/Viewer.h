#ifndef DSTORM_VIEWER_H
#define DSTORM_VIEWER_H

#include "Localizations.h"
#include "ViewerConfig.h"
#include "BinnedLocalizations.h"
#include "HighDepthImage.h"
#include <ConfigEntry.hh>
#include <cc++/thread.h>
#include <CImg.h>
#include "ViewportImage.h"
#include "LocalizationList.h"

namespace dStorm {
   /** The Viewer class collects fits into a BinnedLocalizations
    *  image, normalizes the resulting image and shows a part
    *  of that image in a window. */
   class Viewer : public dStorm::Transmission,
                  public ost::Thread,
                  public simparm::ConfigNode::Callback
   {
      public:
        /** Constructor will not display image; this is deferred
         *  until announceStormSize(). */
        Viewer(ViewerConfig& config) throw();
        Viewer(const Viewer&) throw();
        virtual ~Viewer() throw();
        Viewer* clone();

        AdditionalData announceStormSize(int w, int h, int l) throw();
        Result receiveLocalizations(const EngineResult&) throw();
        void deleteAllResults() throw();
        void finish() throw();

      protected:
        /** Method that is called for subthread. */
        void run() throw();
        /** Configuration event received. */
        void operator()(ConfigNode&, Cause, ConfigNode *) throw();

        /** Write the current image into a file. The whole
         *  image is written, regardless of the settings
         *  for viewport or zoom. */
        void writeToFile(const std::string& name) throw();

      private:
         /** Mutex for localizationsStore */
         ost::Mutex locStoreMutex;
         /** List of all received localizations. Necessary if
          *  viewing amplitude threshold is changed. */
         LocalizationList localizationsStore;
         /** Binned image with all localizations in localizationsStore.*/
         BinnedLocalizations image;
         /** Reference to the configuration given in constructor. */
         ViewerConfig& config;
         /** Discretized version of \c image. */
         HighDepthImage discretization;
         /** Viewport to histogram-equalized version of \c image */
         ViewportImage result;
         /** Mutex protecting \c image, \c discretization and 
          *  \c result.*/
         ost::Mutex structureMutex;
         /** Minimal and maximal fit strength encountered. */
         double minStrength, maxStrength;
         /** Current amplitude threshold for viewing. */
         double ampThres;
         /** Factor to zoom the image in or out. By default, both
          *  are 1; to zoom in, zi is raised, to zoom out, zo is
          *  raised. */
         int zi, zo;
         /** Desired width and height of the display window. */
         unsigned int windowWidth, windowHeight;
         /** The last encountered dimensions for \c viewport. */
         unsigned int lastWW, lastWH;

         /** Displays \c viewport */
         std::auto_ptr<cimg_library::CImgDisplay> viewport;

         /** Is the subthread currently running? */
         bool runningViewer;
         /** Should the subthread be terminated? */
         bool terminateViewer;
         /** Was the amplitude threshold changed externally? */
         bool ampThresChanged;
         /** Should \c viewport be resized as soon as possible? */
         bool needsResizing;

         /** Check all thresholds in the chain from \c image,
          *  \c discretization to \c result and recompute necessary
          *  results. */
         void cleanImage() throw();
         /** Resize \c result to match, if possible, the window size
          *  of \c viewport.
          *
          *  @return true if \c viewport must be resized to match
          *          result.
          **/
         bool resizeViewportToWindow() throw();
   };
}

#endif
