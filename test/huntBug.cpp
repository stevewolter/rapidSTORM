#include "ColorImage.h"
#include <dStormUtils/Driver.h>
#include <foreach.h>
#include <iomanip>

#include "SpotRegion.h"
#include "dStorm/GaussFitter.h"

using namespace dStorm;
using namespace dStormUtils;
using namespace cimg_library;
using namespace std;

#define SQ(x) ((x)*(x))

static double findX = 148.60 - 1, findY = 174.00 - 1;

bool equality(double val1, double val2, double r) {
   return (val1 > val2-r && val1 < val2+r);
}
void after_fit_hook(int, const Localization &fit, const Image &i) {
   if (fit.isGood() && 
       equality(fit.getPreciseX(), findX, 0.05) &&
       equality(fit.getPreciseY(), findY, 0.05))
   {
#ifdef GAUSSIAN_FIT_INFO
      GaussFitter::GaussFit& f = (GaussFitter::GaussFit&)fit;
      cout << f.amplitude << " " << f.shift << endl;
#endif
      ColorImage ci(i);
      ci.mark(fit);
      CImgDisplay d(768,768,"",1);
      d << ci;
      while (! d.is_key()) d.wait();
   }
}

int main(int argc, char *argv[]) {
#ifdef DSTORMENGINE_AFTER_FIT_HOOK
   Engine::after_fit_hook = after_fit_hook;
#endif
   Driver::Config config;
   config.amplitude_threshold = 8;
   config.endImage = 435;
   config.sigma_x_sq = 5.06;
   config.sigma_y_sq = 4.16;
   config.inputFile = "/media/scratch/Steve/Lokalisationsgenauigkeit "
      "Alexa647-Antibody/080627 - Teil II (besser)/Alexa647-Ak - Mito"
      "-Oberflaeche- 22mW 647nm, 5mW 514nm - 2000 - 7.sif";
   config.stderr_progress = true;
   config.readConfig(argc, argv);

   Driver(config).drive();
   return 0;
}
