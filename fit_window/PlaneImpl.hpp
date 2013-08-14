#ifndef DSTORM_GUF_DATAPLANE_IMPL_HPP
#define DSTORM_GUF_DATAPLANE_IMPL_HPP

#include "PlaneImpl.h"
#include "Optics.h"
#include "fit_position_out_of_range.h"
#include "Centroid.h"

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
    typedef typename Data::DataRow DataRow;

    const float background_part = 0.25;

    this->pixel_size = optics.pixel_size(position);
    data.pixel_size = quantity<si::area>( this->pixel_size ).value() * 1E12;

    /* Initialize iteratively computed statistics */
    for (int i = 0; i < 2; ++i) {
        data.min[i] = quantity<si::length>(position[i]).value() * 1E6;
    }
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

        Spot sample = i->sample_position;
        const typename Data::value_type::Intensity value = 
            std::max( 0.0, optics.absolute_in_photons( image( i->image_position ) * camera::ad_count ) );
        pixels.push_back( value );
        this->integral += value;
        if ( value >= this->peak_intensity ) {
            this->peak_intensity = value;
            this->highest_pixel = sample;
        }

        Eigen::Vector2d sample_in_mum;
        for (int i = 0; i < 2; ++i) {
            sample_in_mum[i] = quantity<si::length>(sample[i]).value() * 1E6;
        }
        *o++ = typename Data::value_type( sample_in_mum, value );
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
                double offset = i->position(dim) - quantity<si::length>(this->highest_pixel[dim]).value() * 1E6;
                double intensity_above_background = 
                    std::max(0.0, double(i->value() - this->background_estimate));
                acc[dim]( offset, weight = intensity_above_background );
            }
        }
        for (int dim = 0; dim < 2; ++dim)
            this->standard_deviation[dim] = quantity<si::length>( 
                sqrt( weighted_variance(acc[dim]) ) * 1E-6 * si::meter );
    }
}

template <typename Tag>
std::auto_ptr<Centroid> PlaneImpl<Tag>::_residue_centroid() const
{ 
    std::auto_ptr<Centroid> rv( new Centroid( data.min, data.max ) );  
    for ( typename Tag::Data::const_iterator i= data.begin(), e = data.end(); i != e; ++i ) {
        rv->add( i->position().template cast<double>(), i->residue() );
    }

    return rv;
}

}
}

#endif
