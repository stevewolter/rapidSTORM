#define DSTORM_SHOCKABSORBER_CPP
#include "FileShockAbsorber.h"
#include <fstream>
#include <stdlib.h>
#include <foreach.h>
#include <cassert>
#include "Localizations.h"

using namespace dStorm;
using namespace std;

ShockAbsorberConfig::ShockAbsorberConfig() {
    input.setName("ShockAbsorptionFile");
    input.setDesc("STM file for motion correction");
    input.setHelp("The file given here, if any, should contain one point "
                 "in every image, giving the coordinates of the first "
                 "image's center in the given image.");
    input = "";
    input.setUserLevel(Entry::Intermediate);
}

void ShockAbsorberConfig::registerEntries(Set &s) {
   s.register_entry(&input);
}

FileShockAbsorber::FileShockAbsorber(const ShockAbsorberConfig &c) 
 : c(c) 
{
    if ( ! c.input ) { x = y = NULL; return; }
    ifstream instream(c.input().c_str());
    Localizations compensation(instream);

    size = compensation.size();
    x = (long int*)malloc(sizeof(long int) * size);
    y = (long int*)malloc(sizeof(long int) * size);
    foreach_const( f, Localizations, compensation ) {
        unsigned int im = f->getImageNumber();
        assert( int(im) < size );
        x[im] = f->getXLow(1);
        y[im] = f->getYLow(1);
    }
}

FileShockAbsorber::~FileShockAbsorber() {
    if (x) free(x);
    if (y) free(y);
}

void FileShockAbsorber::editFits(Localization *fits, int num, const Image &) {
    if (num == 0) return;
    int imNum = fits[0].N();
    if (imNum >= size) imNum = size-1;
    for (int i = 0; i < num; i++) {
        fits[i].shiftX( -x[imNum] );
        fits[i].shiftY( -y[imNum] );
    }
}
