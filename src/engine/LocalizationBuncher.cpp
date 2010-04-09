#include "debug.h"

#include "LocalizationBuncher.h"
#include <dStorm/output/Output.h>
#include <dStorm/output/Trace.h>

using namespace dStorm::output;

namespace dStorm {
namespace engine {

void LocalizationBuncher::output( Can* locs ) 
throw(Output*) 
{
    if ( outputImage >= first && outputImage <= last ) {
        engine_result.forImage = outputImage;
        if ( locs ) {
            DEBUG("Outputting " << locs->size() << " localizations for " << outputImage);
            engine_result.first = locs->ptr();
            engine_result.number = locs->size();
        } else {
            DEBUG("Outputting no localizations for " << outputImage);
            engine_result.first = NULL;
            engine_result.number = 0;
        }
        Output::Result result =
            target.receiveLocalizations(engine_result);
        if (result == Output::StopEngine) {
            DEBUG("Got result StopEngine");
            target.propagate_signal( 
                Output::Engine_run_is_aborted );
            target.propagate_signal( 
                Output::Engine_run_failed );
            throw &target;
        } else {
            DEBUG("Got result " << result << ", continuing");
        }
    } else {
        DEBUG("Image " << outputImage << " not in range " << first << " to " << last);
    }
    
    outputImage = outputImage + 1 * cs_units::camera::frame;
}

void LocalizationBuncher::print_canned_results_where_possible()
throw(Output*) 
{
    while ( outputImage <= currentImage ) {
        if ( canned.find( outputImage ) != canned.end() ) {
            frame_index decanned = outputImage;
            output( canned[decanned] );
            delete canned[decanned];
            canned.erase(decanned);
#if 0           /* Makes problems. Rather can everything. */
        } else if ( outputImage + 50 < currentImage ) {
            output( NULL );
#endif
        } else
            break;
    }
}

void LocalizationBuncher::can_results_or_publish(frame_index) 
throw(Output*) 
{
    print_canned_results_where_possible();

    if ( outputImage == currentImage ) {
        output( buffer.get() );
        buffer->clear();
    } else if ( buffer->size() != 0 ) {
        canned.insert( std::make_pair( currentImage, buffer.release() ) );
        buffer.reset( new Can() );
    }
}

LocalizationBuncher::LocalizationBuncher(Output& output)
: currentImage(0), outputImage(0), target(output), last_index(0)
{
}

LocalizationBuncher::~LocalizationBuncher() {
    ensure_finished();
}

void LocalizationBuncher::ensure_finished() 
{
    if ( buffer.get() != NULL ) {
        DEBUG("Canning last results");
        can_results_or_publish( currentImage );
    }
    DEBUG("Finished reading file");
    while ( outputImage < lastInFile ) {
        DEBUG("Finish iteration with output image " << outputImage);
        Canned::iterator i;
        i = canned.find( outputImage );
        if ( i != canned.end() ) {
            DEBUG("Outputting from can");
            outputImage = i->first;
            output( i->second );
            delete i->second;
            canned.erase(i);
        } else if ( outputImage == currentImage ) {
            DEBUG("Outputting current");
            if ( buffer.get() != NULL ) {
                output( buffer.get() );
                buffer->clear();
            }
        } else {
            DEBUG("Outputting nothing");
            output( NULL );
        }
    }
    target.propagate_signal(Output::Engine_run_succeeded);
    DEBUG("Sent success signal");
}

void LocalizationBuncher::noteTraits(
    const input::Traits<Localization>& traits,
    frame_index firstImage, frame_index lastImage)

{
    DEBUG("Set bounds " << firstImage << " " << lastImage);
    if ( ! traits.total_frame_count.is_set() )
        throw std::runtime_error("Total number of frames in STM file must be known");
    lastInFile = *traits.total_frame_count;
    last = lastImage;
    first = std::min( firstImage, last );

    Output::Announcement announcement(traits);
    Output::AdditionalData data = 
        target.announceStormSize(announcement);

    if ( data.test( output::Capabilities::SourceImage ) ) {
        std::stringstream message;
        message << "One of your output modules needs access to the raw "
                << "images of the acquisition. These are not present in "
                << "a localizations file. Either remove the output or "
                << "choose a non-text input file";
        throw std::runtime_error(message.str());
    } else if (
        data.test( output::Capabilities::SmoothedImage) ||
        data.test( output::Capabilities::CandidateTree) ||
        data.test( output::Capabilities::InputBuffer) ) 
    {
        std::stringstream ss;
        ss << 
            "A selected data processing function "
            "requires data about the input data ("
            << data << ") that are not "
            "present in a localization file.";
        throw std::runtime_error(ss.str());
    }
}

void LocalizationBuncher::Can::push_back( const Localization &loc )
{
    traces.allocate( number_of_traces( loc ) );
    deep_copy( loc, *this );
}

int LocalizationBuncher::Can::number_of_traces( const Localization& loc ) {
    if ( ! loc.has_source_trace() )
        return 1;
    else {
        int accum = 0;
        const Trace& t = loc.get_source_trace();
        for ( Trace::const_iterator i = t.begin(); i != t.end(); i++)
            accum += number_of_traces( loc );
        return accum;
    }
        
}

void LocalizationBuncher::Can::deep_copy( 
    const Localization &loc, data_cpp::Vector<Localization>& to )
{
    Localization *target = to.allocate(1);
    *target = loc;
    if ( loc.has_source_trace() ) {
        Trace *trace = traces.allocate( 1 );
        new (trace) Trace();
        const Trace& t = loc.get_source_trace();
        for ( Trace::const_iterator i = t.begin(); i != t.end(); i++)
            deep_copy( *i, *trace );
        traces.commit( 1 );
        target->set_source_trace( *trace );
    }
    to.commit(1);
}

input::Management
LocalizationBuncher::accept(int index, int num, Localization *loc)
{
    if (index < last_index)
        reset();
    for (int i = 0; i < num; i++) {
        const Localization& l = loc[i];
        frame_index imNum = l.getImageNumber();
        if ( buffer.get() == NULL )
            buffer.reset( new Can() );
        else {
            try {
                if (currentImage != imNum)
                    can_results_or_publish( imNum );
            } catch (const Output*c) {
                return input::Delete_objects;
            }
        }

        currentImage = imNum;
        buffer->push_back( l );
    }

    last_index = index;

    return input::Delete_objects;
}

void LocalizationBuncher::reset() {
    buffer->clear();
    target.propagate_signal( 
        Output::Engine_run_is_aborted );
    target.propagate_signal( 
        Output::Engine_is_restarted );
}

}
}
