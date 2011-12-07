#define DSTORM_LOCALIZATIONFILE_CPP
#include "debug.h"
#include "Output.h"
#include <string.h>
#include <stdlib.h>
#include <boost/units/Eigen/Array>
#include <iomanip>
#include <boost/bind/bind.hpp>

#include "field.h"

using namespace std;

namespace dStorm {
namespace localization_file {
namespace writer {

Output::_Config::_Config() 
: simparm::Object("Table", "Localizations file"),
  outputFile("ToFile", "Write localizations to", ".txt"),
  traces("Traces", "Print localizations seperated by traces")
{
    outputFile.setHelp(
        "If given, this parameters determines a file to which "
        "the raw fit data will be written. The output is one "
        "line per fit, with X- and Y-coordinate, image number "
        "and fit amplitude, fields separated by spaces.");
    outputFile.setUserLevel(simparm::Object::Beginner);
    outputFile.helpID = "Table_ToFile";
}

void Output::open() {
    if ( filename != "-" ) {
        fileKeeper.reset( new ofstream( filename.c_str(), 
                                    ios_base::out | ios_base::trunc ) );
        file = fileKeeper.get();
    } else
        file = &cout;

    field = Field::construct(traits);
    /** Write XML header for localization file */
    std::auto_ptr<TiXmlNode> node = field->makeNode( traits );

    *file << "# " << *node << "\n";
}

Output::AdditionalData
Output::announceStormSize(const Announcement &a) {
    traits = a;

    open();

    return AdditionalData();
}

void Output::output( const Localization& l ) {
    field->write( *file, l );
    *file << "\n";
}

Output::Result Output::receiveLocalizations(const EngineResult &er) 
{
    if ( er.empty() )
        (*file) << "# No localizations in image " << er.forImage.value() << std::endl;
    else
        std::for_each( er.begin(), er.end(), 
            boost::bind(&Output::output, this, _1) ); 
    if ( ! (*file) ) {
        std::cerr << "Warning: Writing localizations to "
                  << filename << " failed.\n";
        return RemoveThisOutput;
    }
    return KeepRunning;
}

void Output::propagate_signal(Output::ProgressSignal s) {
    if ( s == Engine_is_restarted ) {
        fileKeeper.reset(NULL);
        open();
    } else if ( s == Prepare_destruction ) {
        if (fileKeeper.get() != NULL) fileKeeper->close();
        fileKeeper.reset(NULL);
    }
}

Output::Output(const Config &c) 

: OutputObject("LocalizationFile", "File output status"),
  filename(c.outputFile())
{
    if ( filename == "" )
        throw std::runtime_error("No filename provided for "
                                 "localization output file");
}

Output::~Output() {}

}
}
}

namespace dStorm {
namespace output {

template <>
std::auto_ptr<OutputSource> 
make_output_source< localization_file::writer::Output >()
{
    return std::auto_ptr<OutputSource>( new localization_file::writer::Output::Source() );
}

}
}
