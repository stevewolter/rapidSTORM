#include "BasenamePrinter.h"

BasenamePrinter::_Config::_Config()
 : simparm::Object("BasenamePrinter", "BasenamePrinter"),
   simparm::Listener( simparm::Event::ValueChanged ),
   outputFile("ToFile", "", ".foo")
{
}

BasenamePrinter* BasenamePrinter::clone() const { 
    return new BasenamePrinter(*this); 
}

BasenamePrinter::BasenamePrinter( const Config& config )
        : OutputObject("SegFault", "SegFault")
{
    std::cerr << "Output file basename is " << config.outputFile() << "\n";
}


