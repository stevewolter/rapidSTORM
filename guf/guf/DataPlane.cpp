#ifndef DSTORM_GUF_DATAPLANE_IMPL_H
#define DSTORM_GUF_DATAPLANE_IMPL_H

#include "DataPlane.h"
#include "InputPlane.h"

#include <nonlinfit/index_of.h>
#include "EvaluationTags.h"
#include <dStorm/engine/InputTraits.h>

#include "dejagnu.h"
#include <dStorm/traits/ScaledProjection.h>
#include <boost/units/cmath.hpp>

using namespace nonlinfit::plane;

namespace dStorm {
namespace guf {

std::auto_ptr<DataPlane> InputPlane::set_image( const Image& image, const Spot& position ) const {
    int index = index_finder.get_evaluation_tag_index( position );
    std::auto_ptr<DataPlane> rv = extractor_table.get( index ).extract_data( image, position );
    rv->tag_index = index;
    return rv;
}

InputPlane::InputPlane( const Config& c, const engine::InputPlane& plane )
: optics( Spot::Constant( Spot::Scalar( c.fit_window_size() ) ), plane ),
  index_finder( c.allow_disjoint(), ! c.double_computation(), optics ),
  extractor_table( optics )
{
}

void test_DataPlane_scaled( TestState& state )
{
    Config config;
    config.fit_window_size = 1600 * si::nanometre;
    config.allow_disjoint = true;

    engine::InputPlane traits;
    traits.image.size.fill( 100 * camera::pixel );
    traits.image.set_resolution( 0, 230 * si::nanometre / camera::pixel );
    traits.image.set_resolution( 1, 120 * si::nanometre / camera::pixel );
    traits.optics.set_projection_factory( traits::test_scaled_projection() );
    traits.create_projection();

    dStorm::engine::Image2D image( traits.image.size );
    for (int x = 0; x < traits.image.size.x().value(); ++x)
        for (int y = 0; y < traits.image.size.y().value(); ++y)
            image(x,y) = x + y;

    Spot position;
    position.x() = 5600.0E-9 * si::metre;
    position.y() = 3000.0E-9 * si::metre;

    InputPlane scaled( config, traits );
    std::auto_ptr<DataPlane> data1 = scaled.set_image( image , position );
    state( data1->tag_index == nonlinfit::index_of< 
        evaluation_tags, 
        nonlinfit::plane::xs_disjoint<double,PSF::LengthUnit,14>::type >::value,
        "DataPlane selects disjoint fitting when applicable" );
    state( abs( data1->pixel_size - 230E-9 * si::metre * 120E-9 * si::metre ) < pow<2>(1E-9 * si::metre),
        "Disjoint fitting has correct pixel size" ); 
    state( abs( data1->highest_pixel.x() - 7130E-9 * si::metre ) < 1E-9 * si::metre,
        "Highest pixel is located correctly (X)" );
    state( abs( data1->highest_pixel.y() - 4560E-9 * si::metre ) < 1E-9 * si::metre,
        "Highest pixel is located correctly (Y)" );
    state( abs( data1->integral - 18711 ) < 1,
        "Integral is correctly computed" );
    state( abs( data1->peak_intensity - 69 ) < 1,
        "Peak intensity is correctly computed" );
    state( abs( data1->background_estimate - 43 ) < 1,
        "Quarter percentile pixel is correctly computed" );
    state( abs( data1->standard_deviation[0] - 870E-9 * si::meter ) < 1E-9 * si::meter,
        "Sigma X is correctly computed" );
    state( abs( data1->standard_deviation[1] - 636E-9 * si::meter ) < 1E-9 * si::meter,
        "Sigma Y is correctly computed" );
    state( data1->pixel_count == 378, "Pixel count is correct" );
}

void test_DataPlane( TestState& state ) {
    test_DataPlane_scaled( state );
}

}
}

#endif
