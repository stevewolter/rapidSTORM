#ifndef DSTORM_VIEWER_CONFIG_H
#define DSTORM_VIEWER_CONFIG_H

#include "viewer/DensityMapConfig.h"

#include "simparm/ChoiceEntry.h"
#include "simparm/ManagedChoiceEntry.h"
#include "output/BasenameAdjustedFileEntry.h"
#include "simparm/Entry.h"
#include "output/Output.h"
#include "units/microlength.h"

#include "UnitEntries/PixelEntry.h"
#include "viewer/ColourSchemeFactory.h"
#include "viewer/Image.h"

namespace dStorm {
namespace viewer {

class Config : public DensityMapConfig {
public:
    simparm::BoolEntry showOutput;
    output::BasenameAdjustedFileEntry outputFile;
    simparm::Entry<double> histogramPower, top_cutoff;
    simparm::ManagedChoiceEntry<ColourSchemeFactory> colourScheme;
    simparm::BoolEntry invert, save_with_key, save_scale_bar, close_on_completion;
    simparm::Entry< boost::units::quantity<boost::units::si::microlength> > scale_bar_length;

private:
    dStorm::default_on_copy< boost::signals2::signal<void()> > some_value_changed;
    simparm::BaseAttribute::ConnectionStore listening[2];

public:
    Config();
    ~Config();

    void attach_ui( simparm::NodeHandle at );
    void backend_needs_changing( simparm::BaseAttribute::Listener );
    static bool can_work_with(output::Capabilities) { return true; }

    static std::string get_name() { return "Image"; }
    static std::string get_description() { return "Image display"; }
    static simparm::UserLevel get_user_level() { return simparm::Beginner; }
};

}
}

#endif
