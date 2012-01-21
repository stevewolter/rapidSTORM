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
  outputFile("ToFile", "Save image to", ".png"),
  histogramPower("HistogramPower", "Extent of histogram normalization",
                 0.3),
  colourScheme("ColourScheme", "Colour palette for display"),
  invert("InvertColours", "Invert colours", false),
  save_with_key("SaveWithKey", "Save output image with key", true),
  save_scale_bar("SaveScaleBar", "Save output image with scale bar", true),
  close_on_completion("CloseOnCompletion", 
                      "Close display on job completion"),
  border("Border", "Width of border to chop", CropBorder::Constant(0 * camera::pixel))
{
    DEBUG("Building Viewer Config");

    outputFile.make_optional();
    outputFile.optional_given = true;
    outputFile.setUserLevel(simparm::Object::Beginner);

    showOutput.setUserLevel(simparm::Object::Beginner);

    histogramPower.min = (0);
    histogramPower.max = (1);
    /* This level is reset in carStarted() */
    histogramPower.setUserLevel(simparm::Object::Expert);

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

    DEBUG("Built Viewer Config");
}

void _Config::registerNamedEntries() {
   push_back(outputFile);
   push_back(save_with_key);
   push_back(save_scale_bar);
   push_back(showOutput);
   push_back(binned_dimensions);
   push_back(histogramPower);
   push_back(colourScheme);
   push_back(invert);
   push_back(close_on_completion);
   push_back(border);
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
