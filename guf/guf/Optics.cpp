#include "Optics.h"
#include <dStorm/engine/InputPlane.h>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/weighted_variance.hpp>
#include <boost/accumulators/statistics/stats.hpp>

namespace dStorm {
namespace guf {

using namespace boost::units;
using namespace boost::accumulators;

Detector::Detector( const traits::Optics& o ) 
: photon_response_( o.photon_response.get_value_or( 1 * camera::ad_count ) ),
  dark_current( o.dark_current.get_value_or(0*camera::ad_count) ),
  background_noise_variance_(
        ( o.background_stddev.is_initialized() && o.photon_response.is_initialized() )
            ? boost::optional<float>( pow( *o.background_stddev / *o.photon_response, 2 ) )
            : boost::optional<float>()
  ),
  has_precision( o.photon_response.is_initialized() && background_noise_variance_.is_initialized() ),
  poisson_background_( o.dark_current.is_initialized() && o.photon_response.is_initialized() )
{}

Optics::Optics( const Spot& max_distance, const engine::InputPlane& image_optics )
: Detector( image_optics.optics ),
  projection( image_optics.projection() ),
  max_distance(max_distance)
{
}

bool Optics::supports_guaranteed_row_width() const {
    return projection.supports_guaranteed_row_width();
}

int Optics::get_fit_window_width( const guf::Spot& at ) const {
    traits::Projection::Bounds bounds = projection.
        get_region_of_interest( traits::Projection::ROISpecification(at, max_distance) );
    return bounds.width(Direction_X) / camera::pixel;
}

double Detector::absolute_in_photons( quantity<camera::intensity> amp ) const
    { return ( amp - dark_current ) / photon_response_; }
double Detector::relative_in_photons( quantity<camera::intensity> amp ) const
    { return amp / photon_response_; }

quantity< si::area > Optics::pixel_size( const Spot& center ) const {
    return projection.pixel_size( projection.nearest_point_in_image_space( center ) );
}

traits::Projection::ROI
Optics::get_region_of_interest( const Spot& center, OptPixel width ) const {
    traits::Projection::ROISpecification roi_request( center, max_distance );
    roi_request.guaranteed_row_width = width;
    return projection.cut_region_of_interest( roi_request );
}

}
}
