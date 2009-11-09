#include "Config.h"
#include "doc/help/context.h"
#include "OutputSource.h"
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {
namespace output {

Config::Config() 
: simparm::NodeChoiceEntry<OutputSource>
    ("ChooseTransmission", "Choose new output")
{
    this->helpID = HELP_ChooseOutput;
}

Config::Config( const Config& other ) 
: SourceFactory(other),
  simparm::NodeChoiceEntry<OutputSource>(other, 
        simparm::NodeChoiceEntry<OutputSource>::DeepCopy )
{
}

Config* Config::clone() const {
    return new Config(*this); 
}

std::auto_ptr<OutputSource> 
Config::make_output_source() 
{
    if ( isValid() ) {
        std::auto_ptr<OutputSource> rv( value().clone() );
        rv->show_in_tree = true;
        return rv;
    } else {
        return std::auto_ptr<OutputSource>(NULL);
    }
}

void Config::addChoice(OutputSource *toAdd) {
    toAdd->show_in_tree = false;
    simparm::NodeChoiceEntry<OutputSource>::addChoice( toAdd );
}

Config::BasenameResult
Config::set_output_file_basename(
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
}
