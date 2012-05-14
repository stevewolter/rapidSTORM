#include "Config.h"
#include "ColourScheme.h"
#include "debug.h"
#include <simparm/ChoiceEntry_Impl.hh>
#include "colour_schemes/decl.h"
#include <simparm/Entry_Impl.hh>

namespace dStorm {
namespace viewer {

_Config::_Config()
: simparm::Object("Image", "Image display"),
  showOutput("ShowOutput", "Display dSTORM result image"),
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

    outputFile.setUserLevel(simparm::Object::Beginner);

    density_matrix_given.setUserLevel(simparm::Object::Expert);
    density_matrix.setUserLevel(simparm::Object::Expert);
    density_matrix.help = "Save a text file with the unnormalized intensities of the result image";

    scale_bar_length.setUserLevel( simparm::Object::Intermediate );

    showOutput.setUserLevel(simparm::Object::Beginner);

    histogramPower.min = (0);
    histogramPower.max = (1);
    /* This level is reset in carStarted() */
    histogramPower.setUserLevel(simparm::Object::Expert);
    top_cutoff.min = 0;
    top_cutoff.max = 1.0;
    top_cutoff.userLevel = simparm::Object::Expert;
    top_cutoff.help = "Maximum displayed intensity as a fraction of the "
       "maximum found intensity";

#define DISC_INSTANCE(Scheme) \
    colourScheme.addChoice( ColourScheme::config_for<Scheme>() );
#include "colour_schemes/instantiate.h"

    close_on_completion.setUserLevel(simparm::Object::Debug);
    save_with_key.setUserLevel(simparm::Object::Intermediate);
    save_scale_bar.setUserLevel(simparm::Object::Intermediate);
    border.setUserLevel(simparm::Object::Intermediate);
    invert.userLevel = simparm::Object::Intermediate;

    outputFile.helpID = "#Viewer_ToFile";
    showOutput.helpID = "#Viewer_ShowOutput";
    colourScheme.helpID = "#Viewer_ColorScheme";
    invert.helpID = "#Viewer_InvertColors";
    top_cutoff.helpID = "#Viewer_TopCutoff";

    DEBUG("Built Viewer Config");
}

void _Config::registerNamedEntries( simparm::Node& n ) {
   n.push_back(outputFile);
   n.push_back(save_with_key);
   n.push_back(save_scale_bar);
   n.push_back(showOutput);
   n.push_back(density_matrix_given);
   n.push_back(density_matrix);

   n.push_back(binned_dimensions);
   n.push_back(histogramPower);
   n.push_back(top_cutoff);
   n.push_back(colourScheme);
   n.push_back(invert);
   n.push_back(close_on_completion);
   n.push_back(border);
   n.push_back(scale_bar_length);
}

void _Config::add_listener( simparm::Listener& l ) {
    l.receive_changes_from( showOutput.value );
    binned_dimensions.add_listener(l);
    l.receive_changes_from( histogramPower.value );
    l.receive_changes_from( top_cutoff.value );
    l.receive_changes_from( colourScheme.value );
    l.receive_changes_from( invert.value );
    l.receive_changes_from( border.value );

    for ( simparm::ManagedChoiceEntry<ColourScheme>::iterator i = colourScheme.begin(); i != colourScheme.end(); ++i)
        i->add_listener( l );
}

_Config::~_Config() {}

_Config::CropBorder
_Config::crop_border() const
{
    CropBorder rv = border();
    if ( ! binned_dimensions.is_3d() )
        rv[2] = 0 * camera::pixel;
    return rv;
}

}
}
