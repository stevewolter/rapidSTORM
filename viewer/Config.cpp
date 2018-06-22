#include "viewer/Config.h"
#include "viewer/ColourSchemeFactory.h"
#include "viewer/colour_schemes/decl.h"

namespace dStorm {
namespace viewer {

Config::Config()
: showOutput("ShowOutput", "Display dSTORM result image", true),
  outputFile("ToFile", "Save image to", ".png"),
  histogramPower("HistogramPower", "Extent of histogram normalization", 0.3),
  top_cutoff("IntensityCutoff", "Intensity cutoff", 1.0),
  colourScheme("ColourScheme", "Colour palette for display"),
  invert("InvertColours", "Invert colours", false),
  save_with_key("SaveWithKey", "Save output image with key", true),
  save_scale_bar("SaveScaleBar", "Save output image with scale bar", true),
  close_on_completion("CloseOnCompletion", 
                      "Close display on job completion", true),
  scale_bar_length("ScaleBarLength", "Length of scale bar", 5 * boost::units::si::micrometer)
{
    outputFile.set_user_level(simparm::Beginner);
    scale_bar_length.set_user_level( simparm::Intermediate );
    showOutput.set_user_level(simparm::Beginner);

    histogramPower.min = (0);
    histogramPower.max = (1);
    histogramPower.set_user_level(simparm::Beginner);

    top_cutoff.min = 0;
    top_cutoff.max = 1.0;
    top_cutoff.set_user_level( simparm::Expert );
    top_cutoff.setHelp( "Maximum displayed intensity as a fraction of the "
       "maximum found intensity" );

    colourScheme.addChoice( colour_schemes::make_hot_factory() );
    colourScheme.addChoice( colour_schemes::make_greyscale_factory() );
    colourScheme.addChoice( colour_schemes::make_colored_factory() );
    colourScheme.addChoice( colour_schemes::make_coordinate_factory() );

    close_on_completion.set_user_level(simparm::Debug);
    save_with_key.set_user_level(simparm::Intermediate);
    save_scale_bar.set_user_level(simparm::Intermediate);
    invert.set_user_level( simparm::Intermediate );

    outputFile.setHelpID( "#Viewer_ToFile" );
    showOutput.setHelpID( "#Viewer_ShowOutput" );
    colourScheme.setHelpID( "#Viewer_ColorScheme" );
    invert.setHelpID( "#Viewer_InvertColors" );
    top_cutoff.setHelpID( "#Viewer_TopCutoff" );
}

void Config::attach_ui( simparm::NodeHandle n ) {
    listening[0] = colourScheme.value.notify_on_value_change( boost::ref(some_value_changed) );
    listening[1] = invert.value.notify_on_value_change( boost::ref(some_value_changed) );

    outputFile.attach_ui(n);
    save_with_key.attach_ui(n);
    save_scale_bar.attach_ui(n);
    showOutput.attach_ui(n);

    DensityMapConfig::attach_ui( n );
    histogramPower.attach_ui(n);
    top_cutoff.attach_ui(n);
    colourScheme.attach_ui(n);
    invert.attach_ui(n);
    close_on_completion.attach_ui(n);
    scale_bar_length.attach_ui(n);
}

void Config::backend_needs_changing( simparm::BaseAttribute::Listener l ) {
    DensityMapConfig::backend_needs_changing( l );
    some_value_changed.connect( l );

    for ( simparm::ManagedChoiceEntry<ColourSchemeFactory>::iterator i = colourScheme.begin(); i != colourScheme.end(); ++i)
        i->add_listener( l );
}

Config::~Config() {}

}
}

namespace simparm { template class Entry< dStorm::viewer::Config::CropBorder >; }
