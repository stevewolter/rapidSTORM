#define DSTORM_CARCONFIG_CPP
#include "CarConfig.h"
#include <dStorm/input/Config.h>
#include <dStorm/output/FilterSource.h>
#include <dStorm/output/SourceFactory.h>

#include <dStorm/helpers/thread.h>
#include <sstream>

#include <simparm/ChoiceEntry_Impl.hh>

using namespace std;

namespace dStorm {
namespace engine {

class CarConfig::TreeRoot : public simparm::Object, public output::FilterSource
{
    output::Config* my_config;
  public:
    TreeRoot()
    : simparm::Object("EngineOutput", "dSTORM engine output")
    {
        output::Config config;
        output::FilterSource::initialize( config );
        assert( getFactory() != NULL );
        assert( dynamic_cast<output::Config*>(getFactory()) != NULL );
        my_config = dynamic_cast<output::Config*>(getFactory());
    }
    TreeRoot( const TreeRoot& other )
    : simparm::Object(other),
      output::FilterSource(other)
    {
        output::FilterSource::initialize( *other.my_config );
        my_config = dynamic_cast<output::Config*>(getFactory());
    }
    ~TreeRoot() { removeFromAllParents(); }

    TreeRoot* clone() const { return new TreeRoot(*this); }
    std::string getDesc() const { return desc(); }
    output::Config &root_factory() { return *my_config; }
};

CarConfig::CarConfig() 
: Object("Car", "Job options"),
  _inputConfig( new input::Config() ),
  _engineConfig( new engine::Config() ),
  outputRoot( new TreeRoot() ),
  inputConfig(*_inputConfig),
  engineConfig(*_engineConfig),
  outputSource(*outputRoot),
  outputConfig(outputRoot->root_factory()),
  outputBox("Output", "Output options"),
  configTarget("SaveConfigFile", "Store config used in computation in")
{
   setName("Car");
   configTarget.setUserLevel(simparm::Entry::Intermediate);

    registerNamedEntries();
}

CarConfig::CarConfig(const CarConfig &c) 
: Node(c),
  simparm::Object(c),
  _inputConfig(c.inputConfig.clone()),
  _engineConfig(c.engineConfig.clone()),
  outputRoot(c.outputRoot->clone()),
  inputConfig(*_inputConfig),
  engineConfig(*_engineConfig),
  outputSource(*outputRoot),
  outputConfig(outputRoot->root_factory()),
  outputBox(c.outputBox),
  configTarget(c.configTarget)
{
    registerNamedEntries();
}

CarConfig::~CarConfig() {
    outputRoot.reset( NULL );
    _engineConfig.reset( NULL );
    _inputConfig.reset( NULL );
}

void CarConfig::registerNamedEntries() {
   outputBox.push_back( *outputRoot );
   push_back( inputConfig );
   push_back( engineConfig );
   push_back( outputBox );
   push_back( configTarget );
}

}
}
