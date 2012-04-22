#ifndef DSTORM_GUF_DATAPLANE_IMPL_H
#define DSTORM_GUF_DATAPLANE_IMPL_H

#include "DataPlane.h"
#include "InputPlane.h"

#include "TransformedImage.hpp"
#include "fit_position_out_of_range.h"
#include <nonlinfit/plane/DisjointData.hpp>
#include <nonlinfit/plane/JointData.hpp>
#include "Centroid.h"
#include <nonlinfit/index_of.h>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>
#include "EvaluationTags.h"
#include "mle_converter.h"
#include <boost/type_traits/is_same.hpp>
#include "Statistics.h"
#include <dStorm/engine/InputTraits.h>

#include "dejagnu.h"
#include <dStorm/traits/ScaledProjection.h>
#include <boost/units/cmath.hpp>

using namespace nonlinfit::plane;

namespace dStorm {
namespace guf {

template <typename Data>
const Statistics<2> InputPlane::set_data( Data& d, const Image& i, const Spot& s ) const
{
    mle_converter t(dark_current, photon_response_);
    return transformation.set_data( d, i, s, t );
}

std::auto_ptr<DataPlane> InputPlane::set_image( const Image& image, const Spot& position ) const {
    int index = index_finder.get_evaluation_tag_index( position );
    return extractor_table.get( index ).extract_data( image, position );
}

template <typename Tag>
struct TaggedDataPlane
: public DataPlane {
    typename Tag::Data data;
    Statistics<2> image_stats;
    virtual const void* get_data() const { return &data; }
    virtual std::auto_ptr<Centroid> _residue_centroid() const { 
        std::auto_ptr<Centroid> rv( new Centroid( data.min, data.max ) );  
        for ( typename Tag::Data::const_iterator i= data.begin(), e = data.end(); i != e; ++i )
            rv->add( i->position().template cast< Centroid::Coordinate >(), i->residue() );

        return rv;
    }
    virtual quantity< si::area > pixel_size() const 
        { return quantity< si::area >(data.pixel_size); }
    virtual const Statistics<2>& get_statistics() const { return image_stats; }

  public:
    TaggedDataPlane( 
        const InputPlane& input,
        const dStorm::engine::Image2D& image,
        const guf::Spot& position )
    : DataPlane(input, nonlinfit::index_of< evaluation_tags, Tag >::value )
    {
        image_stats = input.set_data( data, image, position );
        if ( image_stats.pixel_count <= 0 ) 
            throw fit_position_out_of_range();
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

ScheduleIndexFinder::ScheduleIndexFinder( const Config& config, const engine::InputPlane& plane, const InputPlane& input_plane )
: do_disjoint( config.allow_disjoint() && 
    plane.projection().supports_guaranteed_row_width() ),
  use_floats( !config.double_computation() ),
  plane(plane),
  input_plane(input_plane)
{
}

struct ScheduleIndexFinder::set_if_appropriate
{
    typedef void result_type;
    const ScheduleIndexFinder& m;

    set_if_appropriate( const ScheduleIndexFinder& m ) : m(m) {}

    template <int ChunkSize, typename Num, typename P1, typename P2>
    bool is_appropriate( 
        nonlinfit::plane::Disjoint<Num,ChunkSize,P1,P2> t,
        int width
    ) const { 
        const int slack = boost::is_same<Num,float>::value ? 1 : 0;
        return m.do_disjoint && 
            (ChunkSize >= (width-slack))  &&
            (ChunkSize < (width + 2 + slack)) &&
            (m.use_floats == boost::is_same<Num,float>::value);
    }

    template <int ChunkSize, typename Num, typename P1, typename P2>
    bool is_appropriate( 
        nonlinfit::plane::Joint<Num,ChunkSize,P1,P2>, int
    ) const { 
        return (m.use_floats == boost::is_same<Num,float>::value); 
    }

    template <typename Tag>
    void operator()( int& result, int window_width, Tag t ) const
    {
        if ( is_appropriate( t, window_width ) ) {
            result = std::min( result, nonlinfit::index_of<evaluation_tags,Tag>::value );
        }
    }
};

int ScheduleIndexFinder::get_evaluation_tag_index( const guf::Spot& position ) const
{
    int rv = boost::mpl::size<evaluation_tags>::value;
    boost::mpl::for_each< evaluation_tags >( 
        boost::bind( 
            set_if_appropriate(*this), boost::ref(rv), input_plane.get_fit_window_width(position), _1 ) );
    if ( rv == boost::mpl::size<evaluation_tags>::value )
        throw std::logic_error("No appropriate-sized fitter found");
    return rv;
}

fit_position_out_of_range::fit_position_out_of_range()
: std::runtime_error("Selected fit position not in all layers of image") {}

InputPlane::InputPlane( const Config& c, const engine::InputPlane& plane )
: index_finder( c, plane, *this ),
  extractor_table( evaluation_tags(), *this ),
  im_size( plane.image.size.array() - 1 * camera::pixel ),
  transformation( guf::Spot::Constant( guf::Spot::Scalar( c.fit_window_size() ) ),
                  plane )
{
    const traits::Optics& t = plane.optics;
    photon_response_ = t.photon_response.get_value_or( 1 * camera::ad_count );
    dark_current = quantity<camera::intensity,int>
        (t.dark_current.get_value_or(0*camera::ad_count));
    if ( t.background_stddev.is_initialized() && t.photon_response.is_initialized() )
        background_noise_variance_ = pow( *t.background_stddev / *t.photon_response, 2 );
    has_precision 
        = t.photon_response.is_initialized() &&
          background_noise_variance_.is_initialized();
    poisson_background_ = 
        t.dark_current.is_initialized() && t.photon_response.is_initialized();
}

int InputPlane::get_fit_window_width(const Spot& at) const
{
    typename TransformedImage::Bounds bounds = 
        transformation.cut_region( at, im_size );
    int width = (bounds(0,1) - bounds(0,0)).value() + 1;
    return width;
}

template <typename Tag>
struct DataExtractorImpl : public DataExtractor {
    const InputPlane& input;
    std::auto_ptr<DataPlane> extract_data_( const engine::Image2D& image, const Spot& position ) const {
        return std::auto_ptr<DataPlane>( new TaggedDataPlane<Tag>( input, image, position ) );
    }
public:
    DataExtractorImpl( const InputPlane& input ) : input(input) {}
};

struct DataExtractorTable::instantiator {
    typedef void result_type;
    template <typename Tag>
    void operator()( DataExtractorTable& target, Tag ) 
    {
        target.table_.push_back( new DataExtractorImpl<Tag>(target.input) );
    }
};


template <typename EvaluationSchedule>
DataExtractorTable::DataExtractorTable( 
    EvaluationSchedule, 
    const InputPlane& input
) : input(input) 
{
    boost::mpl::for_each< EvaluationSchedule >(
        boost::bind(
            instantiator(),
            boost::ref(*this),
            _1 ) );
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
    state( data1->tag_index() == nonlinfit::index_of< 
        evaluation_tags, 
        nonlinfit::plane::xs_disjoint<double,PSF::LengthUnit,14>::type >::value,
        "DataPlane selects disjoint fitting when applicable" );
    state( abs( data1->pixel_size() - 230E-9 * si::metre * 120E-9 * si::metre ) < pow<2>(1E-9 * si::metre),
        "Disjoint fitting has correct pixel size" ); 
    state( &data1->input_plane() == &scaled,
        "Input plane is correctly referenced" );
    Statistics<2> stats = data1->get_statistics();
    state( abs( stats.highest_pixel.x() - 7130E-9 * si::metre ) < 1E-9 * si::metre,
        "Highest pixel is located correctly (X)" );
    state( abs( stats.highest_pixel.y() - 4560E-9 * si::metre ) < 1E-9 * si::metre,
        "Highest pixel is located correctly (Y)" );
    state( abs( stats.integral - 18711 * camera::ad_count ) < 1 * camera::ad_count,
        "Integral is correctly computed" );
    state( abs( stats.peak_intensity - 69 * camera::ad_count ) < 1 * camera::ad_count,
        "Peak intensity is correctly computed" );
    state( abs( stats.quarter_percentile_pixel - 43 * camera::ad_count ) < 1 * camera::ad_count,
        "Quarter percentile pixel is correctly computed" );
    state( abs( stats.sigma[0] - 870E-9 * si::meter ) < 1E-9 * si::meter,
        "Sigma X is correctly computed" );
    state( abs( stats.sigma[1] - 636E-9 * si::meter ) < 1E-9 * si::meter,
        "Sigma Y is correctly computed" );
    state( stats.pixel_count == 378, "Pixel count is correct" );
}

void test_DataPlane( TestState& state ) {
    test_DataPlane_scaled( state );
}

}
}

#endif
