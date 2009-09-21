#ifndef DSTORM_HIGHDEPTHIMAGE_H
#define DSTORM_HIGHDEPTHIMAGE_H

#include <dStorm/Output.h>
#include <CImg.h>
#include <iostream>
#include <cc++/thread.h>
#include <dStorm/transmissions/BinnedLocalizations.h>

#define USE_INTERMED_IMAGE

namespace dStorm {

    struct DummyDiscretizationListener {
        void setSize(int, int, int) {}
        void updatePixel(int, int, SmoothedPixel, SmoothedPixel) {}
        void clean() {}
        void clear() {}
    };

    template <typename Listener>
    struct DiscretizationPublisher;

    template <>
    class DiscretizationPublisher
        <DummyDiscretizationListener>
    {
        DummyDiscretizationListener dummy;
      public:
        inline DummyDiscretizationListener* operator->()
            { return &dummy; }
        inline void setListener(DummyDiscretizationListener*)
 { assert( false ); }
    };

    template <typename Listener>
    class DiscretizationPublisher
    {
        Listener *fwd;
      public:
        inline void setListener(Listener* target)
            { fwd = target; }
        inline Listener* operator->() { return fwd; }
    };

    template <
        typename BinningListener = DummyBinningListener,
        typename DiscretizationListener = DummyDiscretizationListener>
    class HighDepthImage {
        ost::Mutex mutex;
        BinningPublisher<BinningListener> binningPub;
        DiscretizationPublisher<DiscretizationListener> discPub;

        inline SmoothedPixel discrete( float value ) 
            { return discrete(value, discretFactor); }
        inline SmoothedPixel discrete
            ( float value, const float& factor )
            { return std::min<SmoothedPixel>
                    ( std::max<int>(0, round(value * factor)), depth - 1 ); }
      public:
        HighDepthImage(int depth, int rediscThreshold)
            : depth(depth), discretFactor(0), thres(rediscThreshold),
              dirty(thres+1), need_rediscretization(true) {}

        void setSize(int width, int height) { 
            binningPub->setSize(width, height);
#ifdef USE_INTERMED_IMAGE
            intermed.resize(width, height, 1, 1, -1);
#endif
            discPub->setSize(width, height, depth);
            clear();
        }

        inline void announce(const Output::Announcement& a)
            { binningPub->announce(a); }
        inline void announce(const Output::EngineResult& er)
            { binningPub->announce(er); }
        inline void announce(const Localization& l)
            { binningPub->announce(l); }

        inline void updatePixel(int x, int y, float old_val, float val)
 
        {
            binningPub->updatePixel(x, y, old_val, val);
            if (val > discretMax) {
                if ( val > currentMax )
                    currentMax = val;
                dirty++;
                if ( dirty > thres )
                    need_rediscretization = true;
            }

            //if (need_rediscretization) return;
#ifdef USE_INTERMED_IMAGE
            SmoothedPixel& oldDisc = intermed(x,y),
                          newDisc = discrete(val);
#else
            SmoothedPixel oldDisc = discrete(old_val), 
                          newDisc = discrete(val);
#endif

            if (oldDisc != newDisc) {
                discPub->updatePixel( x, y, oldDisc, newDisc );
#ifdef USE_INTERMED_IMAGE
                oldDisc = newDisc;
#endif
            }
        }

        void clean(const cimg_library::CImg<float>& src);
        void clear();

        void setListener(BinningListener *h) 
            { binningPub.setListener(h); }
        void setListener(DiscretizationListener *h) 
            { discPub.setListener(h); }

      private:
        SmoothedImage intermed;
        int depth;
        float discretFactor, discretMax, currentMax;
        int thres, dirty;
        bool need_rediscretization;
    };
}

#endif
