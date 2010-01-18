#ifndef DSTORM_BINNEDLOCALIZATIONS_H
#define DSTORM_BINNEDLOCALIZATIONS_H

#include <dStorm/output/Output.h>
#include <CImg.h>
#include <cassert>

namespace dStorm {
namespace outputs {

    typedef cimg_library::CImg<float> BinnedImage;

    /** Public interface necessary for a class listening
     *  to \c BinnedLocalizations. */
    class BinningListener {
      public:
        /** The setSize method is called before any announcements are made,
         *  and is called afterwards when the image size changes.
         *  @param traits     Traits of the binned image
         */
        inline void setSize(const input::Traits< BinnedImage >&) ;
        /** This method is a forward for dStorm::Output method 
         *  announceStormSize. */
        inline void announce(const output::Output::Announcement&) ;
        /** This method is called when processing of an engine result 
         *  starts. */
        inline void announce(const output::Output::EngineResult&) ;
        /** This method is called once for each localization processed. */
        inline void announce(const Localization&) ;
        /** Called when a pixel changes in the binned image. The parameters
         *  give the x and y position of the changed pixel and its old and
         *  new value. */
        inline void updatePixel(int, int, float, float) ;
        /** Forwards the call to BinnedLocalizations::clean(), that is,
         *  the listener should clean its state.
         *  @param lastClean    This clean is the final clean after the
         *                      whole image was constructed */
        inline void clean( bool lastClean ) ;
        /** Forwards the call to BinnedLocalizations::clear(), that is,
         *  the state should be reset to an empty image. */
        inline void clear() ;
    };

    /** A dummy class implementing all methods necessary for a
     *  BinningListener.
     *  All methods are empty so that, after inlining, no superfluous
     *  code is left in BinnedLocalizations for an empty listener
     *  slot. */
    struct DummyBinningListener : public BinningListener {
        void setSize(const input::Traits< BinnedImage >&) {}
        void announce(const output::Output::Announcement&) {}
        void announce(const output::Output::EngineResult&) {}
        void announce(const Localization&) {}
        void updatePixel(int, int, float, float) {}
        void clean(bool) {}
        void clear() {}
    };

    /** The BinningPublisher class stores a pointer to the currently
     *  set listener, if any is provided. The publisher acts as a
     *  pointer to a class compatible to DummyBinningListener. */
    template <typename Listener>
    struct BinningPublisher
    {
        Listener *fwd;
      public:
        inline void setListener(Listener* target)
            { fwd = target; }
        inline Listener& binningListener() { return *fwd; }

        inline const BinnedImage& get_binned_image();
        inline float get_binned_pixel(int x, int y);
    };

    /** This class accumulates the Localization results of an Engine
     *  into a single image. This image is not normalized
     *  and should not be used for display; use the Viewer class
     *  or a ViewportImage for that.
     *
     *  \author Steve Wolter
     *  \date   October 2008
     *  \sa dStorm::Viewer
     **/
    template <typename KeepUpdated = DummyBinningListener>
    class BinnedLocalizations 
        : public output::OutputObject, public BinningPublisher<KeepUpdated>
    {
      public:
        typedef boost::units::quantity<cs_units::camera::length,int>
            Crop;
      protected:
        /** Mutex protects all methods from the dStorm::Output
         *  interface */
        ost::Mutex mutex;
        /** Configured resolution enhancement. */
        float re;
        /** Crop given in the constructor. */
        Crop crop;
        /** Accumulator image, or in other terms, the density image of
         *  localizations. */
        BinnedImage base_image;
        /** Copy of the announcement made by announceStormSize. 
         *  Used in set_resolution_enhancement. */
        std::auto_ptr<Announcement> announcement;

        void set_base_image_size();

      public:
        /** @param crop Gives the amount of space to be cut from all
         *              image borders. */
        BinnedLocalizations(double res_enh, Crop crop = 0);
        BinnedLocalizations(const BinnedLocalizations&);
        virtual ~BinnedLocalizations() {}
        BinnedLocalizations<KeepUpdated>* clone() const 
            { return new BinnedLocalizations<KeepUpdated>(*this); }

        AdditionalData announceStormSize(const Announcement&);
        Result receiveLocalizations(const EngineResult&);
        void propagate_signal(ProgressSignal s); 

        const BinnedImage& operator()() const { return base_image; }

        /** Check thresholds and recompute for this image and its listener.*/
        void clean();
        /** Delete all localizations in this image and its listener. */
        void clear();

        /** Width of the binned image. */
        int width() const { return base_image.width; }
        /** Height of the binned image. */
        int height() const { return base_image.height; }

        /** Change the resolution enhancement. The new image will be
         *  empty, and all localizations have to be re-sent. */
        void set_resolution_enhancement(double to) 
;

        ost::Mutex& getMutex() { return mutex; }
    };
}
}
#endif
