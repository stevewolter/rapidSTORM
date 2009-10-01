#define DSTORM_CARCONFIG_CPP
#include "CarConfig.h"
#include "foreach.h"
#include <CImgBuffer/Config.h>
#include "FilterSource.h"
#include "OutputFactory.h"

#include <cc++/thread.h>
#include <sstream>

#include <CImg.h>
#include "engine/LocalizationFileReader.h"
#include <CImgBuffer/AndorSIF.h>
#include <CImgBuffer/TIFF.h>
#ifdef HAVE_ANDOR_CAMERA
#include <CImgBuffer/AndorDirect.h>
#endif

#include <simparm/ChoiceEntry_Impl.hh>

using namespace std;

namespace dStorm {

class TreeRoot : public simparm::Object, public FilterSource
{
  public:
    TreeRoot( OutputFactory& tc_factory )
    : simparm::Object("EngineOutput", "dSTORM engine output")
    {
        initialize( tc_factory );
    }
    ~TreeRoot() { removeFromAllParents(); }

    TreeRoot* clone() const { return new TreeRoot(*this); }
    std::string getDesc() const { return desc(); }
};

CarConfig::CarConfig( OutputFactory& tc_factory ) 
: Object("Car", "Job options"),
  _inputConfig( new CImgBuffer::Config() ),
  _engineConfig( new dStorm::Config() ),
  outputRoot( new TreeRoot( tc_factory ) ),
  inputConfig(*_inputConfig),
  engineConfig(*_engineConfig),
  outputConfig(*outputRoot),
  outputBox("Output", "Output options"),
  configTarget("SaveConfigFile", "Store config used in computation in")
{
   setName("Car");

   configTarget.setUserLevel(Entry::Intermediate);
    inputConfig.inputMethod.addChoice( 
        new LocalizationFileReader::Config( inputConfig ) );
    inputConfig.inputMethod.addChoice( 
        new CImgBuffer::AndorSIF::Config<StormPixel>( inputConfig ) );
    inputConfig.inputMethod.addChoice( 
        new CImgBuffer::TIFF::Config<StormPixel>( inputConfig ) );
#ifdef HAVE_ANDOR_CAMERA
    inputConfig.inputMethod.addChoice( 
        new CImgBuffer::AndorDirect::Config( inputConfig ) );
#endif
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
  outputConfig(*outputRoot),
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
   outputBox.push_back( outputConfig );
   push_back( inputConfig );
   push_back( engineConfig );
   push_back( outputBox );
   push_back( configTarget );
}

}
