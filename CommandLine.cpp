#include "debug.h"

/* wxWidgets includes must come first, lest they conflict with -DBOOST_USE_WINDOWS_H */
#include "config_file.h"

#include <simparm/Message.h>
#include "CommandLine.h"
#include <vector>
#include <string>
#include "base/Config.h"
#include <fstream>
#include "output/OutputSource.h"
#include "output/FilterSource.h"
#include "InputStream.h"
#include <simparm/cmdline_ui/RootNode.h>
#include "display/Manager.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include "job/Config.h"
#include "simparm/wx_ui/Launcher.h"
#include "simparm/text_stream/Launcher.h"
#include "ui/serialization/deserialize.h"
#include "unit_tests.h"
#include <boost/filesystem/path.hpp>
#include "ModuleLoader.h"

namespace dStorm {

using namespace output;

class TransmissionTreePrinter 
: public simparm::TriggerEntry
{
    const job::Config &config;
    void printNode( const output::OutputSource&, int indent );
    void printTree();
    simparm::BaseAttribute::ConnectionStore listening;
  public:
    TransmissionTreePrinter(const job::Config&);
    void attach_ui( simparm::NodeHandle );
};

static void run_unit_tests( char* arg0, int& exit_status ) {
    char *argv[] = { arg0 };
    if ( ::run_unit_tests( 1, argv ) != EXIT_SUCCESS )
        exit_status = EXIT_FAILURE;
}

int parse_command_line( int argc, char *argv[] ) {
    boost::shared_ptr< simparm::cmdline_ui::RootNode > argument_parser( new simparm::cmdline_ui::RootNode() );
    simparm::NodeHandle cmdline_ui = argument_parser;

    int exit_status = EXIT_SUCCESS;
    std::string basename = boost::filesystem::path( argv[0] ).filename().string();
    if ( argc <= 1 ) {
        std::auto_ptr< JobConfig > config( new job::Config( false ) );
        simparm::wx_ui::Launcher wx_launcher( *config );
        wx_launcher.launch();
    } else {
        bool localization_job = false;
        std::vector<std::string> config_files;
        while ( argc > 2 ) {
            int n;
            if (argv[1] == std::string("--config")) {
                config_files.push_back(argv[2]);
                n = 2;
            } else if (argv[1] == std::string("--ReplayJob")) {
                localization_job = true;
                n = 1;
            } else {
                break;
            }
            argv[n] = argv[0];
            argc -= n;
            argv += n;
        }

        std::auto_ptr< JobConfig > config_ptr( new job::Config( localization_job ) );
        JobConfig& config = *config_ptr;
        shell::JobFactory with_job_starter( config_ptr, cmdline_ui );

        for (const std::string& config_file : config_files) {
            with_job_starter.deserialize( config_file );
        }

        TransmissionTreePrinter printer( static_cast<const job::Config&>(config) );
        simparm::text_stream::Launcher tw_launcher(config, true), sp_launcher(config, false);
        simparm::wx_ui::Launcher wx_launcher(config);
        simparm::TriggerEntry help("Help", "Help"), unit_tests("UnitTests", "Run unit tests");

        with_job_starter.attach_ui( cmdline_ui );
        printer.attach_ui( cmdline_ui );
        tw_launcher.attach_ui( cmdline_ui );
        sp_launcher.attach_ui( cmdline_ui );
        wx_launcher.attach_ui( cmdline_ui );
        help.attach_ui( cmdline_ui );
        unit_tests.attach_ui( cmdline_ui );

        help.value.notify_on_value_change( boost::bind( &simparm::cmdline_ui::RootNode::print_help, argument_parser.get() ) )->release();
        unit_tests.value.notify_on_value_change( boost::bind( &dStorm::run_unit_tests, argv[0], boost::ref(exit_status) ) )->release();

        argument_parser->parse_command_line( argc, argv );
    }
    return exit_status;
}

TransmissionTreePrinter::TransmissionTreePrinter
    ( const job::Config& c )
: simparm::TriggerEntry("ShowTransmissionTree", 
                        "Print tree view of outputs"),
  config(c)
{
}

void TransmissionTreePrinter::attach_ui( simparm::NodeHandle n ) {
    simparm::TriggerEntry::attach_ui(n);
    listening = value.notify_on_value_change( 
        boost::bind( &TransmissionTreePrinter::printTree, this ) );
}

void TransmissionTreePrinter::printTree()
{
    printNode( config.output_tree(), 0 );
}

void TransmissionTreePrinter::printNode( 
    const output::OutputSource& src, int indent 
) {
    const std::string& name = src.getName();
    if ( indent < 2 )
        std::cout << ((indent)?" ": name ) << "\n";
    else {
        std::cout << std::string(indent-2, ' ') << "|-" << name << "\n";
    }
    const FilterSource* fwd = dynamic_cast<const FilterSource*>(&src);
    if (fwd != NULL) {
        fwd->for_each_suboutput( boost::bind( &TransmissionTreePrinter::printNode, boost::ref(*this), _1, indent+2) );
    }
}

}
