#include "OutputSource.h"
#include "FilterSource.h"
#include <list>

#include "transmissions/LocalizationFile.h"
#include "transmissions/Viewer.h"
#include "transmissions/ViewerConfig.h"
#include "transmissions/LocalizationCounter.h"
#include "transmissions/ProgressMeter.h"
#include "transmissions/AverageImage.h"
#include "transmissions/TraceFilter.h"
#include "transmissions/LocalizationFilter.h"
#include "transmissions/Slicer.h"
#include "transmissions/RawImageFile.h"
#include <simparm/Object.hh>
#include "help_context.h"

#include "BasicOutputs.h"

using namespace std;

template <typename T>
std::auto_ptr<T> make_auto_ptr(T* t) 
    { return std::auto_ptr<T>(t); }

namespace dStorm {

template<typename Type>
void delete_arg(Type *p) { delete p; }

BasicOutputs::BasicOutputs() 
: simparm::NodeChoiceEntry<OutputSource>
    ("ChooseTransmission", "Choose new output")
{
    this->helpID = HELP_ChooseOutput;
    addChoice( new LocalizationFile::Source() );
    addChoice( new Viewer::Source() );
    addChoice( new ProgressMeter::Source() );
    addChoice( new LocalizationCounter::Source() );
    addChoice( new AverageImage::Source() );
    addChoice( new LocalizationFilter::Source() );
    addChoice( new TraceCountFilter::Source() );
    addChoice( new Slicer::Source() );
    addChoice( new RawImageFile::Source() );
}

std::auto_ptr<OutputSource> 
BasicOutputs::make_output_source() 
{
    if ( isValid() ) {
        std::auto_ptr<OutputSource> rv( value().clone() );
        rv->show_in_tree = true;
        return rv;
    } else {
        return std::auto_ptr<OutputSource>(NULL);
    }
}

void BasicOutputs::addChoice(OutputSource *toAdd) {
    toAdd->show_in_tree = false;
    simparm::NodeChoiceEntry<OutputSource>::addChoice( toAdd );
}

BasicOutputs::BasenameResult
BasicOutputs::set_output_file_basename(
    const std::string& new_basename, std::set<std::string>& avoid)

{
    BasenameResult r;
    for ( const_iterator i = beginChoices(); i != endChoices(); i++) {
        r = const_cast<OutputSource&>(*i).
                set_output_file_basename(new_basename, avoid);
        if ( r != OutputSource::Basename_Accepted )
            return r;
    }
    return OutputSource::Basename_Accepted;
}

}
