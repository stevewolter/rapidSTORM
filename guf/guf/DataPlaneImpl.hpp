#ifndef DSTORM_GUF_DATAPLANE_IMPL_HPP
#define DSTORM_GUF_DATAPLANE_IMPL_HPP

#include "DataPlaneImpl.h"
#include "Optics.hpp"
#include "fit_position_out_of_range.h"
#include "Centroid.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/weighted_variance.hpp>
#include <boost/bind/bind.hpp>
#include <boost/units/io.hpp>

namespace dStorm {
namespace guf {

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

template <typename Tag>
DataPlaneImpl<Tag>::DataPlaneImpl( 
    const Optics& optics,
    const dStorm::engine::Image2D& image,
    const guf::Spot& position
) : DataPlane(optics )
{
    typedef typename Tag::Data Data;
    typedef typename Data::data_point::Length Length;
    typedef quantity< typename Length::unit_type > DoubleLength;
    typedef typename Data::DataRow DataRow;

    const float background_part = 0.25;

    this->pixel_size = optics.pixel_size(position);
    data.pixel_size = quantity<typename Data::AreaUnit>( this->pixel_size );

    /* Initialize iteratively computed statistics */
    data.min = position.template cast< DoubleLength >();
    data.max = position.template cast< DoubleLength >();
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
            data.min[d] = std::min( DoubleLength(i->sample_position[d]), data.min[d] );
            data.max[d] = std::max( DoubleLength(i->sample_position[d]), data.max[d] );
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
                Length offset = i->position(dim) - Length(this->highest_pixel[dim]);
                double intensity_above_background = 
                    std::max(0.0, double(i->value() - this->background_estimate));
                acc[dim]( offset.value(),
                    weight = intensity_above_background );
            }
        }
        for (int dim = 0; dim < 2; ++dim)
            this->standard_deviation[dim] = quantity<si::length>( 
                Length::from_value(sqrt( weighted_variance(acc[dim]) ) ) );
    }
}

template <typename Tag>
std::auto_ptr<Centroid> DataPlaneImpl<Tag>::_residue_centroid() const
{ 
    std::auto_ptr<Centroid> rv( new Centroid( data.min, data.max ) );  
    for ( typename Tag::Data::const_iterator i= data.begin(), e = data.end(); i != e; ++i )
        rv->add( i->position().template cast< Centroid::Coordinate >(), i->residue() );

    return rv;
}

}
}

#endif
