#include "test-plugin/BasenamePrinter.h"

#include "simparm/Entry.h"
#include "output/Output.h"
#include "output/FileOutputBuilder.h"
#include <iostream>
#include <stdexcept>
#include <boost/units/io.hpp>

struct BasenamePrinter
: public dStorm::output::Output
{
    struct Config;

    BasenamePrinter(const Config& config);
    BasenamePrinter* clone() const;

    void announceStormSize(const Announcement& a) OVERRIDE {}
    void receiveLocalizations(const EngineResult&) OVERRIDE {}

};

struct BasenamePrinter::Config
{
    dStorm::output::BasenameAdjustedFileEntry outputFile;
    simparm::BaseAttribute::ConnectionStore listening;

    static std::string get_name() { return "BasenamePrinter"; }
    static std::string get_description() { return get_name(); }
    static simparm::UserLevel get_user_level() { return simparm::Debug; }

    Config();
    void attach_ui(simparm::NodeHandle);
    void print() {
        std::cerr << this << ": Displaying output file name "
                  << outputFile.unformatted_name() << "\n";
    }
};
BasenamePrinter::Config::Config()
 : outputFile("ToFile", "", ".foo")
{
}

void BasenamePrinter::Config::attach_ui(simparm::NodeHandle at) {
    listening = outputFile.value.notify_on_value_change( 
        boost::bind( &BasenamePrinter::Config::print, this ) );
    outputFile.attach_ui( at );
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
