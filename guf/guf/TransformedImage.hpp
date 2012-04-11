#ifndef NONLINFIT_IMAGE_TRANSFORMEDIMAGE_IMPL_H
#define NONLINFIT_IMAGE_TRANSFORMEDIMAGE_IMPL_H

#include <Eigen/StdVector>
#include "TransformedImage.h"
#include "TransformedImage_cut_region.h"
#include "Statistics.h"
#include "debug.h"
#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>
#include <algorithm>
#include <nonlinfit/plane/JointData.hpp>
#include <dStorm/traits/Projection.h>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/weighted_variance.hpp>
#include <boost/accumulators/statistics/stats.hpp>

#include <boost/foreach.hpp>

namespace dStorm {
namespace guf {

using namespace boost::units;
using namespace boost::accumulators;

template <typename LengthUnit>
void 
TransformedImage<LengthUnit>::set_generic_data( 
    nonlinfit::plane::GenericData<LengthUnit>& g, const Spot& center ) const
{
    g.pixel_size = quantity<typename nonlinfit::plane::GenericData<LengthUnit>::AreaUnit>(
        optics.projection().pixel_size( 
            optics.projection().nearest_point_in_image_space( center ) ) );

    /** This method initializes the min and max fields to the center coordinate. */
    g.min = center.template cast< quantity<LengthUnit> >();
    g.max = center.template cast< quantity<LengthUnit> >();
}

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

template <typename LengthUnit>
template <typename PixelType, typename Data, typename Transform >
Statistics<2>
TransformedImage<LengthUnit>::set_data( 
   Data& target,
   const dStorm::Image< PixelType, 2 >& image,
   const Spot& center,
   const Transform& transform )  const
{
    typedef typename Data::DataRow DataRow;
    set_generic_data(target, center);
    Statistics<2> rv;
    rv.peak_intensity = rv.integral = 0 * camera::ad_count;
    rv.peak_pixel_area = quantity< si::area >( target.pixel_size );

    traits::Projection::ROISpecification roi_request( center, max_distance );
    roi_request.guaranteed_row_width = row_width( target );
    traits::Projection::ROI points = 
        optics.projection().cut_region_of_interest( roi_request );

    target.clear();
    target.reserve( points.size() );
    std::vector< PixelType > pixels;
    pixels.reserve( points.size() );
    std::back_insert_iterator<Data> o = std::back_inserter( target );
    typedef Bounds::Scalar Pixel;
    for ( traits::Projection::ROI::const_iterator i = points.begin(); i != points.end(); ++i )
    {
        if ( ! image.contains( i->image_position ) )
            continue;

        for (int d = 0; d < 2; ++d) {
            target.min[d] = std::min( quantity<LengthUnit>(i->sample_position[d]), target.min[d] );
            target.max[d] = std::max( quantity<LengthUnit>(i->sample_position[d]), target.max[d] );
        }

        Spot sample = i->sample_position;
        const typename Data::value_type::Intensity value = transform( image( i->image_position ) );
        pixels.push_back( value );
        rv.integral += value * boost::units::camera::ad_count;
        if ( value * camera::ad_count >= rv.peak_intensity ) {
            rv.peak_intensity = value * camera::ad_count;
            rv.highest_pixel = sample;
        }

        *o++ = typename Data::value_type( sample, value );
    }

    target.pad_last_chunk();
    rv.pixel_count = pixels.size();

    if ( ! pixels.empty() ) {
        typename std::vector<PixelType>::iterator qp = pixels.begin() + pixels.size() / 4;
        std::nth_element( pixels.begin(), qp, pixels.end());
        rv.quarter_percentile_pixel = *qp * camera::ad_count;

        accumulator_set<double, stats<tag::weighted_variance(lazy)>, double> acc[2];

        for ( typename Data::const_iterator i = target.begin(); i != target.end(); ++i )
        {
            for (int dim = 0; dim < 2; ++dim) {
                quantity<LengthUnit> offset = i->position(dim) - quantity< LengthUnit >(rv.highest_pixel[dim]);
                acc[dim]( offset.value(),
                    weight = (i->value() - rv.quarter_percentile_pixel.value()) );
            }
        }
        for (int dim = 0; dim < 2; ++dim)
            rv.sigma[dim] = quantity<si::length>( 
                quantity<LengthUnit>::from_value(sqrt( weighted_variance(acc[dim]) ) ) );
    }
    return rv;
}

}
}

#endif
