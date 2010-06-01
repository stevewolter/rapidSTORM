#include "Status.h"
#include "Config.h"

namespace dStorm {
namespace viewer {

Status::Status(const Config& config)
: reshow_output("ReshowOutput", "Show output"),
  tifFile( "ToFile", "Save image to", config.outputFile() ),
  save_with_key( config.save_with_key ),
  resolutionEnhancement( config.res_enh ),
  histogramPower( config.histogramPower ),
  save("SaveImage", "Save image")
{}

Status::~Status() {}

}
}
