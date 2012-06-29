#include <boost/test/unit_test.hpp>
#include <dStorm/image/iterator.h>
#include <dStorm/image/constructors.h>
#include <boost/units/io.hpp>
#include "morphological_reconstruction.hpp"

#include <iostream>
#include <iomanip>

namespace dStorm {
namespace image {

using namespace std;
using namespace boost::units;

static const int n = 8;

static int marker[][n] = {
    { 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 10, 11, 10, 11, 10, 1, 255 },
    { 255, 14, 13, 12, 12, 10, 1, 255 },
    { 255, 11, 12, 16, 15, 10, 1, 255 },
    { 255, 10, 16, 14, 16, 16, 1, 255 },
    { 255, 9,   8,  7,  6,  5, 1, 255 },
    { 255, 1,   1,  1,  1,  1, 1, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255 }
};

static int mask[][n] = {
    { 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 20, 20, 20, 20, 20, 20, 255 },
    { 255, 20, 15, 14, 14, 12, 20, 255 },
    { 255, 20, 14, 18, 17, 12, 20, 255 },
    { 255, 20, 16, 14, 16, 16, 20, 255 },
    { 255, 20,  8, 17,  6,  5, 20, 255 },
    { 255, 20, 20, 20, 20, 20, 20, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255 }
};

static int normResult[][n] = {
    { 0, 0, 0, 0, 0, 0, 0, 0 },
   { 0, 16,   16,   16,   16,   16,16, 0 },
   { 0, 16,   15,   14,   14,   12,16, 0 },
   { 0, 16,   14,   16,   16,   12,16, 0 },
   { 0, 16,   16,   14,   16,   16,16, 0 },
   { 0, 16,    8,   16,    6,    5,16, 0 },
   { 0, 16,   16,   16,   16,   16,16, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};

void reconstruction_by_dilation_unit_test() {
    Image<int,2>::Size size;
    size.x() = n * camera::pixel;
    size.y() = n * camera::pixel;
    Image<int,2> imark(size), imask(size), iresult(size);

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            imark(j,i) = marker[i][j];
            imask(j,i) = mask[i][j];

            iresult(j,i) = normResult[i][j];
        }

    Image<int,2> test(size);
    reconstruction_by_dilation(imark, imask, test);
    bool isEqual = true;
    for (Image<int,2>::const_iterator i = test.begin(), j = iresult.begin(); i != test.end(); ++i, ++j) {
        if ( *i != *j ) {
            std::cerr << "Difference at " << i.x() << " " << i.y() << ", value is " << *i << " and should be " << *j << std::endl;
            isEqual = false;
        }
    }
    BOOST_CHECK( isEqual );
}

}
}
