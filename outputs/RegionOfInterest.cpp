#include <simparm/BoostUnits.h>

#include "RegionOfInterest.h"
#include <boost/multi_array.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <boost/bind/bind.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <Eigen/Core>

#include <dStorm/output/FilterBuilder.h>
#include <dStorm/output/Filter.h>
#include <simparm/Entry.h>
#include <dStorm/Engine.h>
#include <dStorm/units/microlength.h>

namespace dStorm {
namespace outputs {

class ROIFilter : public dStorm::output::Filter
{
  private:
    dStorm::samplepos offset, from, to;
    bool not_within_ROI( const dStorm::Localization& ) const;
    bool is_3d;

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
    simparm::Entry< Eigen::Matrix< boost::units::quantity< boost::units::si::microlength, float >, 3, 1, Eigen::DontAlign > >
        lower, upper;

    Config();

    static std::string get_name() { return "ROIFilter"; }
    static std::string get_description() { return "Select region of interest"; }
    static simparm::UserLevel get_user_level() { return simparm::Beginner; }

    void attach_ui( simparm::NodeHandle at ) {
        lower.attach_ui(at); 
        upper.attach_ui(at);
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
    from = config.lower().cast< samplepos::Scalar >();
    to = config.upper().cast< samplepos::Scalar >();

    for ( int i = 0; i < from.rows(); ++i )  {
        offset[i] = floor(from[i]);
    }
}

dStorm::output::Output::AdditionalData
ROIFilter::announceStormSize(const Announcement &a)
{
    is_3d = a.position().is_given[2];
    Announcement my_announcement = a;
    for ( int i = 0; i < to.rows(); ++i ) {
        my_announcement.position().range()[i].first = from[i];
        my_announcement.position().range()[i].second = to[i];
    }
    return Filter::announceStormSize(my_announcement);
}

bool ROIFilter::not_within_ROI( const dStorm::Localization& l ) const {
    int n = (is_3d) ? 3 : 2;
    return !( (l.position().array() >= from.array()).head(n).all() && (l.position().array() <= to.array()).head(n).all() );
}

void ROIFilter::receiveLocalizations(const EngineResult& e) {
    EngineResult oe = e;
    boost::range::remove_erase_if( oe, boost::bind( &ROIFilter::not_within_ROI, this, _1 ) );
    Filter::receiveLocalizations(oe);
}

ROIFilter::Config::Config() 
: lower("LowerROIBorder", "Lower ROI border"),
  upper("UpperROIBorder", "Upper ROI border")
{
}

std::auto_ptr< dStorm::output::OutputSource > make_roi_filter_source() {
    return std::auto_ptr< dStorm::output::OutputSource >(
        new dStorm::output::FilterBuilder< ROIFilter::Config, ROIFilter >()
    );
}

}
}
