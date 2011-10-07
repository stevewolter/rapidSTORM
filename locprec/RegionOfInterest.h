#ifndef LOCPREC_ROI_FILTER_H
#define LOCPREC_ROI_FILTER_H

#include <simparm/BoostUnits.hh>
#include <dStorm/output/FilterBuilder.h>
#include <simparm/Entry.hh>
#include <dStorm/Engine.h>
#include <simparm/Structure.hh>
#include <dStorm/UnitEntries.h>
#include <simparm/Entry_Impl.hh>

namespace locprec {

class ROIFilter : public dStorm::output::OutputObject
{
  private:
    std::auto_ptr< dStorm::output::Output > output;

    dStorm::samplepos offset, from, to;
    class _Config;

  public:
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::output::FilterBuilder<ROIFilter> Source;

    ROIFilter(const Config& config,
                     std::auto_ptr<dStorm::output::Output> output);
    ~ROIFilter() {}
    ROIFilter* clone() const 
        { throw std::runtime_error("No ROIFilter::clone"); }

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames) 
        { output->check_for_duplicate_filenames(present_filenames); }

    AdditionalData announceStormSize(const Announcement &a);
    RunRequirements announce_run(const RunAnnouncement& a) 
        { return output->announce_run(a); }

    void propagate_signal(ProgressSignal s) 
        { output->propagate_signal(s); }

    Result receiveLocalizations(const EngineResult& e);
};

class ROIFilter::_Config 
: public simparm::Object
{
  public:
    simparm::Entry< dStorm::samplepos::Scalar >
        left, right, top, bottom;

    _Config();

    void registerNamedEntries() {
        push_back(left); 
        push_back(right);
        push_back(top);
        push_back(bottom);
    }

    bool determine_output_capabilities( dStorm::output::Capabilities& )
        { return true; }
};

}

#endif
