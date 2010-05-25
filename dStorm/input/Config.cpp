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
#include <simparm/UnitEntry_Impl.hh>
#include "Config.h"
#include "Method.h"
#include "Source.h"
#include "BasenameWatcher.h"
#include "AutoInputMethod.h"
#include "doc/help/context.h"
#include <dStorm/units/frame_count.h>
#include "InputFilter_impl.h"
#include "ROIFilter.h"
#include <dStorm/engine/Image.h>
#include <dStorm/Localization.h>
#include <dStorm/input/LocalizationTraits.h>
#include "Buffer_impl.h"
#include "ResolutionSetter.h"

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
                   85.0f * boost::units::si::nanometre / cs_units::camera::pixel)
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

template <typename ObjectType>
void Config::try_to_add_ROI_filter(std::auto_ptr<BaseSource>& rv) const
{
    typedef Source<ObjectType> Src;
    typedef ROIFilter<ObjectType> Functor;
    typedef InputFilter<ObjectType,Functor> Filter;

    Src* t = dynamic_cast<Src*>(rv.get());
    if ( t != NULL ) {
        std::auto_ptr<Src> b( t );
        rv.release();
        std::auto_ptr<Src> n( 
            new Filter(
                b, 
                Functor(
                    firstImage() * cs_units::camera::frame,
                    lastImage() * cs_units::camera::frame)));
        rv = n;
    }
}

template <typename Filter>
void try_to_add(std::auto_ptr<BaseSource>& rv) 
{
    typedef Source<typename Filter::value_type> Src;
    std::auto_ptr<Src> t( dynamic_cast<Src*>( rv.get() ) );
    if ( t.get() != NULL ) {
        rv.release();
        rv.reset( new Filter(t) );
    }
}

template <typename Filter>
void try_to_add(std::auto_ptr<BaseSource>& rv, const Config& c) 
{
    typedef Source<typename Filter::value_type> Src;
    std::auto_ptr<Src> t( dynamic_cast<Src*>( rv.get() ) );
    if ( t.get() != NULL ) {
        rv.release();
        rv.reset( new Filter(t, c) );
    }
}

std::auto_ptr< BaseSource > Config::makeImageSource() const 

{
    if ( inputMethod.isValid() ) {
        std::auto_ptr< BaseSource > rv = 
            const_cast< BaseMethod& >(inputMethod())
                .makeSource(*this);
        DEBUG("Source flags are " << rv->flags.to_string());
        if ( firstImage != 0 || lastImage != numeric_limits<long>::max() ) 
        {
            try_to_add_ROI_filter<dStorm::engine::Image>(rv);
            try_to_add_ROI_filter<dStorm::Localization>(rv);
        }

        if ( rv->flags.test( BaseSource::TimeCritical ) ||
             !rv->flags.test( BaseSource::Repeatable ) ||
             !rv->flags.test( BaseSource::MultipleConcurrentIterators ) )
        {
            if ( rv->flags.test( BaseSource::TimeCritical ) ) {
                try_to_add<Buffer<dStorm::engine::Image, true> >(rv);
                try_to_add< Buffer<dStorm::Localization, true> >(rv);
            } else {
                try_to_add<Buffer<dStorm::engine::Image, false> >(rv);
                try_to_add< Buffer<dStorm::Localization, false> >(rv);
            }
        }

        try_to_add<ResolutionSetter<dStorm::engine::Image> >(rv, *this);
        try_to_add<ResolutionSetter<dStorm::Localization> >(rv, *this);
            
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

void _Config::addInput( std::auto_ptr<BaseMethod> method ) {
    inputMethod.addChoice( method );
}

dStorm::SizeTraits<2>::Resolution _Config::get_resolution() const
{
    boost::units::quantity< boost::units::divide_typeof_helper<boost::units::si::length,cs_units::camera::length>::type, float > q1;
    q1 = (pixel_size_in_nm() / (1E9 * boost::units::si::nanometre) * boost::units::si::metre);
    return 1.0f / q1;
}

}
}
