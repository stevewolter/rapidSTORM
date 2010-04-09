#define CIMGBUFFER_CONFIG_CPP
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.h"

#include <limits>
#include <sstream>
#include <dStorm/helpers/thread.h>
#include <algorithm>
#include <simparm/ChoiceEntry_Impl.hh>
#include "Config.h"
#include "Method.h"
#include "Source.h"
#include "BasenameWatcher.h"
#include "AutoInputMethod.h"
#include "doc/help/context.h"

using namespace std;
using namespace ost;

namespace dStorm {
namespace input {

void _Config::registerNamedEntries() 
{
    push_back(inputMethod);
    /* The other elements will be registered by the input configs
     * that need them. */
}

MethodChoice::MethodChoice( string name, string desc )

: simparm::NodeChoiceEntry< BaseMethod >( name, desc )
{
}

MethodChoice::MethodChoice( const MethodChoice& o)
: simparm::NodeChoiceEntry< BaseMethod >(
    o, simparm::NodeChoiceEntry< BaseMethod >::NoCopy)
{
}

void MethodChoice::copyChoices( const MethodChoice& o, 
    Config& master )
{
    try {
        for ( simparm::NodeChoiceEntry< BaseMethod >::const_iterator 
              i = o.beginChoices(); i != o.endChoices();i++)
            addChoice( i->clone( master ) );
    } catch (const std::exception& e) {
        STATUS("Error in IM cloning: " << e.what());
    }
    if ( o.isValid() ) {
        try {
            std::string  name = o().getName();
            setValue( (*this)[name] );
        } catch( const std::exception& e ) {}
    }
}

_Config::_Config() 
: simparm::Set("Method", "Input options"),
  inputMethod("InputMethod", "Input driver"),
  inputFile("InputFile", "Input file location"),
  firstImage("FirstImage", "First image to load", 0),
  lastImage("LastImage", "Last image to load",
            numeric_limits<long>::max() ),
  pixel_size_in_nm("PixelSizeInNM", "Size of one input pixel in nm",
                   85)
{
    PROGRESS("Constructing");
    inputMethod.helpID = HELP_Input_driver;
    inputMethod.setHelp("Select here the type of input data to load.");
    inputMethod.setUserLevel(simparm::Entry::Beginner);

    inputFile.helpID = HELP_Input_file;
    inputFile.setHelp("Select the file to read input data from. The type "
        "of the file can be given with the InputMethod parameter.");
    inputFile.setUserLevel(simparm::Entry::Beginner);

    firstImage.setHelp("Start with this image in the file. Images "
                       "are numbered from 0.");
    firstImage.setUserLevel(simparm::Entry::Intermediate);
    lastImage.setHelp("Stop at this image in the file. Images are "
                      "numbered from 0.");
    lastImage.setUserLevel(simparm::Entry::Intermediate);

    PROGRESS("Finished");
}

std::auto_ptr< BaseSource > Config::makeImageSource() const 

{
    if ( inputMethod.isValid() ) {
        std::auto_ptr< BaseSource > rv = 
            const_cast< BaseMethod& >(inputMethod())
                .makeSource(*this);
        rv->set_ROI( firstImage(), lastImage() );
        return rv;
    } else
        throw std::runtime_error("No input method chosen");
}

_Config::~_Config() {
    inputMethod.removeAllChoices();
}

Config::Config()
: _Config(),
  basename("value", "")
{
    this->inputMethod.addChoice( new AutoMethod(*this) );
    this->registerNamedEntries();

    watcher.reset( new BasenameWatcher( this->inputMethod, basename ) );
}

Config::Config(const Config& c)
: _Config(c),
  basename(c.basename)
{ 
    this->inputMethod.copyChoices( c.inputMethod, *this );
    this->registerNamedEntries();
    watcher.reset( new BasenameWatcher( this->inputMethod, basename ) );
}

Config::~Config() {
}

}
}
