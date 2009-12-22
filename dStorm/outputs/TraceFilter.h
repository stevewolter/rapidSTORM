#ifndef DSTORM_TRACE_FILTER
#define DSTORM_TRACE_FILTER

#include <dStorm/output/Trace.h>
#include <dStorm/output/FilterBuilder.h>
#include <simparm/NumericEntry.hh>
#include <dStorm/output/ResultRepeater.h>
#include <simparm/Structure.hh>

namespace dStorm {
namespace outputs {

class TraceCountFilter : public output::OutputObject,
                         public simparm::Node::Callback
{
  private:
    data_cpp::Vector<Localization> localizations;
    int minCount;
    bool disassemble;
    std::auto_ptr< output::Output > output;
    ost::Mutex mutex;

    simparm::BoolEntry selectSpecific;
    simparm::UnsignedLongEntry whichSpecific;
    output::ResultRepeater *result_repeater;
    int processed_locs;

    int count_localizations_in( const Localization &l );
    void processLocalization( const Localization& l);
    void operator()(Node& src, Cause c, Node*);

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

    AdditionalData announceStormSize(const Announcement &a) ;

    void propagate_signal(ProgressSignal s);

    Result receiveLocalizations(const EngineResult& e);
};

class TraceCountFilter::_Config 
: public simparm::Object
{
  public:
    simparm::UnsignedLongEntry min_count;
    simparm::BoolEntry disassemble;
    simparm::BoolEntry selectSpecific;
    simparm::UnsignedLongEntry whichSpecific;

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
        void operator()(Node& src, Cause c, Node*) {
            if ( &src == &condition.value && c == ValueChanged ) 
                toShow.viewable = condition();
        }
      public:
        WhichSpecificShower(simparm::BoolEntry& condition, 
                            simparm::Object& toShow)
            : condition(condition), toShow(toShow)
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
