#include <dStorm/SIFLoader.h>
#include <foreach.h>
#include <dStorm/Driver.h>
#include "ColorImage.h"
#include <queue>
#include <dStorm/GaussFitter.h>

using namespace cimg_library;
using namespace std;
using namespace dStormEngine;
using namespace dStormUtils;

typedef GaussFitter::GaussFit GaussFit;

static queue<GaussFit> fails;

ostream &operator<<(ostream &o, const GaussFit &fit) {
   o << fit.getX() << " " << fit.getY() << " " 
#ifdef GAUSSIAN_FIT_INFO
     << fit.amplitude << " ("
     << fit.threshold << ") "
     << fit.xdeviance << " " << fit.ydeviance << " " << fit.isGood() 
#endif
     << endl;
   return o;
}

void fit(int motivation, const Localization &prefit, const Image &i) {
   const GaussFit &fit = static_cast<const GaussFit&>(prefit);
   if (motivation == 1 && fit.isGood()) {
      CImgDisplay d(768, 768, "Corner frustration case", 1);
      ColorImage ci (i);
      while (!fails.empty()) {
         ci.mark(fails.front());
         cout << fails.front();
         fails.pop();
      }
      ci.mark(fit, 2);
      cout << fit << endl;
      d << ci;
      while ( ! d.is_key() ) d.wait();
   } else if (motivation == 1) {
      while (! fails.empty())
         fails.pop();
   } else
      fails.push(fit);
}

int main(int argc, char *argv[]) {
   Driver::Config config;
   config.readConfig(argc, argv);
   string s = config.inputFile();
   SIFVector input(s.c_str());
   Engine master(config);
#ifdef DSTORMENGINE_AFTER_FIT_HOOK
   master.after_fit_hook = fit;
   master.storm(input);
#endif
}
