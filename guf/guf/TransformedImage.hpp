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
#include <dStorm/traits/ScaledProjection.h>

namespace dStorm {
namespace guf {

using namespace boost::units;

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

template <typename LengthUnit>
template <typename PixelType, typename Num, int ChunkSize, typename Transform >
Statistics<2>
TransformedImage<LengthUnit>::set_data( 
   nonlinfit::plane::DisjointData< Num,LengthUnit,ChunkSize >& target,
   const dStorm::Image< PixelType, 2 >& image,
   const Spot& center,
   const Transform& transform ) const
{
    set_generic_data(target, center);

    typedef nonlinfit::plane::DisjointData< Num,LengthUnit,ChunkSize > Target;
    Statistics<2> rv;
    rv.peak_intensity = rv.integral = 0 * camera::ad_count;
    rv.peak_pixel_area = quantity< si::area >( target.pixel_size );

    const typename Bounds::Scalar one_pixel = 1 * camera::pixel;

    ImageSize upper_bound;
    upper_bound.x() = image.sizes().x() - one_pixel;
    upper_bound.y() = image.sizes().y() - one_pixel;
    Bounds cut_region = this->cut_region(center, upper_bound);
    typedef Bounds::Scalar Pixel;

    const int orig_width = (cut_region(0,1) - cut_region(0,0)).value() + 1;
    switch ( ChunkSize - orig_width ) {
        case 2: cut_region(0,1) += 1 * camera::pixel;
        case 1: cut_region(0,0) -= 1 * camera::pixel;
        case 0: break;
        case -1: cut_region(0,1) -= 1 * camera::pixel;
    }
    if ( cut_region(0,0) < 0 * camera::pixel ) {
        cut_region(0,1) += - cut_region(0,0);
        cut_region(0,0) = 0 * camera::pixel;
    }
    if ( cut_region(0,1) > upper_bound.x() ) {
        cut_region(0,0) -= cut_region(0,1) - upper_bound.x();
        cut_region(0,1) = upper_bound.x();
    }
    if ( (cut_region(0,1) - cut_region(0,0)).value() + 1 != ChunkSize ) {
        assert(false);
        throw std::runtime_error("Could not find appropriate fit window width");
    }

    rv.pixel_count = (cut_region(0,1) - cut_region(0,0)).value() + 1;
    rv.pixel_count *= (cut_region(1,1) - cut_region(1,0)).value() + 1;

    std::vector< PixelType > pixels;
    pixels.reserve( rv.pixel_count );

    const traits::ScaledProjection& scale = 
        dynamic_cast< const traits::ScaledProjection& >( this->optics.projection() );
    for (int i = 0; i < 2; ++i) {
        target.min[i] = quantity<LengthUnit>( scale.length_in_sample_space( i, cut_region(i,0) ) );
        target.max[i] = quantity<LengthUnit>( scale.length_in_sample_space( i, cut_region(i,1) ) );
    }

    for (Pixel x = cut_region(0,0); x <= cut_region(0,1); x += one_pixel) {
        int dx = (x - cut_region(0,0)).value();
        target.xs[dx] = 
            quantity<LengthUnit>
                (scale.length_in_sample_space( 0, x )).value();
    }
    DEBUG("X coordinates are " << target.xs.transpose());
    target.data.clear();
    target.data.reserve( (cut_region(1,1) - cut_region(1,0)).value() + 1 );
    for (Pixel y = cut_region(1,0); y <= cut_region(1,1); y += one_pixel) {
        target.data.push_back( typename Target::DataRow() );
        typename Target::DataRow& current = target.data.back();
        DEBUG("Cutting line " << y << " from " << cut_region(0,0) << " to " << 
              cut_region(0,1) << " from image of size " << image.sizes().transpose() );

        quantity<LengthUnit> sample_y( scale.length_in_sample_space( 1, y ) );
        current.inputs(0,0) = sample_y.value();
        for (Pixel x = cut_region(0,0); x <= cut_region(0,1); x += one_pixel) {
            Num value = transform( image(x,y) );
            current.output[(x - cut_region(0,0)).value()] = value;
            rv.integral += value * camera::ad_count;
            pixels.push_back( value );
            if ( value * camera::ad_count > rv.peak_intensity ) {
                rv.peak_intensity = value * camera::ad_count;
                rv.highest_pixel.x() = Spot::Scalar(
                    quantity<LengthUnit>::from_value(
                        target.xs[(x - cut_region(0,0)).value()]) );
                rv.highest_pixel.y() = Spot::Scalar(
                    quantity<LengthUnit>::from_value(current.inputs(0,0)) );
            }
        }
        DEBUG("Y coordinate is " << current.inputs(0,0) << " and values are " << current.output.transpose());
    }

    if ( ! pixels.empty() ) {
        typename std::vector<PixelType>::iterator qp = pixels.begin() + pixels.size() / 4;
        std::nth_element( pixels.begin(), qp, pixels.end());
        rv.quarter_percentile_pixel = *qp * camera::ad_count;
    }
    return rv;
}

template <typename LengthUnit>
template <typename PixelType, typename Num, int ChunkSize, typename Transform>
Statistics<2>
TransformedImage<LengthUnit>::set_data( 
   nonlinfit::plane::JointData< Num, LengthUnit, ChunkSize >& target,
   const dStorm::Image< PixelType, 2 >& image,
   const Spot& center,
   const Transform& transform )  const
{
    typedef nonlinfit::plane::JointData<Num,LengthUnit,ChunkSize > Data;
    typedef typename Data::DataRow DataRow;
    set_generic_data(target, center);
    Statistics<2> rv;
    rv.peak_intensity = rv.integral = 0 * camera::ad_count;
    rv.peak_pixel_area = quantity< si::area >( target.pixel_size );

    DEBUG("Cutting outer box " << cut_region.row(0) << " in x and in y " << cut_region.row(1));
    traits::Projection::ROI points = 
        optics.projection().
            cut_region_of_interest( traits::Projection::ROISpecification(center.head<2>(), max_distance.head<2>()) );
    target.clear();
    target.reserve( points.size() );
    std::vector< PixelType > pixels;
    pixels.reserve( points.size() );
    typename Data::data_point_iterator o = target.point_back_inserter();
    for ( traits::Projection::ROI::const_iterator i = points.begin(); i != points.end(); ++i )
    {
        if ( ! image.contains( i->image_position ) )
            continue;

        for (int d = 0; d < 2; ++d) {
            target.min[d] = std::min( quantity<LengthUnit>(i->sample_position[d]), target.min[d] );
            target.max[d] = std::max( quantity<LengthUnit>(i->sample_position[d]), target.max[d] );
        }

        Spot sample = i->sample_position;
        const Num value = transform( image( i->image_position ) );
        pixels.push_back( value );
        rv.integral += value * boost::units::camera::ad_count;
        if ( value * camera::ad_count >= rv.peak_intensity ) {
            rv.peak_intensity = value * camera::ad_count;
            rv.highest_pixel = sample;
        }

        *o++ = typename Data::DataPoint( sample, value );
    }

    o.pad_last_chunk();
    rv.pixel_count = pixels.size();

    if ( ! pixels.empty() ) {
        typename std::vector<PixelType>::iterator qp = pixels.begin() + pixels.size() / 4;
        std::nth_element( pixels.begin(), qp, pixels.end());
        rv.quarter_percentile_pixel = *qp * camera::ad_count;
    }
    return rv;
}

}
}

#endif
