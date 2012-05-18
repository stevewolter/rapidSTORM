#ifndef DSTORM_VIEWER_CONFIG_H
#define DSTORM_VIEWER_CONFIG_H

#include "Config_decl.h"

#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ManagedChoiceEntry.hh>
#include <simparm/Structure.hh>
#include <dStorm/output/BasenameAdjustedFileEntry.h>
#include <simparm/Entry.hh>
#include <dStorm/output/Output.h>
#include <dStorm/units/microlength.h>

#include <dStorm/UnitEntries/PixelEntry.h>
#include <dStorm/outputs/BinnedLocalizations_strategies_config.h>
#include "ColourScheme.h"
#include "Image.h"

namespace dStorm {
namespace viewer {

class _Config : public simparm::Object {
  public:
    typedef Eigen::Matrix< boost::units::quantity<boost::units::camera::length,int>, 3, 1 >
        CropBorder;

    simparm::BoolEntry showOutput, density_matrix_given;
    output::BasenameAdjustedFileEntry outputFile, density_matrix;
    outputs::DimensionSelector<Im::Dim> binned_dimensions;
    simparm::Entry<double> histogramPower, top_cutoff;
    simparm::ManagedChoiceEntry<ColourScheme> colourScheme;
    simparm::BoolEntry invert, save_with_key, save_scale_bar, close_on_completion;
    simparm::Entry< CropBorder > border;
    simparm::Entry< boost::units::quantity<boost::units::si::microlength> > scale_bar_length;

    _Config();
    ~_Config();

    void registerNamedEntries() { attach_ui_elements(*this); }
    void attach_ui_elements( simparm::Node& at );
    void add_listener( simparm::Listener& );
    static bool can_work_with(output::Capabilities) { return true; }
    
    CropBorder crop_border() const;
};

}
}

#endif
