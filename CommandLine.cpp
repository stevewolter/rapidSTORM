#include "debug.h"

#include <simparm/Message.h>
#include "CommandLine.h"
#include <vector>
#include <string>
#include <dStorm/Config.h>
#include "JobStarter.h"
#include <fstream>
#include <dStorm/output/OutputSource.h>
#include <dStorm/output/FilterSource.h>
#include "InputStream.h"
#include "ModuleLoader.h"
#include <simparm/cmdline_ui/RootNode.h>
#include <dStorm/display/Manager.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include "job/Config.h"
#include "JobStarter.h"
#include "wx_ui/Launcher.h"
#include "simparm/text_stream/Launcher.h"

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

void CommandLine::parse( int argc, char *argv[] ) {
    boost::shared_ptr< simparm::cmdline_ui::RootNode > argument_parser( new simparm::cmdline_ui::RootNode() );
    cmdline_ui = argument_parser;

    int shift = find_config_file(argc,argv);
    argc -= shift;
    argv += shift;

    TransmissionTreePrinter printer(config);
    simparm::text_stream::Launcher tw_launcher(config, true), sp_launcher(config, false);
    simparm::wx_ui::Launcher wx_launcher(config);
    JobStarter starter( argument_parser, config );
    simparm::TriggerEntry help("Help", "Help");

    config.attach_ui( cmdline_ui );
    printer.attach_ui( cmdline_ui );
    tw_launcher.attach_ui( cmdline_ui );
    sp_launcher.attach_ui( cmdline_ui );
    wx_launcher.attach_ui( cmdline_ui );
    starter.attach_ui( cmdline_ui );
    help.attach_ui( cmdline_ui );

    help.value.notify_on_value_change( boost::bind( &simparm::cmdline_ui::RootNode::print_help, argument_parser.get() ) )->release();

    argument_parser->parse_command_line( argc, argv );
}

int CommandLine::find_config_file( int argc, char* argv[] ) {
    if ( argc > 1 && std::string(argv[1]) == "--no-config" ) return 1;
    int shift = 0;
    DEBUG("Checking for relevant environment variables");
    const char *home = getenv("HOME"),
               *homedrive = getenv("HOMEDRIVE"),
               *homepath = getenv("HOMEPATH");
    bool have_file = false;
    DEBUG("Checking for command line config file");
    while ( argc > 2 && std::string(argv[1]) == "--config" ) {
        bool successfully_opened = deserialize(config, std::string(argv[2]), cmdline_ui);
        have_file = have_file || successfully_opened;
        if ( !successfully_opened )
            DEBUG("Skipped unreadable config file '" << argv[2] << "'");
        argc -= 2;
        shift += 2;
        argv[2] = argv[0];
        argv = argv + 2;
    }
    DEBUG("Checking for home directory config file");
    if ( !have_file && home != NULL )
        have_file = deserialize( config, std::string(home) + "/.dstorm", cmdline_ui);
    if ( !have_file && homedrive != NULL && homepath != NULL )
        have_file = deserialize( config, 
            std::string(homedrive) + std::string(homepath) + "/dstorm.txt", cmdline_ui);
    return shift;
}

CommandLine::CommandLine()
{
    add_modules( config );
    config.all_modules_loaded();
}
CommandLine::~CommandLine() {
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
    printNode( config.outputSource, 0 );
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
