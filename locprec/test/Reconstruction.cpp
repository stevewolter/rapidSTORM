#include <dStorm/engine/Image.h>
#include <dStorm/image/iterator.h>
#include <dStorm/image/constructors.h>
#include "../Reconstruction.hh"
#include "../Reconstruction.cc"

#include <iostream>
#include <iomanip>

using namespace std;
using namespace dStorm;
using namespace boost::units;

const int n = 8;

int marker[][n] = {
    { 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 10, 11, 10, 11, 10, 1, 255 },
    { 255, 14, 13, 12, 12, 10, 1, 255 },
    { 255, 11, 12, 16, 15, 10, 1, 255 },
    { 255, 10, 16, 14, 16, 16, 1, 255 },
    { 255, 9,   8,  7,  6,  5, 1, 255 },
    { 255, 1,   1,  1,  1,  1, 1, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255 }
};

int mask[][n] = {
    { 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 20, 20, 20, 20, 20, 20, 255 },
    { 255, 20, 15, 14, 14, 12, 20, 255 },
    { 255, 20, 14, 18, 17, 12, 20, 255 },
    { 255, 20, 16, 14, 16, 16, 20, 255 },
    { 255, 20,  8, 17,  6,  5, 20, 255 },
    { 255, 20, 20, 20, 20, 20, 20, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255 }
};

int normResult[][n] = {
    { 0, 0, 0, 0, 0, 0, 0, 0 },
   { 0, 16,   16,   16,   16,   16,16, 0 },
   { 0, 16,   15,   14,   14,   12,16, 0 },
   { 0, 16,   14,   16,   16,   12,16, 0 },
   { 0, 16,   16,   14,   16,   16,16, 0 },
   { 0, 16,    8,   16,    6,    5,16, 0 },
   { 0, 16,   16,   16,   16,   16,16, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};

int main(int argc, char *argv[]) throw() {
    engine::SmoothedImage::Size size;
    size.x() = n * camera::pixel;
    size.y() = n * camera::pixel;
    engine::SmoothedImage imark(size), imask(size), iresult(size);

    if (argc == 2 && !strcmp(argv[1], "--matlab")) {
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                cout << "marker(" << i+1 << "," << j+1 << ") = " << 
                        marker[i][j] << endl;
                cout << "mask(" << i+1 << "," << j+1 << ") = " << 
                        mask[i][j] << endl;
            }
    } else {
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                imark(j,i) = marker[i][j];
                imask(j,i) = mask[i][j];

                iresult(j,i) = normResult[i][j];
            }

        engine::SmoothedImage test(size);
        dStorm::ReconstructionByDilationII<engine::SmoothedPixel>(imark, imask, test);
        bool isEqual = true;
        for (engine::SmoothedImage::const_iterator i = test.begin(), j = iresult.begin(); i != test.end(); ++i, ++j) {
            if ( *i != *j ) {
                std::cerr << "Difference at " << i.x() << " " << i.y() << ", value is " << *i << " and should be " << *j << std::endl;
                isEqual = false;
            }
        }
        return ( (isEqual) ? 0 : 1 );
    }
    return 0;
}
