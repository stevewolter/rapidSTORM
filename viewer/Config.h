#ifndef DSTORM_VIEWER_CONFIG_H
#define DSTORM_VIEWER_CONFIG_H

#include "Config_decl.h"

#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ManagedChoiceEntry.hh>
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

class Config {
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

private:
    dStorm::default_on_copy< boost::signals2::signal<void()> > some_value_changed;
    simparm::BaseAttribute::ConnectionStore listening[6];

public:
    Config();
    ~Config();

    void attach_ui( simparm::NodeHandle at );
    void backend_needs_changing( simparm::BaseAttribute::Listener );
    static bool can_work_with(output::Capabilities) { return true; }

    static std::string get_name() { return "Image"; }
    static std::string get_description() { return "Image display"; }
    
    CropBorder crop_border() const;
};

}
}

#endif
