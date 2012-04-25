#include "ScheduleIndexFinder.hpp"
#include <boost/test/unit_test.hpp>
#include <dStorm/engine/InputPlane.h>
#include <dStorm/traits/ScaledProjection.h>
#include <boost/mpl/vector.hpp>
#include <nonlinfit/plane/Disjoint.h>
#include <nonlinfit/plane/Joint.h>

namespace dStorm {
namespace fit_window {

int ScheduleIndexFinder::get_evaluation_tag_index( const Spot& position ) const
{
    int width = std::min( int(table.size())-1, optics.get_fit_window_width(position) );
    return table[width];
}

typedef boost::units::si::length LengthUnit;
typedef boost::mpl::vector<
    nonlinfit::plane::xs_disjoint<double,LengthUnit,14>::type,
    nonlinfit::plane::xs_disjoint<double,LengthUnit,10>::type,
    nonlinfit::plane::xs_disjoint<double,LengthUnit,8>::type,
    nonlinfit::plane::xs_joint<double,LengthUnit,8>::type,
    nonlinfit::plane::xs_disjoint<float,LengthUnit,12>::type,
    nonlinfit::plane::xs_disjoint<float,LengthUnit,8>::type,
    nonlinfit::plane::xs_joint<float,LengthUnit,8>::type
> test_tags;

class ScheduleIndexFinderFixture {
    engine::InputPlane traits;
    Spot max_distance;
    Optics optics;

    engine::InputPlane mock_input_plane( boost::shared_ptr< const traits::ProjectionFactory > projection ) {
        engine::InputPlane traits;
        traits.image.size.fill( 100 * camera::pixel );
        traits.image.set_resolution( 0, 230E-9 * si::metre / camera::pixel );
        traits.image.set_resolution( 1, 120E-9 * si::metre / camera::pixel );
        traits.optics.set_projection_factory( projection );
        traits.create_projection();
        return traits;
    }

public:
    ScheduleIndexFinderFixture( boost::shared_ptr< const traits::ProjectionFactory > projection )
    : traits( mock_input_plane(projection) ),
      max_distance( Spot::Constant( 1.6E-6f * si::meter ) ),
      optics( max_distance, traits ) {}

    void check_indices( Spot position, int disjoint_double, int disjoint_float ) {
        BOOST_CHECK_EQUAL( ScheduleIndexFinder(test_tags(),true,false,optics,20)
            .get_evaluation_tag_index( position ), disjoint_float );
        BOOST_CHECK_EQUAL( ScheduleIndexFinder(test_tags(),false,false,optics,20)
            .get_evaluation_tag_index( position ), 6 );
        BOOST_CHECK_EQUAL( ScheduleIndexFinder(test_tags(),true,true,optics,20)
            .get_evaluation_tag_index( position ), disjoint_double );
        BOOST_CHECK_EQUAL( ScheduleIndexFinder(test_tags(),false,true,optics,20)
            .get_evaluation_tag_index( position ), 3 );
    }
};

static void test_central_selection() {
    ScheduleIndexFinderFixture optics( traits::test_scaled_projection() );
    Spot position;
    position.x() = 5600.0E-9 * si::metre;
    position.y() = 3000.0E-9 * si::metre;
    optics.check_indices( position, 0, 6 );
}

static void test_border_selection() {
    ScheduleIndexFinderFixture optics( traits::test_scaled_projection() );
    Spot position;
    position.x() = 800.0E-9 * si::metre;
    position.y() = 3000.0E-9 * si::metre;
    optics.check_indices( position, 3, 4 );
}

static void test_closer_border_selection() {
    ScheduleIndexFinderFixture optics( traits::test_scaled_projection() );
    Spot position;
    position.x() = 650.0E-9 * si::metre;
    position.y() = 3000.0E-9 * si::metre;
    optics.check_indices( position, 1, 4 );
}

boost::unit_test::test_suite* test_schedule_index_finder() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "ScheduleIndexFinder" );
    rv->add( BOOST_TEST_CASE( &test_central_selection ) );
    rv->add( BOOST_TEST_CASE( &test_border_selection ) );
    rv->add( BOOST_TEST_CASE( &test_closer_border_selection ) );
    return rv;
}

}
}
