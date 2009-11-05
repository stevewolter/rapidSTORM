#include <CImgBuffer/Image.h>
#include <iostream>
#include <iomanip>
#include <dStorm/engine/Image.h>
#include <CImg.h>
#include "spotFinders/averageSmooth.h"
#include "spotFinders/dilation.h"
#include <time.h>

using namespace std;
using namespace dStorm;
using namespace cimg_library;

static const int xsz = 128, ysz = 128, xms = 25, yms = 25;

void print(const CImg<SmoothedPixel>& img, int xb, int yb) throw() {
    for (int y = 0; y < yb; y++) {
        for (int x = 0; x < xsz; x++)
            cout << setw(1) << 0 << " ";
        cout << "\n";
    }

    for (int y = yb; y < ysz-yb; y++) {
        for (int x = 0; x < xb; x++)
            cout << setw(1) << 0 << " ";
        for (int x = xb; x < xsz-xb; x++)
            cout << setw(1) << img(x,y) << " ";
        for (int x = 0; x < xb; x++)
            cout << setw(1) << 0 << " ";
        cout << "\n";
    }
    for (int y = 0; y < yb; y++) {
        for (int x = 0; x < xsz; x++)
            cout << setw(1) << 0 << " ";
        cout << "\n";
    }
}

bool testDilation(int (*pixelFunc)(int x, int y), bool dilate) {
    CImg<SmoothedPixel> in(xsz, ysz), out(xsz, ysz), test, 
                        mask(2*xms+1, 2*yms+1);
    mask.fill(1);

    for (int x = 0; x < xsz; x++)
        for (int y = 0; y < ysz; y++)
            in(x, y) = pixelFunc(x,y);

    memcpy(out.ptr(), in.ptr(), in.size() * sizeof(SmoothedPixel));
    if (dilate) {
        rectangular_dilation(out, out, xms, yms, 0, 0);
        test = in.get_dilate(mask);
    } else {
        rectangular_erosion(out, out, xms, yms, 0, 0);
        test = in.get_erode(mask);
    }

    bool differ = false;
    for (int x = 0; x < int(out.width); x++)
        for (int y = 0; y < int(out.height); y++)
            if (test(x,y) != out(x,y))
                differ = true ;

    if (differ) {
        cerr << "Test failed" << endl;
        print(out, 0, 0);
        cout << "\n";
        print(test, 0, 0);
        cout << "\n";
    }
    return !differ;
}

void timeTest() {
    clock_t b, e;
    CImg<SmoothedPixel> in(xsz, ysz), out(xsz, ysz);

    for (int x = 0; x < xsz; x++)
        for (int y = 0; y < ysz; y++)
            in(x,y) = rand() % 10;

    int BX = xms, BY = yms;
    int n = 1000;
    b = clock();
    for (int i = 0; i < n; i++)
        rectangular_dilation(in, out, xms, yms, BX, BY);
    e = clock();

    cout << "Took " << (e-b) / (n*double(CLOCKS_PER_SEC)) << " seconds."
         << endl;
}

int columns(int x, int) throw() {
    return ((x) % 2 == 0) + ((x)%4 == 0) + ((x)%6 == 0);
}
int rows(int, int y) throw() {
    return ((y) % 2 == 0) + ((y)%4 == 0) + ((y)%6 == 0);
}
int diags(int x, int y) throw() {
    return ((x+y) % 2 == 0) + ((x+y)%4 == 0) + ((x+y)%6 == 0);
}

int rands(int , int ) throw() {
    return rand() % 1000;
}

int main() throw() {
    bool working = (testDilation(columns, false) && testDilation(rows, false) &&
            testDilation(diags, false) && testDilation(rands, false)
            && testDilation(columns, true) && testDilation(rows, true) &&
            testDilation(diags, true) && testDilation(rands, true));

    //if (working) timeTest();

    return (working) ? EXIT_SUCCESS : EXIT_FAILURE;
}
