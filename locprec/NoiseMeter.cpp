#include "NoiseMeter.h"

namespace locprec {

NoiseMeter::_Config::_Config()
: simparm::Object("NoiseMeter", "Histogram noise pixels"),
  adCorrection("ADCorrection", "A/D error correction factor", 8),
  outputFile("ToFile", "Output noise statistics to")
{ userLevel = simparm::Object::Expert; }
}
