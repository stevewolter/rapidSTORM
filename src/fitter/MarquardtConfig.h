#ifndef DSTORM_FITTER_MARQUARDCONFIG_H
#define DSTORM_FITTER_MARQUARDCONFIG_H

#include <simparm/Set.hh>
#include <simparm/NumericEntry.hh>

namespace dStorm {
namespace fitter {

class MarquardtConfig
: public simparm::Set 
{
  public:
    MarquardtConfig(std::string name, std::string desc);
    ~MarquardtConfig();
    void registerNamedEntries();

    /** Start value for Marquardt lambda. */
    simparm::DoubleEntry marquardtStartLambda;
    /** Maximum number of iteration steps in fitting process. */
    simparm::UnsignedLongEntry maximumIterationSteps;
    /** Maximum length of negligibly short iteration step. */
    simparm::DoubleEntry negligibleStepLength;
    /** Number of successive negligibly short steps indicating fit
    *  success. */
    simparm::UnsignedLongEntry successiveNegligibleSteps;
};

}
}

#endif
