#ifndef DSTORM_TRACE_FILTER
#define DSTORM_TRACE_FILTER

#include "../output/FilterBuilder.h"
#include <simparm/Entry.hh>
#include "../Engine_decl.h"
#include <simparm/Structure.hh>
#include <vector>

namespace dStorm {
namespace outputs {

class TraceCountFilter : public output::OutputObject,
                         public simparm::Node::Callback
{
  private:
    EngineResult localizations;
    int minCount;
    bool disassemble;
    std::auto_ptr< output::Output > output;

    simparm::BoolEntry selectSpecific;
    simparm::Entry<unsigned long> whichSpecific;
    Engine *engine;
    int processed_locs;

    int count_localizations_in( const Localization &l );
    void processLocalization( const Localization& l);
    void operator()(const simparm::Event&);

    /** As of yet, the copy constructor is not implemented. */
    TraceCountFilter(const TraceCountFilter&);
    TraceCountFilter& operator=(const TraceCountFilter&);

    class _Config;

  public:
    class Config;
    typedef output::FilterBuilder<TraceCountFilter> Source;

    TraceCountFilter(const Config& config,
                     std::auto_ptr<output::Output> output);
    ~TraceCountFilter() {}
    TraceCountFilter* clone() const 
        { throw std::runtime_error("No TraceCountFilter::clone"); }

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames) 
        { output->check_for_duplicate_filenames(present_filenames); }

    AdditionalData announceStormSize(const Announcement &a) ;
    RunRequirements announce_run(const RunAnnouncement& a) 
        { processed_locs = 0; return output->announce_run(a); }

    void store_results();

    void receiveLocalizations(const EngineResult& e);
};

class TraceCountFilter::_Config 
: public simparm::Object
{
  public:
    simparm::Entry<unsigned long> min_count;
    simparm::Entry<bool> disassemble;
    simparm::Entry<bool> selectSpecific;
    simparm::Entry<unsigned long> whichSpecific;

    _Config();

    void registerNamedEntries() {
        push_back(min_count); 
        push_back(disassemble);
        push_back(selectSpecific);
        push_back(whichSpecific);
    }

    virtual void set_source_capabilities( output::Capabilities ) = 0;

    bool determine_output_capabilities( output::Capabilities& cap ) {
        if ( ! cap.test( output::Capabilities::ClustersWithSources ) )
            return false;
        return true;
    }
};

class TraceCountFilter::Config 
: public simparm::VirtualStructure<_Config>
{
  private:
    class WhichSpecificShower : public simparm::Node::Callback {
        simparm::BoolEntry &condition;
        simparm::Object &toShow;
        void operator()(const simparm::Event&) {
            toShow.viewable = condition();
        }
      public:
        WhichSpecificShower(simparm::BoolEntry& condition, 
                            simparm::Object& toShow)
            : simparm::Node::Callback( simparm::Event::ValueChanged ),
            condition(condition), toShow(toShow)
            { receive_changes_from(condition.value); }
    };
    WhichSpecificShower shower;

  public:
    Config() 
        : shower(selectSpecific, whichSpecific) {}
    Config(const Config &c)
        : simparm::VirtualStructure<_Config>(c),
          shower(selectSpecific, whichSpecific) {}
    Config* clone() const = 0;
};

}
}

#endif
