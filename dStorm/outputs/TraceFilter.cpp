#include "TraceFilter.h"
#include <dStorm/output/Filter.h>
#include <dStorm/Engine.h>
#include <numeric>
#include <boost/bind/bind.hpp>
#include <boost/iterator/transform_iterator.hpp>

namespace dStorm {
using namespace output;
namespace outputs {

class TraceCountFilter : public output::Filter
{
  private:
    EngineResult localizations;
    int minCount;
    bool disassemble;

    simparm::BoolEntry selectSpecific;
    simparm::Entry<unsigned long> whichSpecific;
    dStorm::Engine *engine;
    int processed_locs;
    simparm::BaseAttribute::ConnectionStore listening[3];

    int count_localizations_in( const Localization &l );
    void processLocalization( const Localization& l);

    /** As of yet, the copy constructor is not implemented. */
    TraceCountFilter(const TraceCountFilter&);
    TraceCountFilter& operator=(const TraceCountFilter&);

    void store_results_( bool success );
    void attach_ui_( simparm::NodeHandle );

    void set_which_viewability() { whichSpecific.viewable = selectSpecific(); }
    void repeat_results() { engine->repeat_results(); }

  public:
    TraceCountFilter(const TraceCountConfig& config,
                     std::auto_ptr<output::Output> output);
    ~TraceCountFilter() {}

    AdditionalData announceStormSize(const Announcement &a) ;
    RunRequirements announce_run(const RunAnnouncement& a) 
        { processed_locs = 0; return Filter::announce_run(a); }

    void receiveLocalizations(const EngineResult& e);
};

TraceCountFilter::TraceCountFilter(
    const TraceCountConfig& c,
    std::auto_ptr<output::Output> output
) 
: Filter(output),
  minCount(c.min_count()), 
    disassemble(c.disassemble()), 
    selectSpecific(c.selectSpecific),
    whichSpecific(c.whichSpecific),
    engine(NULL)
{  
}

void TraceCountFilter::attach_ui_( simparm::NodeHandle at ) {
    listening[0] = selectSpecific.value.notify_on_value_change(
        boost::bind( &TraceCountFilter::set_which_viewability, this ) );
    listening[1] = selectSpecific.value.notify_on_value_change(
        boost::bind( &TraceCountFilter::repeat_results, this ) );
    listening[2] = whichSpecific.value.notify_on_value_change(
        boost::bind( &TraceCountFilter::repeat_results, this ) );

    this->selectSpecific.attach_ui( at );
    this->whichSpecific.attach_ui( at );
    Filter::attach_children_ui( at ); 
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

Output::AdditionalData 
TraceCountFilter::announceStormSize(const Announcement &a) 
 
{ 
    if ( a.engine != NULL && a.engine->can_repeat_results() ) {
        engine = a.engine;
        selectSpecific.viewable = true;
        selectSpecific.editable = true;
    }
    processed_locs = 0;
    AdditionalData rv = Filter::announceStormSize(a);
    return rv.set_cluster_sources();
}

void TraceCountFilter::store_results_( bool success ) { 
    if ( selectSpecific() )
        whichSpecific.max = processed_locs;
    Filter::store_children_results( success ); 
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
    Filter::receiveLocalizations(eo);
}

TraceCountConfig::TraceCountConfig()
: min_count("MinEmissionCount", 
            "Minimum number of emissions in trace"),
  disassemble("Disassemble", "Output source localizations instead of"
            " average position", false),
  selectSpecific("SelectSpecific", "Select trace by number", false),
  whichSpecific("WhichTrace", "Trace to select", 1)
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

void TraceCountConfig::attach_ui( simparm::NodeHandle at )
{
    min_count.attach_ui(at);
    disassemble.attach_ui(at);
    selectSpecific.attach_ui(at);
    whichSpecific.attach_ui(at);

    listening = selectSpecific.value.notify_on_value_change(
        boost::bind( &TraceCountConfig::set_which_viewability, this )
    );
}

}
}
