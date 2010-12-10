#include <dStorm/Image.h>
#include <foreach.h>
#include <fstream>
#include <CImgBuffer/Buffer.h>
#include <CImgBuffer/Image.h>
#include <CImgBuffer/Slot.h>

#include <CImg.h>
#ifdef INCLUDE_LOCPREC
#include <locprec/NoiseSource.h>
#endif

#include <limits>
#include <stdint.h>

using namespace cimg_library;
using namespace CImgBuffer;
using namespace dStorm;
using namespace std;

int main(int argc, char *argv[]) {
#ifdef INCLUDE_LOCPREC
   locprec::NoiseMethod<unsigned short>::registerMethod();
#endif
   CImgBuffer::Config<dStorm::Image> config;
   config.readConfig(argc, argv);
   bool paused = false;

    try {
        Buffer<dStorm::Image> iv( config );
        CImgDisplay d(768, 768, "Storm", 1);
        int ic = 0;
        foreach(i, Buffer<dStorm::Image>, iv) {
            ic++;
            Claim<dStorm::Image> claim = i->claim();
            if ( ! claim.isGood() ) continue;
            dStorm::Image &im = *claim;

            d.display(im);
            d.set_title("%i", ic);
            d.wait(1800);

            do {
                d.wait(200);
                if (d.is_closed || d.is_key(cimg::keyQ, true)) break;
                if (d.is_key(cimg::keyS, true)) {
                    im.get_normalize(0,255).save("Snapshot.png");
                    cerr << "Snapshot saved to Snapshot.png" << endl;
                }
                if (d.is_key(cimg::keyP, true)) {
                    if (paused)
                        d.set_title("%i (paused)", ic);
                    paused = !paused;
                }
                if (d.is_key(cimg::keyT, true)) {
                    CImg<uint8_t> tim = im.get_normalize(0,255);
                    ofstream f("image.tbl", ios_base::out);
                    cimg_forXY(tim, x, y) {
                        f << x << " " << y << " " << (int)tim(x,y) << "\n";
                    }
                }
            } while (paused);
        }
    } catch (const std::exception& e) {
        cerr << e.what() << endl;
        exit(1);
    }

   return 0;
}
