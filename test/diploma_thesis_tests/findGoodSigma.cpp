#include <dStorm/engine/SpotFinder.h>
#include <dStorm/SpotFitter.h>
#include <dStorm/GaussFitter.h>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <dStormUtils/SIFLoader.h>
#include <dStormUtils/Driver.h>
#include "ColorImage.h"
#include <math.h>

using namespace dStorm;
using namespace dStormUtils;
using namespace std;
using namespace cimg_library;

namespace dStorm {
   class SpotFitterTest {
      public:
         static int outputSpotStatistics(Driver::Config &conf) {
            conf.sigma_x_sq = 1.5;
            conf.sigma_y_sq = 3.5;

            int samples = 500;
            for (int trials = 0; trials < 3999 / samples; trials++) {
               double oldx = conf.sigma_x_sq(), oldy = conf.sigma_y_sq();
               int it;
               for (it = 0; it < 10; it++) {
                  int count = 0;
                  double weighted_x_average = 0, weighted_y_average = 0, total_weight = 0;
                  auto_ptr<SpotFitter> finder(new GaussFitter(conf)),
                                       fitter(new GaussFitter(conf, false));
                                           

                  SIFVector iv(conf.inputFile().c_str());
                  int pos = 0;
                  for (SIFVector::iterator i = iv.begin(); i != iv.end(); i++, pos++) {
                     if (pos < trials * samples) continue;
                     if (pos > (trials+1) * samples) break;
                     auto_ptr<FoundSpots> fs = 
                        SpotFinder(conf.maskSize()).findSpots(*i);
                     finder->prepareToFitImage(*i);
                     bool prepared = false;

                     int motivation = conf.motivation();
                     while (fs->hasSpots() && motivation > 0) {
                        Spot s = fs->nextSpot();
                        const GaussFitter::GaussFit& found = 
                             (const GaussFitter::GaussFit&)(finder->fitSpot(s));
                        motivation = (found.isGood()) ? 
                           conf.motivation() : 
                           motivation - 1;

                        if (found.isGood()) {
#ifdef GAUSSIAN_FIT_INFO
                           if (!prepared)
                              fitter->prepareToFitImage(*i);
                           const GaussFitter::GaussFit &f = 
                              (const GaussFitter::GaussFit&)fitter->fitSpot(s);
                           if ( ! f.isGood() ) continue;

                           double weight = (found.amplitude / found.residues);
                           weighted_x_average += f.sigma_x * weight;
                           weighted_y_average += f.sigma_y * weight;
                           total_weight += weight;
                           count++;
                        }
                     }
#endif
                  }
                  conf.sigma_x_sq = weighted_x_average / total_weight;
                  conf.sigma_y_sq = weighted_y_average / total_weight;
                  if (abs(oldx - conf.sigma_x_sq()) / oldx < 0.01 && abs(oldy - conf.sigma_y_sq()) / oldy)
                     break;
                  else {
                     oldx = conf.sigma_x_sq(), oldy = conf.sigma_y_sq();
                  }

               }
               cerr << trials << " " << it << " " << conf.sigma_x_sq() << " " << conf.sigma_y_sq() << endl;
            }
            return 0;
         }
   };
}

int main(int argc, char* argv[]) {
   Driver::Config conf;
   conf.inputFile = "/media/windows/StormIn/Done/01 20mW 647, 5mW 514, 100ms Teil1.sif";
   conf.readConfig(argc, argv);

   SpotFitterTest::outputSpotStatistics(conf);
   return 0;
}
