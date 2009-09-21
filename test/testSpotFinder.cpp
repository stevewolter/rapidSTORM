#include <dStorm/SpotFinder.h>
#include <iostream>
#include <cassert>
#include <SIFStorm/SIFLoader.h>
#include "ColorImage.h"

using namespace dStorm;
using namespace SIFStorm;
using namespace std;
using namespace cimg_library;

namespace dStorm {
   class SpotFinderTest {
      public:
         static auto_ptr<Image> naively_compute_average_mask(int sz,
            const Image &i)
         {
            int w = i.width;
            int h = i.height;
            auto_ptr<Image> t(new Image(w-2*sz, h-2*sz));
            for (int x = sz; x < w-sz; x++)
               for (int y = sz; y < h-sz; y++) {
                  int sum = 0;
                  for (int dx = -sz; dx <= sz; dx++)
                     if (x+dx >= 0 && x+dx < w)
                        for (int dy = -sz; dy <= sz; dy++)
                           if (y+dy >= 0 && y+dy < h)
                              sum += i(x+dx, y+dy);
                  sum /= (2*sz+1) * (2*sz+1);
                  (*t)(x-sz, y-sz) = sum;
               }
            return t;
         }

         static int testAverageMask() {
            int h = 20, w = 10;
            Image src(w, h);

            for (int x = 0; x < w; x++)
               for (int y = 0; y < h; y++)
                  src(x, y) = x+y;

            SpotFinder finder;
            auto_ptr<Image> test_a = src.averageMask(0);
            
            for (int r = 0; r < 5; r++) {
               if (h - 2*r < 0 || w - 2*r < 0) {
                  cerr << "Can't test averageMask for size " << r << endl;
                  continue;
               }
               auto_ptr<Image> res = src.averageMask(r),
                              naive = naively_compute_average_mask(r, src);

               if ( *res != *naive) {
                  cerr << "Test failed: dStormEngine::SpotFinder::averageMask"
                        " fails for radius " << r << endl;
                  cerr << "Output for matrices follows, format is finder/naive" << endl;
                  for (int y = 0; y < h-2*r; y++) {
                     for (int x = 0; x < w-2*r; x++)
                        cout << (*res)(x, y) << "\t" << (*naive)(x, y) << "\t|";
                     cout << endl;
                  }
                  return 1;
               }
            }

            return 0;
         }

         static void showAverageAndMaximum(const char *file, int image) {
            SIFVector iv(file);
            SIFVector::Iterator i = iv[image];
            SpotFinder f;

            CImgDisplay a(768, 768, "Average", 1);

            auto_ptr<Image> glatt = i->averageMask(3);

            CImg<Image::Pixel> marked(glatt->width, glatt->height, 1, 3);
            Image::Pixel blue[] = { glatt->min(), glatt->max(), glatt->max() };
            for (int x = 0; x < marked.dimx(); x++)
               for (int y = 0; y < marked.dimy(); y++) {
                  marked(x, y, 0, 0) = (*glatt)(x, y);
                  marked(x, y, 0, 1) = (*glatt)(x, y);
                  marked(x, y, 0, 2) = (*glatt)(x, y);
               }

            auto_ptr<FoundSpots> fs = SpotFinder(3).findSpots(*i);
            
            while (fs->hasSpots()) {
               a << marked;
               Spot s = fs->nextSpot();
               marked.draw_rectangle(max<int>(0, int(s.x)-3-4), max<int>(0, int(s.y)-3-4),
                              min<int>(glatt->width-1, int(s.x-3+4)),
                              min<int>(glatt->height-1, int(s.y-3+4)),
                              (blue), 1.0, ~0U);
               
               cout << s.x << " " << s.y << " " << s.average << endl;
               while (! a.is_key(true)) a.wait();
            }
         }

         static int testSpotFinder(const char *file, int nimage) {
            SIFVector iv(file);
            SIFVector::Iterator i = iv[nimage];
            int ms = 3;

            CImgDisplay b(768, 768, "testSpotFinder", 1);

            const Image &image(*i);
            auto_ptr<FoundSpots> fs = SpotFinder(ms).findSpots(image);
            ColorImage marked(image);

            bool rest_bad = false;
            int n = 0;
            while(fs->hasSpots()) {
               Spot s = fs->nextSpot();

               marked.mark(s);

               if (!rest_bad) {
                  b << marked;
                  b.flush();
                  while (!b.is_key() && !b.is_closed) b.wait();
                  if (b.is_key(cimg::keyB, false))  
                     { cout << "bad "; }
                  else if (b.is_key(cimg::keyR, false))  
                     { cout << "bad "; rest_bad = true; }
                  else
                     { cout << "good "; }
                  b.flush();
               } else
                  cout << "bad ";
               cout << n++ << " " << s.variance(image, ms) << endl;
            }

            return 0;
         }

         static int outputSpotStrengths(const char *file, int image) {
            SIFVector iv(file);
            SIFVector::Iterator i = iv[image];
            int ms = 3;

            auto_ptr<FoundSpots> fs = SpotFinder(ms).findSpots(*i);

            int sn = 0;
            while (fs->hasSpots()) {
               Image::Pixel strength = fs->nextSpotsStrength();
               Spot s = fs->nextSpot();

               cout << sn++ << " " << strength << endl;
            }

            return 0;
         }

         static int testMaximum() {
            bool failed = false;
            Image::Pixel data[][5] = {
               { 1, 2, 3, 2, 1 },
               { 1, 2, 2, 2, 1 },
               { 1, 2, 4, 2, 1 },
               { 1, 2, 3, 2, 1 },
               { 1, 2, 2, 2, 1 }
            };
            
            Image im(5, 5);
            for (int x = 0; x < 5; x++)
               for (int y = 0; y < 5; y++)
                 im(x, y) = data[y][x];

            auto_ptr<CandidateTree> max = im.findLocalMaximums(2, 2);
            if (max->size() != 1) failed = true;
            for (CandidateTree::iterator i = max->begin(); i != max->end(); i++) {
               if (i->first != 4) { 
                  failed = true; 
               }
            }

            return (failed) ? 1 : 0;
         }
      static int testVariance() {
         Image spot(9, 9);
         Image nospot(9, 9);
         double av1 = 0, av2 = 0;
         int sweep = 1;

         for (int x = 0; x <9; x++)
            for (int y = 0; y < 9; y++) {
               spot(x, y) = Image::Pixel(10000 * exp( - ((x-4)*(x-4) + (y-4)*(y-4)) / 4 ));
               nospot(x, y) = Image::Pixel(10000 * (((x+y) % 2) ? 0.5 : 1));
               av1 = ((sweep-1) * av1 + spot(x, y)) / sweep;
               av2 = ((sweep-1) * av2 + nospot(x, y)) / sweep;
               sweep++;
            }

         cout << "Averages: " << av1 << " " << av2 << endl;
         Spot s(4, 4, Image::Pixel(av1));
         cout << "Gauss spot: " << s.variance(spot, 4) << endl;
         s.average = Image::Pixel(av2);
         cout << "Noise spot: " << s.variance(nospot, 4) << endl;

         return 0;
      }
      static int outputVariance(const char *f, int im_index) {
            SIFVector iv(f);
            ImageVector::Iterator image = iv[im_index];
            int ms = 3;

            CImgDisplay a(896, 896, "Test image", 1);
            a << *image;
            while (!a.is_key() && !a.is_closed) a.wait();

            auto_ptr<FoundSpots> fs = SpotFinder(ms).findSpots(*image);
            
            int i = 0;
            while (fs->hasSpots()) {
               cout << i++ << " " << fs->nextSpot().variance(*image, ms) << endl;
            }

            return 0;
      }
   };

}

int main(int argc, char* argv[]) {
   const char *f = "/mnt/windows/StormIn/Done/01 20mW 647, 5mW 514, 100ms Teil1.sif";
   assert(SpotFinderTest::testAverageMask() == 0);
   assert(SpotFinderTest::testMaximum() == 0);

   if (argc > 1 && !strcmp(argv[1], "--average"))
      SpotFinderTest::showMedianAndAverage(f, 20);
   else if (argc > 1 && !strcmp(argv[1], "--outputSpotStrengths"))
      for (int i = 10; i < 20; i++)
         SpotFinderTest::outputSpotStrengths(f, i);
   else if (argc > 1 && !strcmp(argv[1], "--testVariance"))
      SpotFinderTest::testVariance();
   else if (argc > 1 && !strcmp(argv[1], "--outputVariance"))
      SpotFinderTest::outputVariance(f, 20);
   else if (argc > 1 && !strcmp(argv[1], "--showAverageAndMaximum"))
      SpotFinderTest::showAverageAndMaximum(f, 1000);
   else if (argc > 1 && !strcmp(argv[1], "--show1000"))
      SpotFinderTest::testSpotFinder(f, 1000);
   else
      for (int i = 10; i < 20; i++)
         SpotFinderTest::testSpotFinder(f, 20);
}
