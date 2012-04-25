#include <algorithm>

#include <boost/test/unit_test.hpp>

#include <nonlinfit/plane/DisjointData.hpp>
#include <nonlinfit/plane/JointData.hpp>

#include "DataExtractor.h"
#include "FittingRegion.h"
#include "EvaluationTags.h"
#include "fit_position_out_of_range.h"
#include "Optics.hpp"
#include "FittingRegionImpl.hpp"

namespace dStorm {
namespace guf {

using namespace boost::units;
using namespace boost::accumulators;

fit_position_out_of_range::fit_position_out_of_range()
: std::runtime_error("Selected fit position not in all layers of image") {}

template <typename Tag>
struct DataExtractorImpl : public DataExtractor {
    const Optics& input;
    std::auto_ptr<FittingRegion> extract_data_( const engine::Image2D& image, const Spot& position ) const {
        return std::auto_ptr<FittingRegion>( new FittingRegionImpl<Tag>( input, image, position ) );
    }
public:
    DataExtractorImpl( const Optics& input ) : input(input) {}
};

template <typename EvaluationSchedule>
struct DataExtractorTable::instantiator {
    typedef void result_type;
    template <typename Tag>
    void operator()( DataExtractorTable& target, Tag ) 
    {
        target.table_.push_back( new DataExtractorImpl<Tag>(target.optics) );
    }
};


template <typename EvaluationSchedule>
DataExtractorTable::DataExtractorTable( 
    EvaluationSchedule, 
    const Optics& input
) : optics(input) 
{
    boost::mpl::for_each< EvaluationSchedule >(
        boost::bind( instantiator<EvaluationSchedule>(), boost::ref(*this), _1 ) );
}

DataExtractorTable::DataExtractorTable( const Optics& input) 
: optics(input) 
{
    boost::mpl::for_each< evaluation_tags >(
        boost::bind( instantiator<evaluation_tags>(), boost::ref(*this), _1 ) );
}

}
}

#include <dStorm/engine/InputPlane.h>
#include <dStorm/units/nanolength.h>
#include <dStorm/traits/ScaledProjection.h>

#define BOOST_CHECK_CLOSE_UNIT(u,x,y,t) BOOST_CHECK_CLOSE( x/u, y/u, t/u )

namespace dStorm {
namespace guf {
static void test_central_extraction() {
    engine::InputPlane traits;
    traits.image.size.fill( 100 * camera::pixel );
    traits.image.set_resolution( 0, 230 * si::nanometre / camera::pixel );
    traits.image.set_resolution( 1, 120 * si::nanometre / camera::pixel );
    traits.optics.set_projection_factory(  traits::test_scaled_projection() );
    traits.create_projection();

    Spot max_distance( guf::Spot::Constant( 1.6E-6f * si::meter ) );
    Optics optics( max_distance, traits );

    Spot position;
    position.x() = 5600.0E-9 * si::metre;
    position.y() = 3000.0E-9 * si::metre;

    dStorm::engine::Image2D image( traits.image.size );
    for (int x = 0; x < traits.image.size.x().value(); ++x)
        for (int y = 0; y < traits.image.size.y().value(); ++y)
            image(x,y) = x + y;

    DataExtractorImpl< nonlinfit::plane::xs_disjoint<double,PSF::LengthUnit,14>::type > extractor(optics);
    std::auto_ptr<FittingRegion> data = extractor.extract_data( image, position );
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

boost::unit_test::test_suite* test_DataExtractor() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "DataExtractor" );
    rv->add( BOOST_TEST_CASE( &test_central_extraction ) );
    return rv;
}

}
}
