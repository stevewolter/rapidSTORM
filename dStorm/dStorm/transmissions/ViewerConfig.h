#ifndef DSTORM_VIEWER_CONFIG_H
#define DSTORM_VIEWER_CONFIG_H

#include <dStorm/transmissions/Viewer.h>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {

class Viewer::_Config : public simparm::Object {
  public:

    simparm::BoolEntry showOutput;
    simparm::FileEntry outputFile;
    simparm::DoubleEntry res_enh;
    simparm::UnsignedLongEntry refreshCycle;
    simparm::DoubleEntry histogramPower;
    simparm::LongEntry zoom;
    simparm::ChoiceEntry colourScheme;
    simparm::DoubleEntry hue, saturation;
    simparm::BoolEntry invert, close_on_completion;

    _Config();

    void registerNamedEntries();
};

}

#endif
