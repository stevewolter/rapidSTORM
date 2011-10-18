#include "TraceFilter.h"
#include <dStorm/Engine.h>
#include <numeric>
#include <boost/bind/bind.hpp>
#include <boost/iterator/transform_iterator.hpp>

namespace dStorm {
using namespace output;
namespace outputs {

TraceCountFilter::TraceCountFilter(
    const Config& c,
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
            std::copy( l.children->begin(), l.children->end(), back_inserter( localizations ) );
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

void TraceCountFilter::propagate_signal(ProgressSignal s) { 
    if ( s == Engine_is_restarted )
        processed_locs = 0;
    else if ( s == Engine_run_succeeded && selectSpecific() ) {
        whichSpecific.max = processed_locs;
    }
    output->propagate_signal(s); 
}

Output::Result
TraceCountFilter::receiveLocalizations(const EngineResult& e)
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
    std::swap( static_cast< std::vector<Localization>& >(eo), localizations );
    return output->receiveLocalizations(eo);
}

TraceCountFilter::_Config::_Config()
: simparm::Object("TraceFilter", "Trace filter"),
  min_count("MinEmissionCount", 
            "Minimum number of emissions in trace"),
  disassemble("Disassemble", "Output source localizations instead of"
            " average position", false),
  selectSpecific("SelectSpecific", "Select trace by number", false),
  whichSpecific("WhichTrace", "Trace to select", 1)
{
    whichSpecific.min = 1;
    whichSpecific.viewable = false;

    userLevel = Intermediate;
}

}
}
