#define CIMGBUFFER_CONFIG_CPP
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <CImgBuffer/Config.h>
#include <limits>
#include <sstream>
#include "InputMethod.h"
#include "Image.h"
#include "Source.h"
#include "foreach.h"
#include <cc++/thread.h>
#include <algorithm>

#include "Pixel.h"

using namespace std;
using namespace ost;

namespace CImgBuffer {

typedef InputMethod<Object> Method;

template<>
void _Config<Object>::registerNamedEntries() throw() 
{
    register_entry(&inputMethod);
    /*
    register_entry(&inputFile);
    register_entry(&firstImage);
    register_entry(&lastImage);
    */
}

template<>
InputConfigChoice<Object>::InputConfigChoice( string name, string desc) throw()
: ConfigObject(name,desc), 
    ConfigEntryTypedChoice< InputConfig<Object> >( name, desc ) 
{
}

template<>
InputConfigChoice<Object>::InputConfigChoice( const InputConfigChoice& o) throw()
: ConfigObject(o), ConfigEntryTypedChoice< InputConfig<Object> >(o, false )
{
}

template<>
void InputConfigChoice<Object>::initChoices( Config<Object>& master ) throw() {
    auto_ptr< InputConfig<Object> > config;
    for ( int i = 0; i < InputMethod<Object>::numMethods(); i++) 
    {
        PROGRESS("Creating input method " << i);
        Method& method = Method::getMethod(i);
        try {
            config = method.makeConfig(master);
            if (config.get() == NULL) break;
            PROGRESS("Created input method " << config.get());
            this->addChoice( config );
        } catch (std::exception e) {
            STATUS("Error in IM construction: " << e.what());
        }
    }
}

template<>
void InputConfigChoice<Object>::copyChoices( const InputConfigChoice& o, 
    Config<Object>& master ) throw()
{
    try {
        for ( const_iterator i = o.beginChoices(); i != o.endChoices();i++)
            addChoice( i->clone( master ) );
    } catch (const std::exception& e) {
        STATUS("Error in IM cloning: " << e.what());
    }
    setValue( o.value );
}

#define NAME "InputConfig"
#define DESC "Input options"
template<>
_Config<Object>::_Config() throw () 
: ConfigObject(NAME, DESC),
  ConfigSet(NAME, DESC),
  inputMethod("InputMethod", "Input driver"),
  inputFile("InputFile", "Input file location"),
  firstImage("FirstImage", "First image to load"),
  lastImage("LastImage", "Last image to load")
{
    PROGRESS("Constructing");
    inputMethod.setHelp("Select here the type of input data to load.");
    inputMethod.setUserLevel(ConfigEntry::Beginner);

    inputFile.setHelp("Select the file to read input data from. The type "
        "of the file can be given with the InputMethod parameter.");
    inputFile.setUserLevel(ConfigEntry::Beginner);

    firstImage.setHelp("Start with this image in the file. Images "
                       "are numbered from 0.");
    firstImage = 0;
    firstImage.setUserLevel(ConfigEntry::Intermediate);
    lastImage.setHelp("Stop at this image in the file. Images are "
                      "numbered from 0.");
    lastImage = numeric_limits<unsigned long>::max();
    lastImage.setUserLevel(ConfigEntry::Intermediate);

    PROGRESS("Finished");
}

template<>
std::auto_ptr< Source<Object> > _Config<Object>::makeImageSource() const 
    throw(std::exception)
{
    if ( inputMethod.isValid() )
        return const_cast< InputConfig<Object>& >(
            inputMethod.getCurrentChoice()).makeSource();
    else
        throw std::runtime_error("No input method chosen");
}

template<>
_Config<Object>::~_Config() {
}

template<>
Config<Object>::Config() throw()
: ConfigObject(NAME, DESC)
{
    inputMethod.initChoices( *this );
    this->registerNamedEntries();
}

template<>
Config<Object>::Config(const Config<Object>& c) throw()
: ConfigObject(c), _Config<Object>(c)
{ 
    inputMethod.copyChoices( c.inputMethod, *this );
    this->registerNamedEntries();
}

}
