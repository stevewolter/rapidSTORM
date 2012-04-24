#include "DataExtractor.h"
#include "DataPlane.h"
#include "EvaluationTags.h"
#include "Centroid.h"
#include "fit_position_out_of_range.h"
#include <boost/bind/bind.hpp>
#include <nonlinfit/plane/DisjointData.hpp>
#include <nonlinfit/plane/JointData.hpp>
#include "Optics.hpp"

namespace dStorm {
namespace guf {

fit_position_out_of_range::fit_position_out_of_range()
: std::runtime_error("Selected fit position not in all layers of image") {}

template <typename Tag>
struct DataPlaneImpl
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
    DataPlaneImpl( 
        const Optics& input,
        const dStorm::engine::Image2D& image,
        const guf::Spot& position )
    : DataPlane(input, nonlinfit::index_of< evaluation_tags, Tag >::value )
    {
        image_stats = input.get_data_from_image( data, image, position );
        if ( image_stats.pixel_count <= 0 ) 
            throw fit_position_out_of_range();
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

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
