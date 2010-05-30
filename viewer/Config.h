#ifndef DSTORM_VIEWER_CONFIG_H
#define DSTORM_VIEWER_CONFIG_H

#include "Config_decl.h"

#include <simparm/ChoiceEntry.hh>
#include <simparm/Structure.hh>
#include <dStorm/output/BasenameAdjustedFileEntry.h>
#include <simparm/NumericEntry.hh>
#include <dStorm/output/Output.h>

namespace dStorm {
namespace viewer {

class _Config : public simparm::Object {
  public:

    simparm::BoolEntry showOutput;
    output::BasenameAdjustedFileEntry outputFile;
    simparm::DoubleEntry res_enh;
    simparm::UnsignedLongEntry refreshCycle;
    simparm::DoubleEntry histogramPower;
    simparm::ChoiceEntry colourScheme;
    simparm::DoubleEntry hue, saturation;
    simparm::BoolEntry invert, save_with_key, close_on_completion;

    _Config();
    ~_Config();

    void registerNamedEntries();
    static bool can_work_with(output::Capabilities) { return true; }
};

}
}

#endif
