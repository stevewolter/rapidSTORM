#include "BasenamePrinter.h"

#include <simparm/Entry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <iostream>
#include <stdexcept>
#include <boost/units/io.hpp>

struct BasenamePrinter
: public dStorm::output::Output
{
    struct Config;

    BasenamePrinter(const Config& config);
    BasenamePrinter* clone() const;

    AdditionalData announceStormSize(const Announcement& a) 
        { return AdditionalData(); }
    void receiveLocalizations(const EngineResult&) {}

};

struct BasenamePrinter::Config
 : simparm::Listener
{
    dStorm::output::BasenameAdjustedFileEntry outputFile;

    static std::string get_name() { return "BasenamePrinter"; }
    static std::string get_description() { return get_name(); }

    Config();
    void attach_ui(simparm::Node&);
    bool can_work_with(const dStorm::output::Capabilities&)
        {return true;}
    void operator()( const simparm::Event& ) {
        std::cerr << this << ": Displaying output file name "
                  << outputFile.unformatted_name() << "\n";
    }
};
BasenamePrinter::Config::Config()
 : 
   simparm::Listener( simparm::Event::ValueChanged ),
   outputFile("ToFile", "", ".foo")
{
}

void BasenamePrinter::Config::attach_ui(simparm::Node& at) {
    outputFile.attach_ui( at );
    receive_changes_from(outputFile.value); 
}

BasenamePrinter* BasenamePrinter::clone() const { 
    return new BasenamePrinter(*this); 
}

BasenamePrinter::BasenamePrinter( const Config& config )
{
    std::cerr << "Output file basename is " << config.outputFile() << "\n";
}

std::auto_ptr< dStorm::output::OutputSource > make_basename_printer_source() {
    return std::auto_ptr< dStorm::output::OutputSource >( 
        new dStorm::output::FileOutputBuilder< BasenamePrinter::Config, BasenamePrinter >() );
}
