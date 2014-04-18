#include "fit_window/PlaneCreator.hpp"

#include <boost/test/unit_test.hpp>
#include "engine/InputPlane.h"
#include "units/nanolength.h"
#include "traits/ScaledProjection.h"
#include <nonlinfit/plane/DisjointData.hpp>
#include <nonlinfit/plane/Disjoint.h>

#define BOOST_CHECK_CLOSE_UNIT(u,x,y,t) BOOST_CHECK_CLOSE( x/u, y/u, t/u )

namespace dStorm {
namespace fit_window {
static void test_central_extraction() {
    engine::InputPlane traits;
    traits.image.size.fill( 100 * camera::pixel );
    traits.image.set_resolution( 0, 230 * si::nanometre / camera::pixel );
    traits.image.set_resolution( 1, 120 * si::nanometre / camera::pixel );
    traits.optics.set_projection_factory(  traits::test_scaled_projection() );
    traits.create_projection();

    Spot max_distance( Spot::Constant( 1.6 ) );
    Optics optics( max_distance, traits );

    Spot position;
    position.x() = 5.6;
    position.y() = 3.0;

    dStorm::engine::Image2D image( traits.image.size );
    for (int x = 0; x < traits.image.size.x().value(); ++x)
        for (int y = 0; y < traits.image.size.y().value(); ++y)
            image(x,y) = x + y;

    PlaneCreatorImpl< nonlinfit::plane::xs_disjoint<double,14>::type > extractor(optics);
    std::auto_ptr<Plane> data = extractor.extract_data( image, position );
    BOOST_CHECK_CLOSE( data->pixel_size, 230E-3 * 120E-3, 1 );
    BOOST_CHECK_CLOSE( data->highest_pixel.x(), 7130E-3, 1 );
    BOOST_CHECK_CLOSE( data->highest_pixel.y(), 4560E-3, 1 );
    BOOST_CHECK_CLOSE( data->integral, 18711, 1 );
    BOOST_CHECK_CLOSE( data->peak_intensity, 69, 1 );
    BOOST_CHECK_CLOSE( data->background_estimate, 43, 1 );
    BOOST_CHECK_CLOSE( data->standard_deviation[0], 870E-3, 1 );
    BOOST_CHECK_CLOSE( data->standard_deviation[1], 636E-3, 1 );
    BOOST_CHECK_EQUAL( data->pixel_count, 378 );
}

boost::unit_test::test_suite* test_FittingRegionCreator() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "FittingRegionCreator" );
    rv->add( BOOST_TEST_CASE( &test_central_extraction ) );
    return rv;
}

}
}
