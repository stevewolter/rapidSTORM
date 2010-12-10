#define DSTORM_CARCONFIG_CPP
#include "CarConfig.h"
#include "foreach.h"
#include <CImgBuffer/Config.h>

#include <cc++/thread.h>

using namespace std;

namespace dStorm {

#define NAME "Car"
#define DESC PACKAGE_VERSION

_CarConfig::_CarConfig() throw()
: ConfigObject(NAME, DESC),
  configTarget("SaveConfigFile", "Store config used in computation in"),
  stderr_progress("ProgressOnStderr", "Display progress bar on stderr"),
  printUsedSigma("PrintUsedSigmas", "Print value of sigma parameters used")
{
   setName("Car");

/*
 * TODO
   averageImage.setUserLevel(ConfigEntry::Intermediate);
*/
   configTarget.setUserLevel(ConfigEntry::Beginner);

   stderr_progress = true;

}

CarConfig::CarConfig( TransmissionSourceFactory& tc_factory ) throw() 
: ConfigObject(NAME, DESC),
  transmissionConfigStore( tc_factory.makeTransmissionSource("") ),
  transmissionConfig( *transmissionConfigStore )
{
    registerNamedEntries();
}

CarConfig::CarConfig(const CarConfig &c) throw() 
: ConfigObject(c),
  _CarConfig(c),
  transmissionConfigStore( c.transmissionConfig.clone() ),
  transmissionConfig( *transmissionConfigStore )
{
    registerNamedEntries();
}

void CarConfig::registerNamedEntries() throw() {
   push_back( inputConfig );
   push_back( engineConfig );
   push_back( transmissionConfig );
   push_back( configTarget );
   push_back( stderr_progress );
   push_back( printUsedSigma );
}

#if 0
TODO
void CarConfig::init() throw() {
   ConfigSet toInsert;
   dStorm::Config::registerEntries(toInsert);
   /* Show the engine choices on all input methods _but_ the
    * STM_choice, so the last. */
   for (int i = inputMethod.numChoices()-2; i >= 0; i--)
        foreach_const( j, vector<ConfigEntry*>, toInsert.getEntries() )
            inputMethod.showOnChoice(i, **j);
}
#endif

}
