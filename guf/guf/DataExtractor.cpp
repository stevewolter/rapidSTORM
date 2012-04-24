#include <algorithm>

#include <nonlinfit/plane/DisjointData.hpp>
#include <nonlinfit/plane/JointData.hpp>

#include "DataExtractor.h"
#include "DataPlane.h"
#include "EvaluationTags.h"
#include "fit_position_out_of_range.h"
#include "Optics.hpp"
#include "DataPlaneImpl.hpp"

namespace dStorm {
namespace guf {

using namespace boost::units;
using namespace boost::accumulators;

fit_position_out_of_range::fit_position_out_of_range()
: std::runtime_error("Selected fit position not in all layers of image") {}

template <typename Tag>
struct DataExtractorImpl : public DataExtractor {
    const Optics& input;
    std::auto_ptr<DataPlane> extract_data_( const engine::Image2D& image, const Spot& position ) const {
        return std::auto_ptr<DataPlane>( new DataPlaneImpl<Tag>( input, image, position ) );
    }
public:
    DataExtractorImpl( const Optics& input ) : input(input) {}
};

template <typename EvaluationSchedule>
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
        boost::bind( instantiator<EvaluationSchedule>(), boost::ref(*this), _1 ) );
}

DataExtractorTable::DataExtractorTable( const Optics& input) 
: optics(input) 
{
    boost::mpl::for_each< evaluation_tags >(
        boost::bind( instantiator<evaluation_tags>(), boost::ref(*this), _1 ) );
}

}
}
