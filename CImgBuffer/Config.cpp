#define CIMGBUFFER_CONFIG_CPP
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <CImgBuffer/Config.h>
#include <limits>
#include <sstream>
#include "InputMethod.h"
#include "Source.h"
#include "foreach.h"
#include <cc++/thread.h>
#include <algorithm>
#include "BasenameWatcher.h"
#include "AutoInputMethod.h"
#include "doc/help/context.h"
#include <simparm/ChoiceEntry_Impl.hh>

using namespace std;
using namespace ost;

namespace CImgBuffer {

void _Config::registerNamedEntries() 
{
    register_entry(&inputMethod);
    /* The other elements will be registered by the input configs
     * that need them. */
}

InputConfigChoice::InputConfigChoice( string name, string desc )

: simparm::NodeChoiceEntry< BaseInputConfig >( name, desc )
{
}

InputConfigChoice::InputConfigChoice( const InputConfigChoice& o)
: simparm::Node(o), 
  simparm::NodeChoiceEntry< BaseInputConfig >(
    o, simparm::NodeChoiceEntry< BaseInputConfig >::NoCopy)
{
}

void InputConfigChoice::copyChoices( const InputConfigChoice& o, 
    Config& master )
{
    try {
        for ( simparm::NodeChoiceEntry< BaseInputConfig >::const_iterator 
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
: simparm::Set("InputConfig", "Input options"),
  inputMethod("InputMethod", "Input driver"),
  inputFile("InputFile", "Input file location"),
  firstImage("FirstImage", "First image to load", 0),
  lastImage("LastImage", "Last image to load",
            numeric_limits<unsigned long>::max() ),
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

std::auto_ptr< BaseSource > _Config::makeImageSource() const 

{
    if ( inputMethod.isValid() ) {
        std::auto_ptr< BaseSource > rv = 
            const_cast< BaseInputConfig& >(inputMethod()).makeSource();
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
    this->inputMethod.addChoice( new AutoInputConfig(*this) );
    this->registerNamedEntries();

    watcher.reset( new BasenameWatcher( this->inputFile, basename ) );
}

Config::Config(const Config& c)
: simparm::Node(c), _Config(c),
  basename(c.basename)
{ 
    this->inputMethod.copyChoices( c.inputMethod, *this );
    this->registerNamedEntries();
    watcher.reset( new BasenameWatcher( this->inputFile, basename ) );
}

Config::~Config() {
}

}
