#include "BasenamePrinter.h"

BasenamePrinter::Config::Config()
 : simparm::Object("BasenamePrinter", "BasenamePrinter"),
   simparm::Listener( simparm::Event::ValueChanged ),
   outputFile("ToFile", "", ".foo")
{
    registerNamedEntries();
}

BasenamePrinter::Config::Config(const Config& c) 
: simparm::Object(c),
  simparm::Listener( simparm::Event::ValueChanged ),
  outputFile(c.outputFile)
{
    registerNamedEntries();
}

BasenamePrinter::Config::~Config() {
}

void BasenamePrinter::Config::registerNamedEntries() {
    push_back( outputFile );
    receive_changes_from(outputFile.value); 
}

BasenamePrinter* BasenamePrinter::clone() const { 
    return new BasenamePrinter(*this); 
}

BasenamePrinter::BasenamePrinter( const Config& config )
        : OutputObject("SegFault", "SegFault")
{
    std::cerr << "Output file basename is " << config.outputFile() << "\n";
}


