#include "GarageConfig.h"
#include <dStorm/input/ImageTraits.h>
#include <dStorm/engine/Image.h>
#include <fstream>
#include <dStorm/input/Buffer.h>
#include <dStorm/input/Slot.h>
#include <dStorm/engine/Input.h>
#include <iomanip>
#include <CImg.h>
#include <simparm/ChoiceEntry_Impl.hh>

#include <limits>
#include <stdint.h>

using namespace cimg_library;
using namespace dStorm::input;
using namespace dStorm::engine;
using namespace dStorm;
using namespace std;
using namespace simparm;

int main(int argc, char *argv[]) {
   GarageConfig garageConfig;
   input::Config& config = garageConfig.get_input_config();
   FileEntry saveMovie("saveMovie", "");
   FileEntry outputPixels("outputPixels", "");
   BoolEntry show("show", "", true);
   UnsignedLongEntry maxPix("max", "", numeric_limits<StormPixel>::max());
   BoolEntry saveImages("SaveImages", "Save each image into Snapshot_NUMBER.png");

   UnsignedLongEntry markX("MarkX", "Mark column", 1000);
   UnsignedLongEntry markY("MarkY", "Mark row", 1000);

   DoubleEntry fps("FPS", "Frames per second", 0.5);

   config.push_back(fps);
   config.push_back(saveImages);
   config.push_back(saveMovie);
   config.push_back(outputPixels);
   config.push_back(show);
   config.push_back(maxPix);
   config.push_back(markX);
   config.push_back(markY);
   config.readConfig(argc, argv);
   bool paused = false;

    StormPixel min = numeric_limits<StormPixel>::max(),
               max = numeric_limits<StormPixel>::min();

    if (saveMovie) {
        try {
            Buffer<Image> iv( config );
            for (Input::iterator i = iv.begin(); i != iv.end(); i++)
            {
                Claim<Image> claim = i->claim();
                cimg_forXY(*claim, x, y) {
                    min = std::min((*claim)(x,y), min);
                    max = std::max((*claim)(x,y), max);
                }
            }
        } catch (const std::exception& e) {
            cerr << e.what() << endl;
            exit(1);
        }
    }
    max = std::min<StormPixel>(maxPix(), max);

    try {
        Buffer<Image> iv( config );
        const Traits<Image>& traits = iv.getTraits();
        CImg<StormPixel> pwLimit( traits.dimx().value(), traits.dimy().value(),
                                          1,1, max-min );
        CImgDisplay d(768, 768, "Storm", 1);
        CImgList<uint8_t> liste;
        int ic = 0;
        for (Input::iterator i = iv.begin(); i != iv.end(); i++) {
            Claim<Image> claim = i->claim();
            if ( ! claim.isGood() ) continue;
            Image &im = *claim;

            CImg<StormPixel> copy = im;
            copy = (copy - min).min(pwLimit) * (255.0 / (max-min));
            if (saveMovie) {
                liste.push_back(copy);
            }
            if ( outputPixels ) {
                ostream& o = outputPixels.get_output_stream();
                cimg_forXY( im, x, y )
                    o << std::setw(12) << x << setw(12) << y << setw(12) << claim.index() << setw(10) << im(x,y) << "\n";
            }
            if ( saveImages() ) {
                std::stringstream name; name << "Snapshot_" << i->index() << ".gif";
                im.get_normalize(0,255).save( name.str().c_str() );
            }
            if ( show() ) {
                if ( markX() < im.width && markY() < im.height )
                    im(markX(),markY()) = im.max();
                d.display(im);
                d.set_title("%i", claim.index() 
                    + config.firstImage() );
                d.wait(std::max( int(1000 / fps())-20, 0));

                do {
                    d.wait(20);
                    if (d.is_closed || d.is_key(cimg::keyQ, true)) break;
                    if (d.is_key(cimg::keyS, true)) {
                        std::string name = "Snapshot_" + string(d.title) + ".gif";
                        im.get_normalize(0,255).save( name.c_str() );
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
        }
        if (saveMovie) {
            liste[0](0,0) = 255;
            liste.save( saveMovie().c_str() );
        }
    } catch (const cimg_library::CImgException& e) {
        cerr << e.message << endl;
        exit(1);
    } catch (const std::exception& e) {
        cerr << e.what() << endl;
        exit(1);
    }

   return 0;
}
