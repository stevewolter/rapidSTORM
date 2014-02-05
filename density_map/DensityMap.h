#ifndef DSTORM_DENSITY_MAP_DENSITY_MAP_H
#define DSTORM_DENSITY_MAP_DENSITY_MAP_H

#include "density_map/Interpolator.h"
#include <Eigen/Core>
#include <boost/units/Eigen/Core>
#include "dStorm/image/Image.h"
#include <dStorm/image/MetaInfo.h>
#include <dStorm/output/Output.h>

namespace dStorm {
namespace density_map {

template <int Dim> struct Coordinates;

/** This class accumulates the Localization results of an Engine
    *  into a single image. This image is not normalized
    *  and should not be used for display; use the Viewer class
    *  or a ViewportImage for that.
    *
    *  \author Steve Wolter
    *  \date   October 2008
    *  \sa dStorm::Viewer
    **/
template <typename Listener_, int Dim>
class DensityMap 
    : public output::Output
{
public:
    typedef dStorm::Image<float,Dim> BinnedImage;
    typedef Eigen::Array< boost::units::quantity<camera::length,int>, 
                            Dim, 1, Eigen::DontAlign > 
            Crop;
    typedef typename Interpolator<Dim>::Ptr InterpolatorPtr;

    static const Crop no_crop;
private:
    /** Crop given in the constructor. */
    Crop crop;
    /** Accumulator image, or in other terms, the density image of
        *  localizations. */
    BinnedImage base_image;
    /** Copy of the announcement made by announceStormSize. 
        *  Used in set_resolution_enhancement. */
    std::auto_ptr<Announcement> announcement;

    Listener_* listener;
    std::auto_ptr< Coordinates<Dim> > strategy;
    InterpolatorPtr binningInterpolator;

    void set_base_image_size();
    
    void store_results_( bool success ); 
    void attach_ui_( simparm::NodeHandle ) {}

public:
    /** @param crop Gives the amount of space to be cut from all
        *              image borders. */
    DensityMap(
        Listener_* listener,
        std::auto_ptr< Coordinates<Dim> > strategy, 
        InterpolatorPtr interpolator,
        Crop crop = no_crop);
    DensityMap(const DensityMap&);

    ~DensityMap();
    
    AdditionalData announceStormSize(const Announcement&);
    RunRequirements announce_run(const RunAnnouncement&) 
        { clear(); return RunRequirements(); }
    void receiveLocalizations(const EngineResult&);

    const BinnedImage& operator()() const { return base_image; }

    /** Check thresholds and recompute for this image and its listener.*/
    void clean();
    /** Delete all localizations in this image and its listener. */
    void clear();
    
    void set_listener( Listener_* listener ) { this->listener = listener; }
    void write_density_matrix( std::ostream& );
};

}
}

#endif
