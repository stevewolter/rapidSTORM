#include "SpotMeter.h"

namespace locprec {

SpotMeter::_Config::_Config()
: simparm::Object("SpotMeter", 
                  "Histogram localization amplitudes"),
  targetFile("OutputFile", "Histogram target file"),
  modulus("BinSize", "Size for histogram bins") 
{ 
    userLevel = Expert;
    modulus.setMin(1); 
}

}
