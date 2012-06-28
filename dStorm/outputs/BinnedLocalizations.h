#ifndef DSTORM_BINNEDLOCALIZATIONS_H
#define DSTORM_BINNEDLOCALIZATIONS_H

#include <Eigen/StdVector>
#include "BinnedLocalizations_decl.h"
#include <Eigen/Core>
#include <boost/units/Eigen/Core>
#include "../Image.h"
#include <dStorm/image/MetaInfo.h>
#include "../output/Output.h"
#include <cassert>
#include "density_map/Interpolator.h"

namespace dStorm {
namespace outputs {

    template <int Dim>
    struct BinningStrategy {
        struct ResultRow {
            Eigen::Matrix<float, Dim, 1> position, position_uncertainty;
            float intensity, intensity_uncertainty;
        };
        typedef std::vector<ResultRow> Result;

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
        : public output::Output
    {
      public:
        typedef dStorm::Image<float,Dim> BinnedImage;
        typedef Eigen::Array< boost::units::quantity<camera::length,int>, 
                              Dim, 1, Eigen::DontAlign > 
                Crop;
        typedef typename density_map::Interpolator<Dim>::Ptr Interpolator;

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

        KeepUpdated* listener;
        std::auto_ptr<BinningStrategy<Dim> > strategy;
        Interpolator binningInterpolator;

        void set_base_image_size();
        
        template<typename Listener, int ODim> friend class BinnedLocalizations;

        void store_results_( bool success ); 
        void attach_ui_( simparm::NodeHandle ) {}

      public:
        /** @param crop Gives the amount of space to be cut from all
         *              image borders. */
        BinnedLocalizations(
            KeepUpdated* listener,
            std::auto_ptr<BinningStrategy<Dim> > strategy, 
            Interpolator interpolator,
            Crop crop = no_crop);
        BinnedLocalizations(const BinnedLocalizations&);

        template <typename OtherListener>
        inline BinnedLocalizations(KeepUpdated* listener, const BinnedLocalizations<OtherListener,Dim>&);

        virtual ~BinnedLocalizations() {}
        
        AdditionalData announceStormSize(const Announcement&);
        RunRequirements announce_run(const RunAnnouncement&) 
            { clear(); return RunRequirements(); }
        void receiveLocalizations(const EngineResult&);

        const BinnedImage& operator()() const { return base_image; }

        /** Check thresholds and recompute for this image and its listener.*/
        void clean();
        /** Delete all localizations in this image and its listener. */
        void clear();
        
        void set_listener( KeepUpdated* listener ) { this->listener = listener; }
        void write_density_matrix( std::ostream& );
    };


template <typename Listener, int Dim>
template <typename OtherListener>
BinnedLocalizations<Listener,Dim>::
    BinnedLocalizations( Listener* listener, const BinnedLocalizations<OtherListener,Dim>& o)
: crop(o.crop),
  base_image(o.base_image),
  announcement( (o.announcement.get()) ? new Announcement(*o.announcement) : NULL ),
  listener( listener ),
  strategy( o.strategy->clone() ),
  binningInterpolator( o.binningInterpolator->clone() )
{
}

}
}
#endif
