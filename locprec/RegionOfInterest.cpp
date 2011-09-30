#include "RegionOfInterest.h"
#include <boost/multi_array.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <Eigen/Array>

namespace simparm {
template class NumericEntry< dStorm::samplepos::Scalar>;
}

namespace locprec {

ROIFilter::ROIFilter(
    const Config& config,
    std::auto_ptr<dStorm::output::Output> output
) 
: OutputObject("ROIFilter", "ROI Filter"), output(output)
{
    from.x() = config.left();
    from.y() = config.top();
    to.x() = config.right();
    to.y() = config.bottom();

    for ( int i = 0; i < from.rows(); ++i )  {
        offset[i] = floor(from[i]);
    }

    push_back(this->output->getNode());
}

dStorm::output::Output::AdditionalData
ROIFilter::announceStormSize(const Announcement &a)
{
    Announcement my_announcement = a;
    for ( int i = 0; i < to.rows(); ++i ) {
        my_announcement.position().range()[i].first = from[i];
        my_announcement.position().range()[i].second = to[i];
    }
    return output->announceStormSize(my_announcement);
}

dStorm::output::Output::Result
ROIFilter::receiveLocalizations(const EngineResult& e) {
    EngineResult oe = e;
    int back = 0;
    for ( EngineResult::const_iterator i = e.begin(); i != e.end(); ++i)
        if ( (i->position().cwise() >= from).all() && (i->position().cwise() <= to).all() )
            oe[++back] = *i;

    oe.resize(back);
    return output->receiveLocalizations(oe);
}

ROIFilter::_Config::_Config() 
: simparm::Object("ROIFilter", "Select region of interest"),
  left("LeftROIBorder", "Left border"),
  right("RightROIBorder", "Right border"),
  top("TopROIBorder", "Top border"),
  bottom("BottomROIBorder", "Bottom border")
{
}

}
