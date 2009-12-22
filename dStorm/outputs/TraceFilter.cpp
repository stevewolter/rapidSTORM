#include "TraceFilter.h"

namespace dStorm {
using namespace output;
namespace outputs {

TraceCountFilter::TraceCountFilter(
    const Config& c,
    std::auto_ptr<output::Output> output
) 
: output::OutputObject("TraceFilter", "Trace filter"),
  simparm::Node::Callback( Node::ValueChanged ),
  minCount(c.min_count()), 
    disassemble(c.disassemble()), output(output),
    selectSpecific(c.selectSpecific),
    whichSpecific(c.whichSpecific),
    result_repeater(NULL)
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
    if ( l.has_source_trace() ) {
        int n = 0;
        for ( Trace::const_iterator i = l.get_source_trace().begin();
                i != l.get_source_trace().end(); i++)
            n += count_localizations_in( *i );
        return n;
    } else
        return 1;
}

void TraceCountFilter::processLocalization( const Localization& l)
 
{
    if ( count_localizations_in( l ) >= minCount ) {
        if (disassemble && l.has_source_trace() ) {
            localizations.push_back( l.get_source_trace() );
        } else
            localizations.push_back( l );
    }
}

void TraceCountFilter::operator()(Node& src, Cause c, Node*) {
    if ( &src == &selectSpecific.value ) {
        whichSpecific.viewable = selectSpecific();
    } else if ( &src == &whichSpecific.value ) {
        result_repeater->repeat_results();
    }
}

Output::AdditionalData 
TraceCountFilter::announceStormSize(const Announcement &a) 
 
{ 
    if ( a.result_repeater != NULL ) {
        result_repeater = a.result_repeater;
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
        whichSpecific.hasMax = true;
    }
    output->propagate_signal(s); 
}

Output::Result
TraceCountFilter::receiveLocalizations(const EngineResult& e)
{
    ost::MutexLock lock(mutex);
    localizations.clear();
    if ( selectSpecific() ) {
        int offset = whichSpecific()-1-processed_locs;
        if ( 0 <= offset && offset < e.number )
            processLocalization (e.first[ offset ]);
        processed_locs += e.number;
    } else
        for ( int i = 0; i < e.number; i++ )
            processLocalization (e.first[i]);
            
    EngineResult eo(e);
    eo.number = localizations.size();
    eo.first = localizations.ptr();
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
    whichSpecific.hasMin = true;
    whichSpecific.viewable = false;

    userLevel = Intermediate;
}

}
}
