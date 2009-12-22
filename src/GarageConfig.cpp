#include "GarageConfig.h"
#include <dStorm/output/OutputSource.h>
#include <dStorm/output/FilterSource.h>
#include <engine/Car.h>

#include "debug.h"

namespace dStorm {

using namespace output;
using namespace simparm;

GarageConfig::GarageConfig(const MasterConfig::Ptr& master) throw()
: 
  master( master ),
  externalControl("TwiddlerControl", "Enable stdin/out control interface"),
  showTransmissionTree("ShowTransmissionTree", 
                       "Output tree view of transmissions"),
  run("Run", "Run")
{
    DEBUG("Constructing GarageConfig");
    carConfig.reset( new dStorm::engine::CarConfig() );
    DEBUG("Built CarConfig, adding modules");
    master->add_modules( *carConfig );
    DEBUG("Added modules");

    externalControl = false;
    externalControl.setUserLevel(simparm::Entry::Expert);

    run.setHelp("Whenever this trigger is triggered or the button "
                "clicked, the dStorm engine will be run with the "
                "current parameters.");
    run.setUserLevel(simparm::Entry::Beginner);

    carConfig->inputConfig.basename.addChangeCallback(*this);

    registerNamedEntries();
    DEBUG("Constructed GarageConfig");
}

GarageConfig::GarageConfig(const GarageConfig &c) throw()
: simparm::Node::Callback(),
  master(master),
  carConfig(c.carConfig->clone()),
  externalControl(c.externalControl),
  showTransmissionTree(c.showTransmissionTree),
  run(c.run)
{
   carConfig->inputConfig.basename.addChangeCallback(*this);
   registerNamedEntries();
}

GarageConfig::~GarageConfig() {
   DEBUG("Unregistering nodes of garage config");
   master->thread_safely_erase_node( *carConfig );
   DEBUG("Unregistered nodes of garage config");
}

static void printTC( const output::OutputSource& src, int indent ) {
    const std::string& name = src.getNode().getName();
    if ( indent < 2 )
        std::cout << ((indent)?" ": name ) << "\n";
    else {
        std::cout << std::string(indent-2, ' ') << "|-" << name << "\n";
    }
    const FilterSource* fwd = dynamic_cast<const FilterSource*>(&src);
    if (fwd != NULL) {
        for (FilterSource::const_iterator
                            i = fwd->begin(); i != fwd->end(); i++)
            printTC(**i, indent+2);
    }
}

void GarageConfig::operator()(Node& src, Cause cause, Node *) throw() {
    if ( &src == &carConfig->inputConfig.basename &&
                cause == ValueChanged && externalControl() ) 
    {
        bool appended = false;
        dStorm::output::OutputSource::BasenameResult r;
        std::string basename = carConfig->inputConfig.basename();
        avoid_auto_filenames.clear();
        do {
            if ( carConfig->inputConfig.inputFile() != "" )
                avoid_auto_filenames
                    .insert( carConfig->inputConfig.inputFile() );

            r = carConfig->outputSource.set_output_file_basename
                    ( basename, avoid_auto_filenames );
            if ( !appended ) 
                basename += 'a'; 
            else 
                basename[ basename.size()-1 ]++;
            appended = true;
        } while ( r == dStorm::output::OutputSource::Basename_Conflicted );
    } else if ( &src == &showTransmissionTree.value && 
                showTransmissionTree.triggered() )
    {
        showTransmissionTree.untrigger();
        printTC( carConfig->outputSource, 0 );
        exit(0);
    } else if ( &src == &run.value && cause == ValueChanged &&
                run.triggered() ) 
    {
        run.untrigger();
        DEBUG("Running job");
        std::auto_ptr<engine::Car> car( 
            new engine::Car(master, *carConfig) );
        car->detach();
        car.release();
    } else if ( &src == &externalControl.value && cause == ValueChanged
                && externalControl.triggered() )
    {
        externalControl.untrigger();
        DEBUG("Reading control stream");
        master->read_input( std::cin );
        engine::Car::terminate_all_Car_threads();
    }
}


void GarageConfig::registerNamedEntries() throw() {
    carConfig->push_back( run );

    master->thread_safely_register_node( *carConfig );

    receive_changes_from( showTransmissionTree.value );
    receive_changes_from( run.value );
    receive_changes_from( externalControl.value );
}

void GarageConfig::set_input_file( const std::string& s ) {
    carConfig->inputConfig.inputFile = s;
}
void GarageConfig::run_job() {
    run.trigger();
}

std::auto_ptr<simparm::Set> GarageConfig::make_set()
{
    std::auto_ptr<simparm::Set> s( new simparm::Set("dSTORM", "") );
    s->push_back( *carConfig );
    s->push_back( externalControl );
    s->push_back( showTransmissionTree );
    return s;
}

}
