#include <algorithm>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/weighted_variance.hpp>
#include <boost/bind/bind.hpp>
#include <boost/units/io.hpp>

#include <nonlinfit/plane/DisjointData.hpp>
#include <nonlinfit/plane/JointData.hpp>

#include "Centroid.h"
#include "DataExtractor.h"
#include "DataPlane.h"
#include "EvaluationTags.h"
#include "fit_position_out_of_range.h"
#include "Optics.hpp"

namespace dStorm {
namespace guf {

using namespace boost::units;
using namespace boost::accumulators;

fit_position_out_of_range::fit_position_out_of_range()
: std::runtime_error("Selected fit position not in all layers of image") {}

template <typename Num, typename LengthUnit, int ChunkSize>
inline boost::optional< quantity<camera::length,int> >
row_width( const nonlinfit::plane::DisjointData<Num,LengthUnit,ChunkSize>& )
{
    return ChunkSize * camera::pixel;
}

template <typename Num, typename LengthUnit, int ChunkSize>
inline boost::optional< quantity<camera::length,int> >
row_width( const nonlinfit::plane::JointData<Num,LengthUnit,ChunkSize>& )
{
    return boost::optional< quantity<camera::length,int> >();
}

template <typename Tag>
struct DataPlaneImpl
: public DataPlane {
    typename Tag::Data data;
    Statistics<2> image_stats;
    const void* get_data() const { return &data; }
    std::auto_ptr<Centroid> _residue_centroid() const { 
        std::auto_ptr<Centroid> rv( new Centroid( data.min, data.max ) );  
        for ( typename Tag::Data::const_iterator i= data.begin(), e = data.end(); i != e; ++i )
            rv->add( i->position().template cast< Centroid::Coordinate >(), i->residue() );

        return rv;
    }
    quantity< si::area > pixel_size() const 
        { return quantity< si::area >(data.pixel_size); }
    const Statistics<2>& get_statistics() const { return image_stats; }

  public:
    DataPlaneImpl( 
        const Optics& input,
        const dStorm::engine::Image2D& image,
        const guf::Spot& position );

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <typename Tag>
DataPlaneImpl<Tag>::DataPlaneImpl( 
    const Optics& optics,
    const dStorm::engine::Image2D& image,
    const guf::Spot& position 
) : DataPlane(optics, nonlinfit::index_of< evaluation_tags, Tag >::value )
{
    typedef typename Tag::Data Data;
    typedef typename Data::data_point::Length Length;
    typedef quantity< typename Length::unit_type > DoubleLength;
    typedef typename Data::DataRow DataRow;

    const float background_part = 0.25;

    data.pixel_size = quantity<typename Data::AreaUnit>( optics.pixel_size(position) );

    /* Initialize iteratively computed statistics */
    data.min = position.template cast< DoubleLength >();
    data.max = position.template cast< DoubleLength >();
    image_stats.peak_intensity = image_stats.integral = 0 * camera::ad_count;
    image_stats.peak_pixel_area = quantity< si::area >( data.pixel_size );

    traits::Projection::ROI points 
        = optics.get_region_of_interest( position, row_width(data) );

    if ( points.size() <= 0 ) 
        throw fit_position_out_of_range();

    data.clear();
    data.reserve( points.size() );
    /* Keep a list of the inserted pixel values for later statistical usage */
    std::vector< engine::Image2D::Pixel > pixels;
    pixels.reserve( points.size() );
    std::back_insert_iterator<Data> o = std::back_inserter( data );
    for ( traits::Projection::ROI::const_iterator i = points.begin(); i != points.end(); ++i )
    {
        assert( image.contains( i->image_position ) );

        for (int d = 0; d < 2; ++d) {
            data.min[d] = std::min( DoubleLength(i->sample_position[d]), data.min[d] );
            data.max[d] = std::max( DoubleLength(i->sample_position[d]), data.max[d] );
        }

        Spot sample = i->sample_position;
        const typename Data::value_type::Intensity value = 
            std::max( 0.0, optics.absolute_in_photons( image( i->image_position ) * camera::ad_count ) );
        pixels.push_back( value );
        image_stats.integral += value * camera::ad_count;
        if ( value * camera::ad_count >= image_stats.peak_intensity ) {
            image_stats.peak_intensity = value * camera::ad_count;
            image_stats.highest_pixel = sample;
        }

        *o++ = typename Data::value_type( sample, value );
    }

    data.pad_last_chunk();
    image_stats.pixel_count = pixels.size();

    if ( ! pixels.empty() ) {
        std::vector<engine::Image2D::Pixel>::iterator qp = pixels.begin() + pixels.size() * background_part;
        std::nth_element( pixels.begin(), qp, pixels.end());
        image_stats.quarter_percentile_pixel = *qp * camera::ad_count;

        accumulator_set<double, stats<tag::weighted_variance(lazy)>, double> acc[2];

        for ( typename Data::const_iterator i = data.begin(); i != data.end(); ++i )
        {
            for (int dim = 0; dim < 2; ++dim) {
                Length offset = i->position(dim) - Length(image_stats.highest_pixel[dim]);
                double intensity_above_background = 
                    std::max(0.0, (i->value() - image_stats.quarter_percentile_pixel.value()));
                acc[dim]( offset.value(),
                    weight = intensity_above_background );
            }
        }
        for (int dim = 0; dim < 2; ++dim)
            image_stats.sigma[dim] = quantity<si::length>( 
                Length::from_value(sqrt( weighted_variance(acc[dim]) ) ) );
    }
}

template <typename Tag>
struct DataExtractorImpl : public DataExtractor {
    const Optics& input;
    std::auto_ptr<DataPlane> extract_data_( const engine::Image2D& image, const Spot& position ) const {
        return std::auto_ptr<DataPlane>( new DataPlaneImpl<Tag>( input, image, position ) );
    }
public:
    DataExtractorImpl( const Optics& input ) : input(input) {}
};

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
        boost::bind( instantiator(), boost::ref(*this), _1 ) );
}

DataExtractorTable::DataExtractorTable( const Optics& input) 
: optics(input) 
{
    boost::mpl::for_each< evaluation_tags >(
        boost::bind( instantiator(), boost::ref(*this), _1 ) );
}

}
}
