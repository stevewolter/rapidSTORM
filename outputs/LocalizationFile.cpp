#define DSTORM_LOCALIZATIONFILE_CPP
#include "debug.h"
#include "LocalizationFile.h"
#include <string.h>
#include <stdlib.h>
#include <dStorm/doc/context.h>
#include <dStorm/unit_matrix_operators.h>
#include <iomanip>
#include <boost/bind/bind.hpp>

#include <dStorm/localization_file/fields.h>

using namespace std;
using dStorm::LocalizationFile::field::Interface;

namespace dStorm {
namespace output {

LocalizationFile::_Config::_Config() 
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
    outputFile.helpID = HELP_Table_ToFile;
}

using namespace dStorm::LocalizationFile;

void LocalizationFile::open() {
    if ( filename != "-" ) {
        fileKeeper.reset( new ofstream( filename.c_str(), 
                                    ios_base::out | ios_base::trunc ) );
        file = fileKeeper.get();
    } else
        file = &cout;

    field = field::Interface::construct(traits);
    /** Write XML header for localization file */
    XMLNode topNode = 
        XMLNode::createXMLTopNode( "dummy" );
    field->makeNode( topNode, traits );

    XMLSTR str = topNode.getChildNode(0).createXMLString(0);
    *file << "# " << str << "\n";
    free( str );
}

Output::AdditionalData
LocalizationFile::announceStormSize(const Announcement &a) {
    traits = a;

    open();

    return AdditionalData();
}

void LocalizationFile::output( const Localization& l ) {
    field->write( *file, l );
    *file << "\n";
}

Output::Result LocalizationFile::receiveLocalizations(const EngineResult &er) 
{
    if ( er.empty() )
        (*file) << "# No localizations in image " << er.forImage.value() << std::endl;
    else
        std::for_each( er.begin(), er.end(), 
            boost::bind(&LocalizationFile::output, this, _1) ); 
    if ( ! (*file) ) {
        std::cerr << "Warning: Writing localizations to "
                  << filename << " failed.\n";
        return RemoveThisOutput;
    }
    return KeepRunning;
}

void LocalizationFile::propagate_signal(Output::ProgressSignal s) {
    if ( s == Engine_is_restarted ) {
        fileKeeper.reset(NULL);
        open();
    } else if ( s == Prepare_destruction ) {
        if (fileKeeper.get() != NULL) fileKeeper->close();
        fileKeeper.reset(NULL);
    }
}

LocalizationFile::LocalizationFile(const Config &c) 

: OutputObject("LocalizationFile", "File output status"),
  filename(c.outputFile()),
  format( 4, Eigen::Raw, " ", " " )
{
    if ( filename == "" )
        throw std::runtime_error("No filename provided for "
                                 "localization output file");
}

LocalizationFile::~LocalizationFile() {}

}
}
