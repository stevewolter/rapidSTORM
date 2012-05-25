#include "Config.h"
#include "ColourScheme.h"
#include "debug.h"
#include <simparm/ChoiceEntry_Impl.h>
#include "colour_schemes/decl.h"

namespace dStorm {
namespace viewer {

Config::Config()
: showOutput("ShowOutput", "Display dSTORM result image"),
  density_matrix_given("SaveDensityMatrix", "Save density matrix", false ),
  outputFile("ToFile", "Save image to", ".png"),
  density_matrix("DensityMatrixFile", "Save density matrix to", "-density.txt"),
  histogramPower("HistogramPower", "Extent of histogram normalization", 0.3),
  top_cutoff("IntensityCutoff", "Intensity cutoff", 1.0),
  colourScheme("ColourScheme", "Colour palette for display"),
  invert("InvertColours", "Invert colours", false),
  save_with_key("SaveWithKey", "Save output image with key", true),
  save_scale_bar("SaveScaleBar", "Save output image with scale bar", true),
  close_on_completion("CloseOnCompletion", 
                      "Close display on job completion"),
  border("Border", "Width of border to chop", CropBorder::Constant(0 * camera::pixel)),
  scale_bar_length("ScaleBarLength", "Length of scale bar", 5 * boost::units::si::micrometer)
{
    DEBUG("Building Viewer Config");

    outputFile.set_user_level(simparm::Beginner);

    density_matrix_given.set_user_level(simparm::Expert);
    density_matrix.set_user_level(simparm::Expert);
    density_matrix.setHelp( "Save a text file with the unnormalized intensities of the result image" );

    scale_bar_length.set_user_level( simparm::Intermediate );

    showOutput.set_user_level(simparm::Beginner);

    histogramPower.min = (0);
    histogramPower.max = (1);
    /* This level is reset in carStarted() */
    histogramPower.set_user_level(simparm::Expert);
    top_cutoff.min = 0;
    top_cutoff.max = 1.0;
    top_cutoff.set_user_level( simparm::Expert );
    top_cutoff.setHelp( "Maximum displayed intensity as a fraction of the "
       "maximum found intensity" );

#define DISC_INSTANCE(Scheme) \
    colourScheme.addChoice( ColourScheme::config_for<Scheme>() );
#include "colour_schemes/instantiate.h"

    close_on_completion.set_user_level(simparm::Debug);
    save_with_key.set_user_level(simparm::Intermediate);
    save_scale_bar.set_user_level(simparm::Intermediate);
    border.set_user_level(simparm::Intermediate);
    invert.set_user_level( simparm::Intermediate );

    outputFile.setHelpID( "#Viewer_ToFile" );
    showOutput.setHelpID( "#Viewer_ShowOutput" );
    colourScheme.setHelpID( "#Viewer_ColorScheme" );
    invert.setHelpID( "#Viewer_InvertColors" );
    top_cutoff.setHelpID( "#Viewer_TopCutoff" );

    DEBUG("Built Viewer Config");
}

void Config::attach_ui( simparm::NodeHandle n ) {
    listening[3] = colourScheme.value.notify_on_value_change( boost::ref(some_value_changed) );
    listening[4] = invert.value.notify_on_value_change( boost::ref(some_value_changed) );
    listening[5] = border.value.notify_on_value_change( boost::ref(some_value_changed) );

   outputFile.attach_ui(n);
   save_with_key.attach_ui(n);
   save_scale_bar.attach_ui(n);
   showOutput.attach_ui(n);
   density_matrix_given.attach_ui(n);
   density_matrix.attach_ui(n);

   binned_dimensions.attach_ui(n);
   histogramPower.attach_ui(n);
   top_cutoff.attach_ui(n);
   colourScheme.attach_ui(n);
   invert.attach_ui(n);
   close_on_completion.attach_ui(n);
   border.attach_ui(n);
   scale_bar_length.attach_ui(n);
}

void Config::backend_needs_changing( simparm::BaseAttribute::Listener l ) {
    some_value_changed.connect( l );
    binned_dimensions.add_listener(l);

    for ( simparm::ManagedChoiceEntry<ColourScheme>::iterator i = colourScheme.begin(); i != colourScheme.end(); ++i)
        i->add_listener( l );
}

Config::~Config() {}

Config::CropBorder
Config::crop_border() const
{
    CropBorder rv = border();
    if ( ! binned_dimensions.is_3d() )
        rv[2] = 0 * camera::pixel;
    return rv;
}

}
}

namespace simparm { template class Entry< dStorm::viewer::Config::CropBorder >; }
