#include <dStorm/SpotFinder.h>
#include <dStorm/SpotFitter.h>
#include <SIFStorm/SIFLoader.h>
#include <iostream>
#include <iomanip>
#include <cassert>
#include "ColorImage.h"

using namespace dStorm;
using namespace SIFStorm;
using namespace std;
using namespace cimg_library;

int main() {
   SIFVector il
      ("/mnt/windows/StormIn/Done/02_1 25mW 647, 1mW514, 50ms Teil3.sif");
   ImageVector::Iterator i = il[13];
   const Image &image(*i);
   ColorImage marked(*i);
   CImgDisplay b(768, 768, "badlySeperable", 1);

   Config conf;
   SpotFinder finder(conf);
   auto_ptr<SpotFitter> fitter(SpotFitter::factory(conf));
   fitter->prepareToFitImage(image);
   b << image;
   while (!b.is_key() && !b.is_closed) b.wait();

   auto_ptr<FoundSpots> fs = finder.findSpots(image);
   while (fs->hasSpots()) {
      const Localization &fit = fitter->fitSpot(fs->nextSpot());
      marked.mark(fit, ((fit.isGood()) ? ColorImage::Cyan : ColorImage::Red));
      printf("%5.2f %5.2f\n", fit.getPreciseX(), fit.getPreciseY());
      b << marked;
      while (!b.is_key() && !b.is_closed) b.wait();
   }

   return 0;
}
