#include <algorithm>

#include <boost/test/unit_test.hpp>

#include <nonlinfit/plane/DisjointData.hpp>
#include <nonlinfit/plane/JointData.hpp>

#include "PlaneCreator.h"
#include "Plane.h"
#include "guf/guf/EvaluationTags.h"
#include "fit_position_out_of_range.h"
#include "Optics.h"
#include "PlaneImpl.hpp"

namespace dStorm {
namespace fit_window {

using namespace boost::units;
using namespace boost::accumulators;

fit_position_out_of_range::fit_position_out_of_range()
: std::runtime_error("Selected fit position not in all layers of image") {}

template <typename Tag>
struct PlaneCreatorImpl : public PlaneCreator {
    const Optics& input;
    std::auto_ptr<Plane> extract_data_( const engine::Image2D& image, const Spot& position ) const {
        return std::auto_ptr<Plane>( new PlaneImpl<Tag>( input, image, position ) );
    }
public:
    PlaneCreatorImpl( const Optics& input ) : input(input) {}
};

template <typename EvaluationSchedule>
struct PlaneCreatorTable::instantiator {
    typedef void result_type;
    template <typename Tag>
    void operator()( PlaneCreatorTable& target, Tag ) 
    {
        target.table_.push_back( new PlaneCreatorImpl<Tag>(target.optics) );
    }
};


template <typename EvaluationSchedule>
PlaneCreatorTable::PlaneCreatorTable( 
    EvaluationSchedule, 
    const Optics& input
) : optics(input) 
{
    boost::mpl::for_each< EvaluationSchedule >(
        boost::bind( instantiator<EvaluationSchedule>(), boost::ref(*this), _1 ) );
}

PlaneCreatorTable::PlaneCreatorTable( const Optics& input) 
: optics(input) 
{
    boost::mpl::for_each< guf::evaluation_tags >(
        boost::bind( instantiator<guf::evaluation_tags>(), boost::ref(*this), _1 ) );
}

}
}

#include <dStorm/engine/InputPlane.h>
#include <dStorm/units/nanolength.h>
#include <dStorm/traits/ScaledProjection.h>

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

    Spot max_distance( Spot::Constant( 1.6E-6f * si::meter ) );
    Optics optics( max_distance, traits );

    Spot position;
    position.x() = 5600.0E-9 * si::metre;
    position.y() = 3000.0E-9 * si::metre;

    dStorm::engine::Image2D image( traits.image.size );
    for (int x = 0; x < traits.image.size.x().value(); ++x)
        for (int y = 0; y < traits.image.size.y().value(); ++y)
            image(x,y) = x + y;

    PlaneCreatorImpl< nonlinfit::plane::xs_disjoint<double,boost::units::si::microlength,14>::type > extractor(optics);
    std::auto_ptr<Plane> data = extractor.extract_data( image, position );
    BOOST_CHECK_CLOSE( data->pixel_size.value(), 230E-9 * 120E-9, 1 );
    BOOST_CHECK_CLOSE( data->highest_pixel.x().value(), 7130E-9, 1 );
    BOOST_CHECK_CLOSE( data->highest_pixel.y().value(), 4560E-9, 1 );
    BOOST_CHECK_CLOSE( data->integral, 18711, 1 );
    BOOST_CHECK_CLOSE( data->peak_intensity, 69, 1 );
    BOOST_CHECK_CLOSE( data->background_estimate, 43, 1 );
    BOOST_CHECK_CLOSE( data->standard_deviation[0].value(), 870E-9, 1 );
    BOOST_CHECK_CLOSE( data->standard_deviation[1].value(), 636E-9, 1 );
    BOOST_CHECK_EQUAL( data->pixel_count, 378 );
}

boost::unit_test::test_suite* test_FittingRegionCreator() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "FittingRegionCreator" );
    rv->add( BOOST_TEST_CASE( &test_central_extraction ) );
    return rv;
}

}
}
