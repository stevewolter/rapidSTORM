#include "engine/GaussFitter.h"
#include "engine/SigmaFitter.h"
#include <iomanip>
#include "ColorImage.h"

using namespace dStorm;
using namespace std;
using namespace cimg_library;

int testSimpleImage(int noise) {
   int ms = 2;
   int sx = 18, sy = 20;
   double av = 0;
   int sweep = 1;
   double cx = 18.3, cy = 20.7;
   double sigx = 1.5, sigy = 2.3, sigxy = 0.2, amp = 5000, ellip = (1-sigxy*sigxy);

   Image i(30, 30, 1);
   for (int x = 0; x < 30; x++)
      for (int y = 0; y < 30; y++) {
         i(x,y) = StormPixel(
            (amp / (2 * M_PI * sigx * sigy * sqrt(ellip)))
              * exp( -(0.5/ellip) * (
                (x-cx)*(x-cx)/(sigx*sigx) +
                (y-cy)*(y-cy)/(sigy*sigy) -
                2 * (x-cx) * (y-cy) * sigxy / (sigy*sigx))) +
                  noise * double(rand()) / RAND_MAX);
         if (x >= sx-ms && x <= sx+ms && y >= sy-ms && y <= sy+ms)
            { av = ((sweep-1) * av + i(x,y)) / sweep; sweep++; }
      }

   Config conf;
   conf.sigma_x = 1.2; conf.sigma_y = 2.6; conf.sigma_xy = 0;
   SigmaFitter ff_fitter(conf);
   GaussFitter<false,false,false> fitter(conf);
   fitter.prepareToFitImage(i, 15);
   Spot s (sx, sy);
   Localization f;
   bool res = fitter.fitSpot(s, &f);
   if (res) {
      cout << "Input:  " << cx << "," << cy << " " << " with sigma "
           << sigx << ", " << sigy << ", " << sigxy << endl;
      cout << "Result: " << f << endl;
   }else {
      cout << "No result." << endl;
      cout << "Last try: " << f.getPreciseX() << "," 
                           << f.getPreciseY() << " "
                           << f.getStrength() << endl;
   }
   ff_fitter.useConfig(conf);
   double r[4];
   ff_fitter.fit(i, f, r);
   cout << "Sigma X: " << setw(11) << r[0] << setw(11) << sigx << endl;
   cout << "Sigma Y: " << setw(11) << r[1] << setw(11) << sigy << endl;
   cout << "Sigma XY: " << setw(11) << r[3] << setw(11) << sigxy << endl;

   //ColorImage marked(i);
   //CImgDisplay d(900, 900, "", 1);
   //marked.mark(f);
   //d << marked;
   //while (! d.is_key() && !d.is_closed) d.wait();

   return 0;
}

#if 0
int testWithImage() {
   const char *file = "/mnt/windows/StormIn/Done/01 20mW 647, 5mW 514, 100ms Teil1.sif";
   SIFVector iv(file);
   SIFVector::Iterator i = iv[20];

   Config conf;
   auto_ptr<FoundSpots> fs = SpotFinder(conf.maskSize()).findSpots(*i);
   GaussFitter fit(conf);
   fit.prepareToFitImage(*i);

   int fits = 20;
   while (fs->hasSpots()) {
      fit.fitSpot(fs->nextSpot());
      if (fits-- <= 0) return 0;
   }

   return 0;
}
#endif

int main() {
   for (int i = 0; i < 1000; i += 1000)
      testSimpleImage(i);
}
