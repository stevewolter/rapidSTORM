#include "GaussSmoothing.h"

#include <iostream>
#include <iomanip>
#include <time.h>

#include <CImg.h>
#include <CImgBuffer/Image.h>
#include <dStorm/Image.h>

using namespace std;
using namespace dStorm;

class GaussSmoothingTester : private dStorm::GaussSmoothing {
  public:
    GaussSmoothingTester() throw() 
    : GaussSmoothing(1.5, 1.5)
    {
    }

    void printParams() throw() {
        cout << xr << " " << yr << endl;
        for (int i = 0; i <= xr; i++)
            cout << setw(3) << xkern[i] << " ";
        cout << endl;
        for (int i = 0; i <= yr; i++)
            cout << setw(3) << ykern[i] << " ";
        cout << endl;
    }

    void printDelta() throw() {
        dStorm::Image image(20, 20, 0);
        for (int x = 0; x < 20; x++)
            for (int y = 0; y < 20; y++)
                image(x,y) = (x % 5 == 4 && y % 6 == 5) ? 1 : 0;

        for (int y = 0; y < 20; y++) {
            for (int x = 0; x < 20; x++)
                cout << setw(3) << image(x,y) << " ";
            cout << endl;
        }
        cout << endl;

        SmoothedImage result(20, 20);
        smooth(image, result);

        for (int y = 0; y < 20; y++) {
            for (int x = 0; x < 20; x++)
                cout << setw(3) << 
                        ((y >= yr && x >= xr && y < 20-yr && x < 20-xr) ?
                          result(x,y) : 0) << " ";
            cout << endl;
        }
    }

    void printTime() throw() {
        int sz = 256;
        dStorm::Image image(sz, sz, 0);
        SmoothedImage result(sz, sz);
        clock_t begin, end;
        int n = 10000;

        for (int x = 0; x < sz; x++)
            for (int y = 0; y < sz; y++)
                image(x,y) = (7 * x + 11 * y) % 0xFFFF;

        begin = clock();
        for (int i = 0; i < n; i++)
            smooth(image, result);
        end = clock();
        
        cout << "Smoothing on 256x256 took " << 
                ((double)(end-begin) / n) / (CLOCKS_PER_SEC/1000) 
                << " ms." << endl;
    }

    int test() throw() {
        printDelta();
        printTime();
        
        return EXIT_SUCCESS;
    }
};

int main() {
    return GaussSmoothingTester().test();
}
