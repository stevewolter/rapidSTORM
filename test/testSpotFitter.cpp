#include <dStorm/SpotFinder.h>
#include <dStorm/SpotFitter.h>
#include <dStorm/GaussFitter.h>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <SIFStorm/SIFLoader.h>
#include "ColorImage.h"
#include <math.h>

using namespace dStorm;
using namespace SIFStorm;
using namespace std;
using namespace cimg_library;

class SpotFitterTest {
   public:
      static int testSpotFitter(const char *file, int image) {
         SIFVector iv(file);
         SIFVector::Iterator i = iv[image];

         ColorImage marked(*i);
         CImgDisplay b(768, 768, "testSpotFinder", 1);

         Config conf;
         auto_ptr<FoundSpots> fs = 
            SpotFinder(conf.maskSize()).findSpots(*i);
         auto_ptr<SpotFitter> fit(SpotFitter::factory(conf));
         fit->prepareToFitImage(*i);

         cout << file << ", Image " << image << endl;
         cout << "Image mean is " << i->mean() << endl;
         while (fs->hasSpots()) {
            Spot s = fs->nextSpot();
            cout << s.x << " " << s.y << endl;
            const Localization &f = fit->fitSpot(s);
            marked.mark(s, ((f.isGood()) ? ColorImage::Cyan : ColorImage::Red));
            printf("%5.2f %5.2f\n", 
               f.getPreciseX() - s.x, f.getPreciseY() - s.y);
            b << marked;
            while (!b.is_key(cimg::keyN, true) && !b.is_closed) b.wait();
            if (b.is_closed) break;
         }

         cout << endl;
         return 0;
      }

      static int outputSpotStatistics(const char *file) {
         Config conf;
         conf.sigma_x_sq = 2.5;

         int samples = 500;
         for (int trials = 0; trials < 4000 / samples; trials++) {
            for (int it = 0; it < 10; it++) {
               double weighted_average = 0, total_weight = 0;
               auto_ptr<SpotFitter> fit(new GaussFitter(conf, false));
                                          

               SIFVector iv(file);
               int pos = 0;
               for (SIFVector::iterator i = iv.begin(); i != iv.end(); i++, pos++) {
                  if (pos < trials * samples) continue;
                  if (pos > (trials+1) * samples) break;
                  auto_ptr<FoundSpots> fs = 
                     SpotFinder(conf.maskSize()).findSpots(*i);
                  fit->prepareToFitImage(*i);

                  int count = 0;
                  while (fs->hasSpots()) {
                     if (count++ > 5) break;
                     Spot s = fs->nextSpot();
   #ifdef GAUSSIAN_FIT_INFO
                     const GaussFitter::GaussFit &f = 
                        (const GaussFitter::GaussFit&)fit->fitSpot(s);
                     if ( ! f.isValid() ) continue;
   #if 0
                     if (f.amplitude > 0)
                        cout << count++ << " " << f.amplitude << " " << f.sigma_x << " "
                           << f.sigma_y << " " << f.shift << " " 
                           << f.residues << endl;
   #endif
                     if (f.residues < 10) {
                        cerr << s.x << " " << s.y << endl;
                        ColorImage marked(*i);
                        CImgDisplay b(768, 768, "testSpotFinder", 1);
                        marked.mark(s);
                        b << marked;
                        while (!b.is_key(cimg::keyN, true) && !b.is_closed) b.wait();
                        if (b.is_closed) break;
                     } else
                     if (f.sigma_x > 0.1 * conf.sigma_x_sq() && f.sigma_x < 10 * conf.sigma_x_sq()
                     && f.sigma_y > 0.1 * conf.sigma_y_sq() && f.sigma_y < 10 * conf.sigma_y_sq()
                     && f.isGood())
                     {
                        double weight = (f.amplitude / f.residues);
                        weighted_average += f.sigma_x * weight;
                        total_weight += weight;
                        cout << trials << " " << it <<  " " << weight << " " << f.sigma_x << " " << f.amplitude << " " << f.residues << " " << f.sigma_y << " " << f.shift << endl;
                     }
   #endif
                  }
               }
               conf.sigma_x_sq = weighted_average / total_weight;
               cerr << conf.sigma_x_sq << endl;
            }
         }
         return 0;
      }

      static void printAverageVariance(const char *file) {
        SIFVector iv(file);
        for (SIFVector::iterator i = iv.begin(); i != iv.end(); i++) {
        }
      }
};

int main(int argc, char* argv[]) {
   CImgBuffer::Config<unsigned short> conf;
   const char *f = "/media/windows/StormIn/Done/01 20mW 647, 5mW 514, 100ms Teil1.sif";

   SpotFitterTest::outputSpotStatistics(f);
   //for (int i = 0; i < 8000; i += 200)
      //SpotFitterTest::testSpotFitter(f, i);
}
