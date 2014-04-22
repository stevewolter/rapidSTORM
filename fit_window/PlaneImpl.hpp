#ifndef DSTORM_GUF_DATAPLANE_IMPL_HPP
#define DSTORM_GUF_DATAPLANE_IMPL_HPP

#include "fit_window/PlaneImpl.h"
#include "fit_window/Optics.h"
#include "fit_window/fit_position_out_of_range.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/weighted_variance.hpp>
#include <boost/bind/bind.hpp>
#include <boost/units/io.hpp>

#include <nonlinfit/plane/fwd.h>

namespace dStorm {
namespace fit_window {

using namespace boost::accumulators;

template <typename Num, int ChunkSize>
inline boost::optional< quantity<camera::length,int> >
row_width( const nonlinfit::plane::DisjointData<Num,ChunkSize>& )
{
    return ChunkSize * camera::pixel;
}

template <typename Num, int ChunkSize>
inline boost::optional< quantity<camera::length,int> >
row_width( const nonlinfit::plane::JointData<Num,ChunkSize>& )
{
    return boost::optional< quantity<camera::length,int> >();
}

template <typename Tag>
PlaneImpl<Tag>::PlaneImpl( 
    const Optics& optics,
    const dStorm::engine::Image2D& image,
    const Spot& position
) : Plane(optics )
{
    typedef typename Tag::Data Data;

    const float background_part = 0.25;

    this->pixel_size = quantity<si::area>(optics.pixel_size(position)).value() * 1E12;
    data.pixel_size = this->pixel_size;

    /* Initialize iteratively computed statistics */
    data.min = position;
    data.max = data.min;
    this->peak_intensity = this->integral = 0;

    traits::Projection::ROI points 
        = optics.get_region_of_interest( position, row_width(data) );

    if ( points.size() <= 0 ) 
        throw fit_position_out_of_range();

    data.clear();
    data.reserve( points.size() );
    /* Keep a list of the inserted pixel values for later statistical usage */
    std::vector<float> pixels;
    pixels.reserve( points.size() );
    std::back_insert_iterator<Data> o = std::back_inserter( data );
    for ( traits::Projection::ROI::const_iterator i = points.begin(); i != points.end(); ++i )
    {
        assert( image.contains( i->image_position ) );

        for (int d = 0; d < 2; ++d) {
            double pos = quantity<si::length>(i->sample_position[d]).value() * 1E6;
            data.min[d] = std::min( pos, data.min[d] );
            data.max[d] = std::max( pos, data.max[d] );
        }

        Spot sample;
        for (int d = 0; d < 2; ++d) {
            sample[d] = quantity<si::length>(i->sample_position[d]).value() * 1E6;
        }
        const typename Data::value_type::Intensity value = 
            std::max( 0.0, optics.absolute_in_photons( image( i->image_position ) * camera::ad_count ) );
        pixels.push_back( value );
        this->integral += value;
        if ( value >= this->peak_intensity ) {
            this->peak_intensity = value;
            this->highest_pixel = sample;
        }

        *o++ = typename Data::value_type( sample, value );
    }

    data.pad_last_chunk();
    this->pixel_count = pixels.size();

    if ( ! pixels.empty() ) {
        std::vector<float>::iterator qp = pixels.begin() + pixels.size() * background_part;
        std::nth_element( pixels.begin(), qp, pixels.end());
        this->background_estimate = *qp;

        accumulator_set<double, stats<tag::weighted_variance(lazy)>, double> acc[2];

        for ( typename Data::const_iterator i = data.begin(); i != data.end(); ++i )
        {
            for (int dim = 0; dim < 2; ++dim) {
                double offset = i->position(dim) - this->highest_pixel[dim];
                double intensity_above_background = 
                    std::max(0.0, double(i->value() - this->background_estimate));
                acc[dim]( offset, weight = intensity_above_background );
            }
        }
        for (int dim = 0; dim < 2; ++dim)
            this->standard_deviation[dim] = sqrt( weighted_variance(acc[dim]) );
    }
}

template <typename Tag>
Spot PlaneImpl<Tag>::_residue_centroid() const
{ 
    Spot location;
    double highest_residue = std::numeric_limits<double>::min();
    for ( typename Tag::Data::const_iterator i= data.begin(), e = data.end(); i != e; ++i ) {
        if (highest_residue < i->residue()) {
            highest_residue = i->residue();
            location = i->position().template cast<double>();
        }
    }

    return location;
}

}
}

#endif
