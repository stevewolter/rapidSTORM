#ifndef DSTORM_BINNEDLOCALIZATIONS_H
#define DSTORM_BINNEDLOCALIZATIONS_H

#include <dStorm/Transmission.h>
#include <CImg.h>

namespace dStorm {

    class HighDepthImage;

    /** This class accumulates the Localization results of an Engine
     *  into a single image. This image is not normalized
     *  and should not be used for display; use the Viewer class
     *  or a ViewportImage for that.
     *
     *  \author Steve Wolter
     *  \date   October 2008
     *  \sa dStorm::Viewer
     **/
    class BinnedLocalizations : public dStorm::Transmission {
    private:
        ost::Mutex mutex;
        double re, ampThres;
        int crop;
        dStorm::Localization::Raster r;
        cimg_library::CImg<float> base_image;

        HighDepthImage *keepUpdated;

    public:
        /** @param crop Gives the amount of space to be cut from all
         *              image borders. */
        BinnedLocalizations(double res_enh, int crop = 0) : 
            ConfigObject("BinnedLocalizations", ""),
            re(res_enh), ampThres(0), crop(crop),
            r(dStorm::Localization::getRaster(res_enh)),
            keepUpdated(NULL) {}
        virtual ~BinnedLocalizations() throw() {}

        AdditionalData announceStormSize(int w, int h, int ) throw();
        Result receiveLocalizations(const EngineResult&) throw();
        void deleteAllResults() throw() 
            { ost::MutexLock lock(mutex); clear(); }
        void finish() throw() {}

        const cimg_library::CImg<float>& operator()() const throw()
            { return base_image; }

        /** The HighDepthImage \c listener will be noticed of any change
         *  in this class. Only one listener at any time is allowed. */
        void setListener(HighDepthImage *listener) throw()
            { keepUpdated = listener; }

        /** Check thresholds and recompute for this image and its listener.*/
        void clean() throw();
        /** Delete all localizations in this image and its listener. */
        void clear() throw();

        int width() const throw() { return base_image.width; }
        int height() const throw() { return base_image.height; }

        void setAmplitudeThreshold(double thres) throw()
            { ampThres = thres; }
    };

}
#endif
