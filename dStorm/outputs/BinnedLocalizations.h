#ifndef DSTORM_BINNEDLOCALIZATIONS_H
#define DSTORM_BINNEDLOCALIZATIONS_H

#include "BinnedLocalizations_decl.h"
#include <Eigen/Core>
#include <boost/units/Eigen/Core>
#include "../Image.h"
#include <dStorm/image/MetaInfo.h>
#include "../output/Output.h"
#include <cassert>

namespace dStorm {
namespace outputs {

    /** Public interface necessary for a class listening
     *  to \c BinnedLocalizations. */
    template <int Dimensions = 2>
    class BinningListener {
      public:
        typedef dStorm::Image<float,Dimensions> BinnedImage;
        typedef dStorm::image::MetaInfo<Dimensions> MetaInfo;

        /** The setSize method is called before any announcements are made,
         *  and is called afterwards when the image size changes.
         *  @param traits     Traits of the binned image
         */
        inline void setSize(const MetaInfo&) ;
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
        inline void updatePixel(const typename BinnedImage::Position&, float, float) ;
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
    template <int Dimensions>
    struct DummyBinningListener : public BinningListener<Dimensions> {
        typedef dStorm::Image<float,Dimensions> BinnedImage;
        typedef typename BinningListener<Dimensions>::MetaInfo MetaInfo;
        void setSize(const MetaInfo&) {}
        void announce(const output::Output::Announcement&) {}
        void announce(const output::Output::EngineResult&) {}
        void announce(const Localization&) {}
        void updatePixel(const typename BinnedImage::Position&, float, float) {}
        void clean(bool) {}
        void clear() {}
    };

    /** The BinningPublisher class stores a pointer to the currently
     *  set listener, if any is provided. The publisher acts as a
     *  pointer to a class compatible to DummyBinningListener. */
    template <int Dimensions, typename Listener>
    struct BinningPublisher
    {
        Listener *fwd;
      public:
        typedef dStorm::Image<float,Dimensions> BinnedImage;
        inline void setListener(Listener* target)
            { fwd = target; }
        inline Listener& binningListener() { return *fwd; }

        inline const BinnedImage& get_binned_image();
        inline float get_binned_pixel(int x, int y);
    };

    template <int Dim>
    struct BinningStrategy {
        typedef Eigen::Matrix<float, Eigen::Dynamic, Dim+1> Result;

        virtual BinningStrategy* clone() const = 0;
        virtual ~BinningStrategy() {}
        virtual void announce(const output::Output::Announcement&) = 0;
        virtual Eigen::Matrix<quantity<camera::length>, Dim, 1> get_size() = 0;
        virtual typename image::MetaInfo<Dim>::Resolutions get_resolution() = 0;
        virtual int bin_points( 
            const output::LocalizedImage&, Result& ) = 0;
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
    template <typename KeepUpdated, int Dim>
    class BinnedLocalizations 
        : public output::OutputObject, public BinningPublisher<Dim,KeepUpdated>
    {
      public:
        typedef dStorm::Image<float,Dim> BinnedImage;
        typedef Eigen::Array< boost::units::quantity<camera::length,int>, 
                              Dim, 1, Eigen::DontAlign > 
                Crop;

        static const Crop no_crop;
      protected:
        /** Crop given in the constructor. */
        Crop crop;
        /** Accumulator image, or in other terms, the density image of
         *  localizations. */
        BinnedImage base_image;
        /** Copy of the announcement made by announceStormSize. 
         *  Used in set_resolution_enhancement. */
        std::auto_ptr<Announcement> announcement;

        std::auto_ptr<BinningStrategy<Dim> > strategy;

        void set_base_image_size();
        
        template<typename Listener, int ODim> friend class BinnedLocalizations;

        void store_results_( bool success ); 

      public:
        /** @param crop Gives the amount of space to be cut from all
         *              image borders. */
        BinnedLocalizations(std::auto_ptr<BinningStrategy<Dim> > strategy, Crop crop = no_crop);
        BinnedLocalizations(const BinnedLocalizations&);

        template <typename OtherListener>
        BinnedLocalizations(const BinnedLocalizations<OtherListener,Dim>&);

        virtual ~BinnedLocalizations() {}
        BinnedLocalizations* clone() const 
            { return new BinnedLocalizations(*this); }
        
        AdditionalData announceStormSize(const Announcement&);
        RunRequirements announce_run(const RunAnnouncement&) 
            { clear(); return RunRequirements(); }
        void receiveLocalizations(const EngineResult&);

        const BinnedImage& operator()() const { return base_image; }

        /** Check thresholds and recompute for this image and its listener.*/
        void clean();
        /** Delete all localizations in this image and its listener. */
        void clear();
        
        void write_density_matrix( std::ostream& );
    };
}
}
#endif
