#include "TraceFilter.h"
#include <dStorm/Engine.h>
#include <numeric>
#include <boost/bind/bind.hpp>
#include <boost/iterator/transform_iterator.hpp>

namespace dStorm {
using namespace output;
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
    dStorm::Engine *engine;
    int processed_locs;

    int count_localizations_in( const Localization &l );
    void processLocalization( const Localization& l);
    void operator()(const simparm::Event&);

    /** As of yet, the copy constructor is not implemented. */
    TraceCountFilter(const TraceCountFilter&);
    TraceCountFilter& operator=(const TraceCountFilter&);

    void store_results_( bool success );

  public:
    TraceCountFilter(const TraceCountConfig& config,
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

    void receiveLocalizations(const EngineResult& e);
};

TraceCountFilter::TraceCountFilter(
    const TraceCountConfig& c,
    std::auto_ptr<output::Output> output
) 
: output::OutputObject("TraceFilter", "Trace filter"),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  minCount(c.min_count()), 
    disassemble(c.disassemble()), output(output),
    selectSpecific(c.selectSpecific),
    whichSpecific(c.whichSpecific),
    engine(NULL)
{  
    receive_changes_from( this->selectSpecific.value );
    receive_changes_from( this->whichSpecific.value );
    push_back(this->selectSpecific);
    push_back(this->whichSpecific);
    push_back(this->output->getNode()); 
}

int TraceCountFilter::count_localizations_in
    ( const Localization &l ) 
{
    int accum = 1;
    if ( l.children.is_initialized() )
        for ( Localization::Children::const_iterator i = l.children->begin(); i != l.children->end(); ++i )
            accum += count_localizations_in(*i);
    return accum;
}

void TraceCountFilter::processLocalization( const Localization& l)
{
    if ( count_localizations_in( l ) >= minCount ) {
        if (disassemble && l.children.is_initialized() ) {
            std::copy( l.children->begin(), l.children->end(), std::back_inserter( localizations ) );
        } else {
            localizations.push_back( l );
        }
    }
}

void TraceCountFilter::operator()(const simparm::Event& e) {
    if ( &e.source == &selectSpecific.value ) {
        whichSpecific.viewable = selectSpecific();
    } else if ( &e.source == &whichSpecific.value ) {
        engine->repeat_results();
    }
}

Output::AdditionalData 
TraceCountFilter::announceStormSize(const Announcement &a) 
 
{ 
    if ( a.engine != NULL && a.engine->can_repeat_results() ) {
        engine = a.engine;
        selectSpecific.viewable = true;
        selectSpecific.editable = true;
        receive_changes_from( selectSpecific.value );
        receive_changes_from( whichSpecific.value );
    }
    processed_locs = 0;
    return output->announceStormSize(a).set_cluster_sources();
}

void TraceCountFilter::store_results_( bool success ) { 
    if ( selectSpecific() )
        whichSpecific.max = processed_locs;
    output->store_results( success ); 
}

void TraceCountFilter::receiveLocalizations(const EngineResult& e)
{
    localizations.clear();
    if ( selectSpecific() ) {
        int offset = whichSpecific()-1-processed_locs;
        if ( 0 <= offset && offset < int(e.size()) )
           processLocalization (e[ offset ]);
        processed_locs += e.size();
    } else
        std::for_each( e.begin(), e.end(), 
            boost::bind( &TraceCountFilter::processLocalization, this, _1 ) );
            
    EngineResult eo(e);
    std::swap( eo, localizations );
    output->receiveLocalizations(eo);
}

TraceCountConfig::TraceCountConfig()
: min_count("MinEmissionCount", 
            "Minimum number of emissions in trace"),
  disassemble("Disassemble", "Output source localizations instead of"
            " average position", false),
  selectSpecific("SelectSpecific", "Select trace by number", false),
  whichSpecific("WhichTrace", "Trace to select", 1),
  shower(selectSpecific, whichSpecific)
{
    whichSpecific.min = 1;
    whichSpecific.viewable = false;
}

std::auto_ptr< output::Output > make_trace_count_filter( const TraceCountConfig& c, std::auto_ptr< output::Output > s )
{
    return std::auto_ptr< output::Output >( new TraceCountFilter( c, s ) );
}

std::auto_ptr< output::OutputSource > make_trace_count_source() {
    return std::auto_ptr< output::OutputSource >( new FilterBuilder< TraceCountConfig, TraceCountFilter >() );
}


}
}
