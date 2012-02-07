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

using namespace nonlinfit::plane;

namespace dStorm {
namespace guf {

template <typename Data>
const Statistics<2> InputPlane::set_data( Data& d, const Image& i, const Spot& s ) const
{
    mle_converter t(dark_current, photon_response_);
    return transformation.set_data( d, i, s, t );
}

template <typename Num, typename LengthUnit, int ChunkSize>
Centroid residue_centroid( const JointData<Num,LengthUnit,ChunkSize>& d ) {
    Centroid rv( d.min.template head<2>(), d.max.template head<2>() );

    for ( typename JointData<Num,LengthUnit,ChunkSize>::const_iterator i= d.begin(), e = d.end(); i != e; ++i ) {
        for (int j = 0; j < i->residues.rows(); ++j) {
	    rv.add( boost::units::from_value< LengthUnit >(i->inputs.row(j).template cast<double>())
                , i->residues[j] );
        }
    }

    return rv;
}

template <typename Num, typename LengthUnit, int ChunkSize>
Centroid
residue_centroid( const DisjointData< Num,LengthUnit,ChunkSize >& d ) {
    Centroid rv( d.min.template head<2>(), d.max.template head<2>() );

    for ( typename DisjointData< Num,LengthUnit,ChunkSize >::const_iterator i= d.data.begin(), e = d.data.end(); i != e; ++i ) {
        for (int j = 0; j < i->residues.rows(); ++j) {
            typename Centroid::Spot spot;
            spot.x() = Centroid::Coordinate::from_value(d.xs[j]);
            spot.y() = Centroid::Coordinate::from_value(i->inputs(0,0));
	    rv.add( spot, i->residues[j] );
        }
    }

    return rv;
}


template <typename Tag>
struct TaggedDataPlane
: public DataPlane {
    typename Tag::Data data;
    Statistics<2> image_stats;
    virtual const void* get_data() const { return &data; }
    virtual std::auto_ptr<Centroid> _residue_centroid() const 
        { return std::auto_ptr<Centroid>( new Centroid(guf::residue_centroid( data )) );  }
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

struct InputPlane::instantiate_appropriate_data {
    typedef void result_type;
    
    template <int ChunkSize, typename Num, typename P1, typename P2>
    bool is_appropriate( 
        nonlinfit::plane::Disjoint<Num,ChunkSize,P1,P2> t,
        bool can_do_disjoint,
        bool use_floats,
        int width
    ) { 
        const int slack = boost::is_same<Num,float>::value ? 1 : 0;
        return can_do_disjoint && 
            (ChunkSize+slack >= width)  &&
            (ChunkSize <= width + 2) &&
            (use_floats == boost::is_same<Num,float>::value);
    }

    template <int ChunkSize, typename Num, typename P1, typename P2>
    bool is_appropriate( 
        nonlinfit::plane::Joint<Num,ChunkSize,P1,P2>, bool, 
        bool use_floats,  int
    ) { 
        return (use_floats == boost::is_same<Num,float>::value); 
    }

    template <typename Tag> 
    void operator()( 
            std::auto_ptr< DataPlane >& result,
            Tag t,
            const InputPlane& input,
            const dStorm::engine::Image2D& image,
            const Spot& position,
            int window_width )
    {
        if ( result.get() ) return;
        if ( is_appropriate( t, input.can_do_disjoint_fitting, input.use_floats, window_width ) ) {
            result.reset( new TaggedDataPlane<Tag>( input, image, position ) );
        }
    }
};

std::auto_ptr< DataPlane >
InputPlane::set_image( 
    const dStorm::engine::Image2D& image,
    const Spot& position
) const {
    std::auto_ptr< DataPlane > rv;
    instantiate_appropriate_data creator;
    boost::mpl::for_each< evaluation_tags >( 
        boost::bind( creator, boost::ref(rv), _1,
                     boost::cref(*this), boost::cref(image),
                     boost::cref(position),
                     get_fit_window_width(position) ) );

    return rv;
}

fit_position_out_of_range::fit_position_out_of_range()
: std::runtime_error("Selected fit position not in all layers of image") {}

guf::Spot
InputPlane::make_max_distance( const dStorm::engine::JobInfo& info, int plane_number )
{
    Spot rv;
    for (int i = 0; i < 2; ++i)
        rv[i] = info.mask_size_in_si(i,plane_number);
    return rv;
}

InputPlane::InputPlane( const Config& c, const dStorm::engine::JobInfo& info, int plane_number )
: im_size( info.traits.size.head<2>().array() - 1 * camera::pixel ),
  transformation(make_max_distance(info,plane_number), info.traits.plane(plane_number)),
  can_do_disjoint_fitting( c.allow_disjoint() && 
    dynamic_cast< const traits::ScaledProjection* >( info.traits.plane(plane_number).projection().get() ) ),
  use_floats( !c.double_computation() )
{
    const traits::Optics<2>& t = info.traits.plane(plane_number);
    photon_response_ = t.photon_response.get_value_or( 1 * camera::ad_count );
    dark_current = quantity<camera::intensity,int>
        (t.dark_current.get_value_or(0*camera::ad_count));
    if ( t.background_stddev.is_initialized() && t.photon_response.is_initialized() )
        background_noise_variance_ = pow( *t.background_stddev / *t.photon_response, 2 );
    has_precision 
        = t.photon_response.is_initialized() &&
          background_noise_variance_.is_initialized();
}

int InputPlane::get_fit_window_width(const Spot& at) const
{
    typename TransformedImage::Bounds bounds = 
        transformation.cut_region( at, im_size );
    return (bounds(0,1) - bounds(0,0)).value() + 1;
}

}
}

#endif
