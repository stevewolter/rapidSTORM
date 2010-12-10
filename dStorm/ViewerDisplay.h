#ifndef DSTORM_VIEWERDISPLAY_H
#define DSTORM_VIEWERDISPLAY_H

#include <string>
#include <cc++/thread.h>
#include "Localization.h"
#include <CImg.h>
#include <memory>
#include "ViewerDisplayConfig.h"
#include <ConfigEventHandler.hh>
#include "LocalizationList.h"
#include <data-c++/Vector.h>

/**********************************************************************
 * ViewerDisplay: A helper class for the Viewer. This class is fed    *
 * with Fits and displays them to a CImgDisplay; it runs concurrently *
 * to the rest of the program and will not stop until closed.         *
 **********************************************************************/

namespace dStorm {
    class ViewerDisplay : public ost::Thread, 
                          public ConfigEventHandler
    {
      public:
        ViewerDisplay(ViewerDisplayConfig &c, 
            unsigned int width, unsigned int height, double res_enh,
            double min_fit_amp) throw();
        virtual ~ViewerDisplay() throw();
        bool needsJoining() const throw() { return finished; }
        bool isDisplaying() const throw() 
            { return viewport.get() != NULL; }

        void insertFits(const Localization *, int num) throw();
        void clear() throw();
        void redraw() throw();

        void writeToFile(const std::string &name) throw();


      protected:
        void run() throw();
        void gotEvent(const ConfigEntry& src) throw();

      private:
        ost::Mutex run_mutex;
        ViewerDisplayConfig& config;
        bool finished;
        std::string name;

        /** Dimensions of the two images */
        int width, height;
        /** Pixel count of the two images */
        unsigned long int imageSize;
        /** Depth of the \c discretizedImage */
        int dI_depth;

        /** Mutex protecting the insertion process */
        ost::Mutex chainMutex;
        /** List with all localizations used in the accumImage.
         *  Needed for rebuild if the threshold changes. */
        Fits sourceFits;
        double currentAmplitudeThreshold, minStrength, maxStrength;
        /** Image with the accumulated, linearly interpolated
        * localization values.  */
        cimg_library::CImg<float> accumImage;
        /** Keeps the current maximum of \c accumImage. */
        float realMax;
        /** Keeps the value of \c realMax that triggers
        * re-discretization if exceeded. */
        float criticalMax;
        /** The discretization factor currently used for 
        *  \c discretizedImage. */
        float discretizationFactor;
        /** Discretization is dirty */
        bool discretizationIsDirty;

        /** \c accumImage scaled by \c discretizationFactor and
        *  discretized */
        cimg_library::CImg<uint16_t> discretizedImage;
        /** Uncommitted changes to the histogram */
        int dirtyHistogramValues;
        /** Histogram normalization power */
        double histogramPower;

        data_cpp::Vector<unsigned long> originalHistogram;
        data_cpp::Vector<uint8_t> realGreyValueMap,
                            usedGreyValueMap;
        bool usedGVM_is_dirty;
        unsigned int x0, x1, y0, y1;
        cimg_library::CImg<uint8_t> normIm;
        int dirtyPixels;
        Localization::Raster r;

        std::auto_ptr<cimg_library::CImgDisplay> viewport;

        void addToAccumImage(unsigned int x, unsigned int y, float val) throw();
        inline void addToAccumImage(const Localization &f, int sgn = 1) throw() {
            int lx = f.getXLow(r), ly = f.getYLow(r);
            float xf = f.getXR(r), yf = f.getYR(r);

            addToAccumImage(lx, ly, (1-xf)*(1-yf)*sgn);
            addToAccumImage(lx+1, ly, (xf)*(1-yf)*sgn);
            addToAccumImage(lx, ly+1, (1-xf)*(yf)*sgn);
            addToAccumImage(lx+1, ly+1, (xf)*(yf)*sgn);
        }

        void recomputeAccumImage() throw();
        void rediscretize() throw();
        void recomputeHistogram() throw();
        void updateUsedGVM() throw();
        void resizeNormImage() throw();
        void recomputeNormImage() throw();

        inline unsigned long histogramValues() const throw()
            { return imageSize - originalHistogram[0]; }

        inline bool thresholdIsDirty() const throw() {
            return config.amplitudeThreshold() 
                != currentAmplitudeThreshold;
        }
        inline bool histogramIsDirty() const throw()
            { return discretizationIsDirty || thresholdIsDirty() ||
                    dirtyHistogramValues > 100; }
        inline bool used_GVM_IsDirty() const throw()
            { return histogramIsDirty() || usedGVM_is_dirty; }
        inline bool displayIsDirty() const throw()
            { return used_GVM_IsDirty() || dirtyPixels > 20; }
    };
}
#endif
