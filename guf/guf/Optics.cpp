#include "Optics.h"
#include <dStorm/engine/InputPlane.h>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/weighted_variance.hpp>
#include <boost/accumulators/statistics/stats.hpp>

namespace dStorm {
namespace guf {

using namespace boost::units;
using namespace boost::accumulators;

Optics::Optics( const Spot& max_distance, const engine::InputPlane& image_optics )
: projection( image_optics.projection() ),
  max_distance(max_distance),
  photon_response_( image_optics.optics.photon_response.get_value_or( 1 * camera::ad_count ) ),
  dark_current( image_optics.optics.dark_current.get_value_or(0*camera::ad_count) ),
  background_noise_variance_(
        ( image_optics.optics.background_stddev.is_initialized() && image_optics.optics.photon_response.is_initialized() )
            ? boost::optional<float>( pow( *image_optics.optics.background_stddev / *image_optics.optics.photon_response, 2 ) )
            : boost::optional<float>()
  ),
  has_precision( image_optics.optics.photon_response.is_initialized() && background_noise_variance_.is_initialized() ),
  poisson_background_( image_optics.optics.dark_current.is_initialized() && image_optics.optics.photon_response.is_initialized() ),
  background_part( 0.25f )
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

double Optics::absolute_in_photons( quantity<camera::intensity> amp ) const
    { return ( amp - dark_current ) / photon_response_; }
double Optics::relative_in_photons( quantity<camera::intensity> amp ) const
    { return amp / photon_response_; }

#if 0
  transformation( guf::Spot::Constant( guf::Spot::Scalar( c.fit_window_size() ) ),
                  plane )

#endif

}
}
