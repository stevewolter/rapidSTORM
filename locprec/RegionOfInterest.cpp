#include <simparm/BoostUnits.hh>

#include "RegionOfInterest.h"
#include <boost/multi_array.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <Eigen/Core>

#include <dStorm/output/FilterBuilder.h>
#include <dStorm/output/Filter.h>
#include <simparm/Entry.hh>
#include <dStorm/Engine.h>
#include <simparm/Structure.hh>
#include <dStorm/UnitEntries.h>
#include <simparm/Entry_Impl.hh>

namespace simparm {
template class Entry< dStorm::samplepos::Scalar>;
}

namespace locprec {

class ROIFilter : public dStorm::output::Filter
{
  private:
    dStorm::samplepos offset, from, to;

  public:
    class Config;

    ROIFilter(const Config& config,
                     std::auto_ptr<dStorm::output::Output> output);
    ~ROIFilter() {}

    AdditionalData announceStormSize(const Announcement &a);

    void receiveLocalizations(const EngineResult& e);
};

class ROIFilter::Config 
{
  public:
    simparm::Entry< dStorm::samplepos::Scalar >
        left, right, top, bottom;

    Config();

    static std::string get_name() { return "ROIFilter"; }
    static std::string get_description() { return "Select region of interest"; }
    static simparm::Object::UserLevel get_user_level() { return simparm::Object::Beginner; }

    void attach_ui( simparm::Node& at ) {
        left.attach_ui(at); 
        right.attach_ui(at);
        top.attach_ui(at);
        bottom.attach_ui(at);
    }

    bool determine_output_capabilities( dStorm::output::Capabilities& )
        { return true; }
};

ROIFilter::ROIFilter(
    const Config& config,
    std::auto_ptr<dStorm::output::Output> output
) 
: Filter(output)
{
    from.x() = config.left();
    from.y() = config.top();
    to.x() = config.right();
    to.y() = config.bottom();

    for ( int i = 0; i < from.rows(); ++i )  {
        offset[i] = floor(from[i]);
    }
}

dStorm::output::Output::AdditionalData
ROIFilter::announceStormSize(const Announcement &a)
{
    Announcement my_announcement = a;
    for ( int i = 0; i < to.rows(); ++i ) {
        my_announcement.position().range()[i].first = from[i];
        my_announcement.position().range()[i].second = to[i];
    }
    return Filter::announceStormSize(my_announcement);
}

void ROIFilter::receiveLocalizations(const EngineResult& e) {
    EngineResult oe = e;
    int back = 0;
    for ( EngineResult::const_iterator i = e.begin(); i != e.end(); ++i)
        if ( (i->position().array() >= from.array()).all() && (i->position().array() <= to.array()).all() )
            oe[++back] = *i;

    oe.resize(back);
    Filter::receiveLocalizations(oe);
}

ROIFilter::Config::Config() 
: left("LeftROIBorder", "Left border"),
  right("RightROIBorder", "Right border"),
  top("TopROIBorder", "Top border"),
  bottom("BottomROIBorder", "Bottom border")
{
}

std::auto_ptr< dStorm::output::OutputSource > make_roi_filter_source() {
    return std::auto_ptr< dStorm::output::OutputSource >(
        new dStorm::output::FilterBuilder< ROIFilter::Config, ROIFilter >()
    );
}

}
