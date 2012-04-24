#ifndef DSTORM_GUF_OPTICS_HPP
#define DSTORM_GUF_OPTICS_HPP

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/weighted_variance.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <algorithm>

#include "Optics.h"

namespace dStorm {
namespace guf {

using namespace boost::units;
using namespace boost::accumulators;

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

template <typename Data >
Statistics<2> Optics::get_data_from_image( 
    Data& target,
    const engine::Image2D& image, const Spot& center 
) const {
    typedef typename Data::data_point::Length Length;
    typedef quantity< typename Length::unit_type > DoubleLength;
    typedef typename Data::DataRow DataRow;

    target.pixel_size = quantity<typename Data::AreaUnit>(
        projection.pixel_size( 
            projection.nearest_point_in_image_space( center ) ) );

    target.min = center.template cast< DoubleLength >();
    target.max = center.template cast< DoubleLength >();

    Statistics<2> rv;
    rv.peak_intensity = rv.integral = 0 * camera::ad_count;
    rv.peak_pixel_area = quantity< si::area >( target.pixel_size );

    traits::Projection::ROISpecification roi_request( center, max_distance );
    roi_request.guaranteed_row_width = row_width( target );
    traits::Projection::ROI points = 
        projection.cut_region_of_interest( roi_request );

    target.clear();
    target.reserve( points.size() );
    std::vector< engine::Image2D::Pixel > pixels;
    pixels.reserve( points.size() );
    std::back_insert_iterator<Data> o = std::back_inserter( target );
    for ( traits::Projection::ROI::const_iterator i = points.begin(); i != points.end(); ++i )
    {
        assert( image.contains( i->image_position ) );

        for (int d = 0; d < 2; ++d) {
            target.min[d] = std::min( DoubleLength(i->sample_position[d]), target.min[d] );
            target.max[d] = std::max( DoubleLength(i->sample_position[d]), target.max[d] );
        }

        Spot sample = i->sample_position;
        const typename Data::value_type::Intensity value = 
            std::max( 0.0, absolute_in_photons( image( i->image_position ) * camera::ad_count ) );
        pixels.push_back( value );
        rv.integral += value * camera::ad_count;
        if ( value * camera::ad_count >= rv.peak_intensity ) {
            rv.peak_intensity = value * camera::ad_count;
            rv.highest_pixel = sample;
        }

        *o++ = typename Data::value_type( sample, value );
    }

    target.pad_last_chunk();
    rv.pixel_count = pixels.size();

    if ( ! pixels.empty() ) {
        std::vector<engine::Image2D::Pixel>::iterator qp = pixels.begin() + pixels.size() * background_part;
        std::nth_element( pixels.begin(), qp, pixels.end());
        rv.quarter_percentile_pixel = *qp * camera::ad_count;

        accumulator_set<double, stats<tag::weighted_variance(lazy)>, double> acc[2];

        for ( typename Data::const_iterator i = target.begin(); i != target.end(); ++i )
        {
            for (int dim = 0; dim < 2; ++dim) {
                Length offset = i->position(dim) - Length(rv.highest_pixel[dim]);
                double intensity_above_background = 
                    std::max(0.0, (i->value() - rv.quarter_percentile_pixel.value()));
                acc[dim]( offset.value(),
                    weight = intensity_above_background );
            }
        }
        for (int dim = 0; dim < 2; ++dim)
            rv.sigma[dim] = quantity<si::length>( 
                Length::from_value(sqrt( weighted_variance(acc[dim]) ) ) );
    }
    return rv;
}

}
}

#endif
