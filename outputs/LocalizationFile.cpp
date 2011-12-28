#define DSTORM_LOCALIZATIONFILE_CPP
#include "debug.h"
#include "LocalizationFile.h"
#include <dStorm/localization/Traits.h>
#include <dStorm/output/Output.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <simparm/FileEntry.hh>
#include <simparm/Structure.hh>
#include <boost/ptr_container/ptr_vector.hpp>
#include <string.h>
#include <stdlib.h>
#include <boost/units/Eigen/Array>
#include <iomanip>
#include <boost/bind/bind.hpp>

#include <dStorm/localization_file/field.h>

using namespace std;

namespace dStorm {
namespace localization_file {
namespace writer {

class Output : public output::OutputObject {
  private: 
    std::string filename;
    std::auto_ptr<std::ofstream> fileKeeper;
    std::ostream *file;
    input::Traits<Localization> traits;

    std::auto_ptr< Field > field;

    void open();
    template <int Field> void make_fields();
    void output( const Localization& );

    class _Config;

  public:
    typedef simparm::Structure<_Config> Config;

    Output(const Config&);
    ~Output();
    Output* clone() const { 
        throw std::runtime_error(
            "LocalizationFile::clone not implemented"); }

    AdditionalData announceStormSize(const Announcement &a);
    RunRequirements announce_run(const RunAnnouncement&);
    void receiveLocalizations(const EngineResult&);
    void store_results();

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames)
    { 
        insert_filename_with_check( filename, present_filenames ); 
    }
};

class Output::_Config : public simparm::Object {
  protected:
    void registerNamedEntries() {
        push_back( outputFile );
        push_back( traces );
    }
  public:
    output::BasenameAdjustedFileEntry outputFile;
    simparm::BoolEntry traces;

    _Config();

    bool can_work_with(output::Capabilities cap) { 
        traces.viewable = cap.test( output::Capabilities::ClustersWithSources );
        return true; 
    }
};

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
    return AdditionalData();
}

void Output::output( const Localization& l ) {
    field->write( *file, l );
    *file << "\n";
}

void Output::receiveLocalizations(const EngineResult &er) 
{
    if ( file == NULL ) return;
    if ( er.empty() )
        (*file) << "# No localizations in image " << er.forImage.value() << std::endl;
    else
        std::for_each( er.begin(), er.end(), 
            boost::bind(&Output::output, this, _1) ); 
    if ( ! (*file) ) {
        std::cerr << "Warning: Writing localizations to "
                  << filename << " failed.\n";
        file = NULL;
        fileKeeper.reset();
    }
}

Output::RunRequirements Output::announce_run(const RunAnnouncement&) {
    fileKeeper.reset(NULL);
    open();
    return Output::RunRequirements();
}

void Output::store_results() {
    file->flush();
    if (fileKeeper.get() != NULL) fileKeeper->close();
    fileKeeper.reset(NULL);
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

std::auto_ptr<output::OutputSource> create() {
    return std::auto_ptr<output::OutputSource>( new output::FileOutputBuilder<Output>() );
}

}
}
}
