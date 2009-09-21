#ifndef DSTORM_BINNEDLOCALIZATIONS_H
#define DSTORM_BINNEDLOCALIZATIONS_H

#include <dStorm/Output.h>
#include <CImg.h>
#include <cassert>

namespace dStorm {

    /** A dummy class implementing all methods necessary for a
     *  class acting as a listener to \c BinnedLocalizations.
     *  All methods are empty so that, after inlining, no superfluous
     *  codeis left in BinnedLocalizations for an empty listener
     *  slot. */
    struct DummyBinningListener {
        /** The setSize method is called before any announcements are made,
         *  and is called afterwards when the image size changes.
         *  Its parameters give the width and the height of the target
         *  image, respectively. */
        void setSize(int, int) {}
        /** This method is a forward for dStorm::Output method 
         *  announceStormSize. */
        void announce(const Output::Announcement&) {}
        /** This method is called when processing of an engine result 
         *  starts. */
        void announce(const Output::EngineResult&) {}
        /** This method is called once for each localization processed. */
        void announce(const Localization&) {}
        /** Called when a pixel changes in the binned image. The parameters
         *  give the x and y position of the changed pixel and its old and
         *  new value. */
        void updatePixel(int, int, float, float) {}
        /** Forwards the call to BinnedLocalizations::clean(), that is,
         *  the listener should clean its state.
         *  The parameter contains a reference to the current image. */
        void clean(const cimg_library::CImg<float>&) {}
        /** Forwards the call to BinnedLocalizations::clear(), that is,
         *  the state should be reset to an empty image. */
        void clear() {}
    };

    /** The BinningPublisher class stores a pointer to the currently
     *  set listener, if any is provided. The publisher acts as a
     *  pointer to a class compatible to DummyBinningListener and
     *  is specialized for a dummy listener. */
    template <typename Listener>
    struct BinningPublisher
    {
        Listener *fwd;
      public:
        inline void setListener(Listener* target)
            { fwd = target; }
        inline Listener* operator->() { return fwd; }
    };

    /** Specialization for BinnedLocalization objects without
     *  a listener. No pointer is stored, and all methods are
     *  empty. */
    template <>
    struct BinningPublisher<DummyBinningListener>
    : public DummyBinningListener
    {
      public:
        inline DummyBinningListener* operator->()
            { return this; }
        inline void setListener(DummyBinningListener*)
 { assert( false ); }
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
        : public dStorm::Output, public simparm::Object
    {
      protected:
        /** Mutex protects all methods from the dStorm::Output
         *  interface */
        ost::Mutex mutex;
        /** Configured resolution enhancement. */
        double re;
        /** Crop given in the constructor. */
        int crop;
        /** Resolution enhancement in terms of Localization class. */
        dStorm::Localization::Raster r;
        /** Accumulator image, or in other terms, the density image of
         *  localizations. */
        cimg_library::CImg<float> base_image;
        /** Copy of the announcement made by announceStormSize. 
         *  Used in set_resolution_enhancement. */
        Announcement announcement;

        BinningPublisher<KeepUpdated> keepUpdated;

        void set_base_image_size();

      public:
        /** @param crop Gives the amount of space to be cut from all
         *              image borders. */
        BinnedLocalizations(double res_enh, int crop = 0);
        virtual ~BinnedLocalizations() {}
        BinnedLocalizations<KeepUpdated>* clone() const 
            { return new BinnedLocalizations<KeepUpdated>(*this); }

        AdditionalData announceStormSize(const Announcement&)
;
        Result receiveLocalizations(const EngineResult&);
        void propagate_signal(ProgressSignal s) {
            if ( s == Engine_is_restarted ) {
                ost::MutexLock lock(mutex); 
                clear();
            }
        }

        const cimg_library::CImg<float>& operator()() const
            { return base_image; }

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

        /** The KeepUpdated \c listener will be noticed of any change
         *  in this class. Only one listener at any time is allowed. */
        void setListener(KeepUpdated *listener);
    };
}
#endif
