#include "GarageConfig.h"
#include <dStorm/output/OutputSource.h>
#include <dStorm/output/FilterSource.h>
#include <engine/Car.h>
#include "doc/help/rapidstorm_help_file.h"
#include "ModuleLoader.h"

#include "debug.h"

namespace dStorm {

using namespace output;
using namespace simparm;

GarageConfig::GarageConfig()
: simparm::Node::Callback(simparm::Event::ValueChanged),
  help_file("help_file", dStorm::HelpFileName),
  run("Run", "Run")
{
    DEBUG("Constructing GarageConfig");
    carConfig.reset( new dStorm::engine::CarConfig() );
    DEBUG("Built CarConfig, adding modules");
    ModuleLoader::getSingleton().
        add_modules( *carConfig );
    DEBUG("Added modules");

    run.setHelp("Whenever this trigger is triggered or the button "
                "clicked, the dStorm engine will be run with the "
                "current parameters.");
    run.setUserLevel(simparm::Entry::Beginner);

    registerNamedEntries();
    DEBUG("Constructed GarageConfig");
}

#if 0
GarageConfig::GarageConfig(const GarageConfig &c)
: simparm::Node::Callback(simparm::Event::ValueChanged),
  carConfig(c.carConfig->clone()),
  help_file( c.help_file ),
  run(c.run)
{
   registerNamedEntries();
}
#endif

GarageConfig::~GarageConfig() {
   DEBUG("Terminating all car threads");
   engine::Car::terminate_all_Car_threads();

   DEBUG("Unregistering nodes of garage config");
   master.thread_safely_erase_node( *carConfig );
   carConfig.reset( NULL );
   DEBUG("Commencing destruction");
}

void GarageConfig::operator()(const simparm::Event& e)
{
    if ( &e.source == &run.value && run.triggered() ) {
        run.untrigger();
        try {
            DEBUG("Running job");
            std::auto_ptr<engine::Car> car( 
                new engine::Car(master, *carConfig) );
            car->detach();
            car.release();
        } catch ( const std::exception& e ) {
            std::cerr << "Starting job failed: " << e.what() << "\n";
        }
    }
}


void GarageConfig::registerNamedEntries() {
    carConfig->push_back( help_file );
    carConfig->push_back( run );

    master.thread_safely_register_node( *carConfig );

    receive_changes_from( run.value );
}

void GarageConfig::set_input_file( const std::string& s ) {
    carConfig->inputConfig.inputFile = s;
}
void GarageConfig::run_job() {
    run.trigger();
}

}
