#include "debug.h"
#include "outputs/LocalizationFile.h"

#include <memory>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <iomanip>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/units/Eigen/Array>
#include <boost/bind/bind.hpp>

#include <simparm/FileEntry.h>
#include <simparm/Message.h>

#include "localization/Traits.h"
#include "output/Output.h"
#include "output/FileOutputBuilder.h"
#include "localization_file/field.h"

using namespace std;

namespace dStorm {
namespace localization_file {
namespace writer {

class Config {
  public:
    output::BasenameAdjustedFileEntry outputFile;
    simparm::BoolEntry xyztI;

    static std::string get_name() { return "Table"; }
    static std::string get_description() { return "Localizations file"; }
    static simparm::UserLevel get_user_level() { return simparm::Beginner; }

    void attach_ui( simparm::NodeHandle at ) {
        outputFile.attach_ui( at );
        xyztI.attach_ui( at );
    }
    Config();

    bool can_work_with(output::Capabilities cap) { 
        return true; 
    }
};

class Output : public output::Output {
  private: 
    std::string filename;
    std::auto_ptr<std::ofstream> fileKeeper;
    std::ostream *file;
    input::Traits<Localization> traits;

    std::auto_ptr< Field > field;
    bool xyztI_format;
    simparm::NodeHandle current_ui;

    void open();
    template <int Field> void make_fields();
    void output( const Localization& );
    void store_results_( bool success );
    void attach_ui_( simparm::NodeHandle at ) { current_ui = at; }

  public:
    Output(const Config&);
    ~Output();

    AdditionalData announceStormSize(const Announcement &a);
    RunRequirements announce_run(const RunAnnouncement&);
    void receiveLocalizations(const EngineResult&);

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames)
    { 
        insert_filename_with_check( filename, present_filenames ); 
    }
};

Config::Config() 
: outputFile("ToFile", "Write localizations to", ".txt"),
  xyztI("XYZTI",   "Output only Malk fields (x,y,z,t,I)", false)
{
    outputFile.setHelp(
        "If given, this parameters determines a file to which "
        "the raw fit data will be written. The output is one "
        "line per fit, with X- and Y-coordinate, image number "
        "and fit amplitude, fields separated by spaces.");
    outputFile.set_user_level(simparm::Beginner);
    outputFile.setHelpID( "Table_ToFile" );

    xyztI.setHelp( 
        "Output only the most common subset of the possible information. "
        "The X,Y and Z coordinates will be displayed along with the frame "
        "number and the intensity, but no other columns. If no Z information "
        "is present, no output will be produced.");
}

void Output::open() {
    if ( filename != "-" ) {
        fileKeeper.reset( new ofstream( filename.c_str(), 
                                    ios_base::out | ios_base::trunc ) );
        file = fileKeeper.get();
    } else
        file = &cout;

    if ( xyztI_format )
        field = Field::construct_xyztI(traits);
    else
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
    if ( ! traits.in_sequence && er.empty() )
        (*file) << "# No localizations in image " << er.forImage.value() << std::endl;
    else
        std::for_each( er.begin(), er.end(), 
            boost::bind(&Output::output, this, _1) ); 
    if ( ! (*file) ) {
        simparm::Message m("Unable to write localizations file",
            "Writing localizations to " + filename + " failed.",
            simparm::Message::Warning );
        m.send( current_ui );
        file = NULL;
        fileKeeper.reset();
    }
}

Output::RunRequirements Output::announce_run(const RunAnnouncement&) {
    fileKeeper.reset(NULL);
    open();
    return Output::RunRequirements();
}

void Output::store_results_( bool ) {
    if ( file ) file->flush();
    if (fileKeeper.get() != NULL) fileKeeper->close();
    fileKeeper.reset(NULL);
    file = NULL;
}

Output::Output(const Config &c) 
: filename(c.outputFile()),
  file(NULL),
  xyztI_format( c.xyztI() )
{
    if ( filename == "" )
        throw std::runtime_error("No filename provided for "
                                 "localization output file");
}

Output::~Output() {}

std::auto_ptr<output::OutputSource> create() {
    return std::auto_ptr<output::OutputSource>( new output::FileOutputBuilder<Config,Output>() );
}

}
}
}
