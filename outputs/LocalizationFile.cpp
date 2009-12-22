#define DSTORM_LOCALIZATIONFILE_CPP
#include "LocalizationFile.h"
#include <string.h>
#include <dStorm/output/Trace.h>
#include "doc/help/context.h"

using namespace std;

namespace dStorm {
namespace output {

LocalizationFile::_Config::_Config() 
: simparm::Object("Table", "Localizations file"),
  outputFile("ToFile", "Write localizations to"),
  traces("Traces", "Print localizations seperated by traces")
{
    outputFile.setHelp(
        "If given, this parameters determines a file to which "
        "the raw fit data will be written. The output is one "
        "line per fit, with X- and Y-coordinate, image number "
        "and fit amplitude, fields separated by spaces.");
    outputFile.setUserLevel(simparm::Entry::Beginner);
    outputFile.default_extension = ".txt";
    outputFile.helpID = HELP_Table_ToFile;
}

static void printFit(const Localization &f, ostream &file,
    int localizationDepth) 
{
    if ( localizationDepth > 0 && f.has_source_trace() ) {
        file << "\n\n";
        const Trace& trace = f.get_source_trace();
        for (Trace::const_iterator i = trace.begin(); 
                                    i != trace.end(); i++)
            printFit(*i, file, 0);
    } else {
      file << f;
    }
}

void LocalizationFile::open() {
    if ( filename != "-" ) {
        fileKeeper.reset( new ofstream( filename.c_str(), 
                                    ios_base::out | ios_base::trunc ) );
        file = fileKeeper.get();
    } else
        file = &cout;
    *file << traits.dimx() << " " << traits.dimy() << " "
          << traits.imageNumber << " 0 0\n";
}

Output::AdditionalData
LocalizationFile::announceStormSize(const Announcement &a) {
    ost::MutexLock lock(mutex);
    traits = input::Traits<Localization>(a.traits);
    if ( a.traits.total_frame_count.is_set() )
        traits.imageNumber = *a.traits.total_frame_count;
    else
        traits.imageNumber = 0;

    open();

    return AdditionalData().set_cluster_sources( localizationDepth > 0 );
}

Output::Result LocalizationFile::receiveLocalizations(const EngineResult &er) 

{
    ost::MutexLock lock(mutex);
    for (int i = 0; i < er.number; i++)
        printFit(er.first[i], *file, localizationDepth);
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
  filename(c.outputFile()), localizationDepth((c.traces()) ? 1 : 0) 
{
    if ( filename == "" )
        throw std::runtime_error("No filename provided for "
                                 "localization output file");
}

LocalizationFile::~LocalizationFile() {}

}
}
