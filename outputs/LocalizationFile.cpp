#define DSTORM_LOCALIZATIONFILE_CPP
#include "LocalizationFile.h"
#include <string.h>
#include <stdlib.h>
#include <dStorm/output/Trace.h>
#include "doc/help/context.h"
#include <dStorm/unit_matrix_operators.h>

#include <dStorm/localization_file/fields.h>
#include <dStorm/localization_file/known_fields.h>

using namespace std;

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
    outputFile.setUserLevel(simparm::Entry::Beginner);
    outputFile.helpID = HELP_Table_ToFile;
}

void LocalizationFile::printFit(const Localization &f, 
    int localizationDepth) 
{
    if ( localizationDepth > 0 && f.has_source_trace() ) {
        (*file) << "\n\n";
        const Trace& trace = f.get_source_trace();
        for (Trace::const_iterator i = trace.begin(); 
                                    i != trace.end(); i++)
            printFit(*i, 0);
    } else {
      (*file) << f.x().value() << " " 
           << f.y().value() << " "
           << f.getImageNumber().value() << " "
           << f.strength().value() ;
      if ( traits.two_kernel_improvement_is_set )
        (*file) << " " << f.two_kernel_improvement();
      if ( traits.covariance_matrix_is_set )
        (*file) << " " << Eigen::unitless_value(f.fit_covariance_matrix()).format(format);
      (*file) << "\n";
    }
}

using namespace dStorm::LocalizationFile;

void LocalizationFile::open() {
    if ( filename != "-" ) {
        fileKeeper.reset( new ofstream( filename.c_str(), 
                                    ios_base::out | ios_base::trunc ) );
        file = fileKeeper.get();
    } else
        file = &cout;

    /** Write XML header for localization file */
    XMLNode topNode = 
        XMLNode::createXMLTopNode( "rapid2storm" );
    field::XCoordinate( traits ).makeNode( topNode );
    field::YCoordinate( traits ).makeNode( topNode );
    field::FrameNumber( traits ).makeNode( topNode );
    field::Amplitude( traits ).makeNode( topNode );
    if ( traits.two_kernel_improvement_is_set )
        field::TwoKernelImprovement( traits ).makeNode( topNode );
    if ( traits.covariance_matrix_is_set )
        field::CovarianceMatrix( traits ).makeNode( topNode );

    XMLSTR str = topNode.createXMLString(0);
    *file << "# " << str << "\n";
    free( str );
}

Output::AdditionalData
LocalizationFile::announceStormSize(const Announcement &a) {
    ost::MutexLock lock(mutex);
    traits = a.traits;

    open();

    return AdditionalData().set_cluster_sources( localizationDepth > 0 );
}

Output::Result LocalizationFile::receiveLocalizations(const EngineResult &er) 
{
    ost::MutexLock lock(mutex);
    if ( er.number == 0 )
        (*file) << "# No localizations in image " << er.forImage.value() << std::endl;
    else
        for (int i = 0; i < er.number; i++)
            printFit(er.first[i], localizationDepth);
    if ( ! (*file) ) {
        std::cerr << "Warning: Writing localizations to "
                  << filename << " failed.\n";
        return RemoveThisOutput;
    }
    return KeepRunning;
}

void LocalizationFile::propagate_signal(Output::ProgressSignal s) {
    ost::MutexLock lock(mutex);
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
  filename(c.outputFile()), localizationDepth((c.traces()) ? 1 : 0) ,
  format( 4, Eigen::Raw, " ", " " )
{
    if ( filename == "" )
        throw std::runtime_error("No filename provided for "
                                 "localization output file");
}

LocalizationFile::~LocalizationFile() {}

}
}
