#include <foreach.h>
#include <CImgBuffer/Image.h>
#include <CImgBuffer/Buffer.h>
#include <CImgBuffer/Slot.h>
#include <iostream>
#include <iomanip>
#include <dStorm/Image.h>
#include <dStorm/CarConfig.h>
#include <dStorm/BasicTransmissions.h>
#include <CImg.h>
#include "spotFinders/averageSmooth.h"
#include "engine/SpotFinder.h"

using namespace std;
using namespace dStorm;
using namespace cimg_library;
using namespace CImgBuffer;

static const int xsz = 128, ysz = 128, xms = 2, yms = 2;
StormPixel testArray[xsz][ysz];

int testAverage() throw() {
    CImg<StormPixel> image(xsz, ysz);
    CImg<SmoothedPixel> smoothed(xsz, ysz);
    for (int x = 0; x < xsz; x++)
        for (int y = 0; y < ysz; y++)
            image(x, y) = testArray[x][y];

    smoothByAverage(image, smoothed, xms, yms);

    int xo = 0, yo = 0, sf = xms*yms;

    bool did_smooth_well = true;
    for (int x = xms; x < xsz-xms; x++) {
        for (int y = yms; y < ysz-yms; y++) {
            SmoothedPixel ref = 0;
            for (int dx = -xms; dx <= xms; dx++)
                for (int dy = -yms; dy <= yms; dy++)
                    ref += image(x+dx, y+dy);

            if ( smoothed(x+xo, y+yo) != ref / sf
                 && smoothed(x+xo, y+yo) != ref)
            {
                did_smooth_well = false;
                cerr << "Mask error at " << x << "," << y << ": "
                     << "Expected " << ref << ", got " << smoothed(x, y)
                     << endl;
            }
        }
    }

    return (did_smooth_well) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#include <time.h>

void testTime(int argc, char *argv[]) throw() {
    BasicTransmissions trans;
    dStorm::CarConfig conf( trans );
    conf.readConfig(argc, argv);
    std::auto_ptr<SpotFinder> sf = 
        SpotFinder::factory(conf.engineConfig, xsz, ysz);
    time_t begin, end;

    //CandidateTree<SmoothedPixel> maximums(conf);
    //maximums.setLimit(20);

    begin = clock();
    Buffer<dStorm::Image> iv( conf.inputConfig );
    foreach(i, Buffer<dStorm::Image>, iv) {
        Claim<dStorm::Image> claim = i->claim();
        if ( ! claim.isGood() ) continue;
        dStorm::Image &image = *claim;

        //if (image.getImageNumber() % 1000 == 0) cerr << image.getImageNumber() << endl;

        sf->prepare(image);
        //sf->findSpots(image, maximums);
    }
    end = clock();

    cout << "Smoothing took " << double(end-begin) / CLOCKS_PER_SEC <<
            " s" << endl;
}

int main(int argc, char *argv[]) {
    int rv = (testAverage());
    if (rv == EXIT_SUCCESS && argc > 1)
        testTime(argc, argv);

    return rv;
}
