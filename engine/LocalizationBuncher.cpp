#include "LocalizationBuncher.h"
#include <dStorm/LocalizationFileReader.h>
#include <dStorm/Output.h>

namespace dStorm {

void LocalizationBuncher::output( data_cpp::Vector<Localization>* locs ) 
throw(Output*) 
{
    if ( outputImage >= first && outputImage <= last ) {
        engine_result.forImage = outputImage;
        if ( locs ) {
            engine_result.first = locs->ptr();
            engine_result.number = locs->size();
        } else {
            engine_result.first = NULL;
            engine_result.number = 0;
        }
        Output::Result result =
            target.receiveLocalizations(engine_result);
        if (result == Output::StopEngine) {
            target.propagate_signal( 
                Output::Engine_run_is_aborted );
            target.propagate_signal( 
                Output::Engine_run_failed );
            throw &target;
        }
    }
    
    outputImage++;
}

void LocalizationBuncher::print_canned_results_where_possible()
throw(Output*) 
{
    while ( outputImage <= currentImage ) {
        if ( canned.find( outputImage ) != canned.end() ) {
            int decanned = outputImage;
            output( canned[decanned] );
            delete canned[decanned];
            canned.erase(decanned);
        } else if ( outputImage + 50 < currentImage ) {
            output( NULL );
        } else
            break;
    }
}

void LocalizationBuncher::can_results_or_publish(int) 
throw(Output*) 
{
    print_canned_results_where_possible();

    if ( outputImage == currentImage ) {
        output( buffer.get() );
        buffer->clear();
    } else if ( buffer->size() != 0 ) {
        canned.insert( std::make_pair( currentImage, buffer.release() ) );
        buffer.reset( new data_cpp::Vector<Localization>() );
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
    while ( outputImage <= last ) {
        std::map<int, data_cpp::Vector<Localization>* >::iterator i;
        i = canned.find( outputImage );
        if ( i != canned.end() ) {
            outputImage = i->first;
            output( i->second );
            delete i->second;
            canned.erase(i);
        } else if ( outputImage == currentImage ) {
            output( buffer.get() );
            buffer->clear();
        } else {
            output( NULL );
        }
    }
    target.propagate_signal(Output::Engine_run_succeeded);
}

void LocalizationBuncher::noteTraits(
    const CImgBuffer::Traits<Localization>& traits,
    unsigned int firstImage, unsigned int lastImage)

{
    last = std::min( lastImage, traits.imageNumber-1 );
    first = std::min( firstImage, last );

    Output::Announcement announcement
        (traits, last-first+1);
    Output::AdditionalData data = 
        target.announceStormSize(announcement);

    if (data != Output::NoData) {
        std::stringstream ss;
        ss << 
            "A selected data processing function "
            "requires data about the input data ("
            << data << ") that are not "
            "present in a localization file.";
        throw std::runtime_error(ss.str());
    }
}

CImgBuffer::Management
LocalizationBuncher::accept(int index, int num, Localization *loc)
{
    if (index < last_index)
        reset();
    for (int i = 0; i < num; i++) {
        const Localization& l = loc[i];
        unsigned int imNum = l.getImageNumber();
        if ( buffer.get() == NULL )
            buffer.reset( new data_cpp::Vector<Localization>() );
        else {
            try {
                if (currentImage != imNum)
                    can_results_or_publish( imNum );
            } catch (const Output*c) {
                return CImgBuffer::Delete_objects;
            }
        }

        currentImage = imNum;
        buffer->push_back(l);
    }

    last_index = index;

    return CImgBuffer::Delete_objects;
}

void LocalizationBuncher::reset() {
    buffer->clear();
    target.propagate_signal( 
        Output::Engine_run_is_aborted );
    target.propagate_signal( 
        Output::Engine_is_restarted );
}

}
