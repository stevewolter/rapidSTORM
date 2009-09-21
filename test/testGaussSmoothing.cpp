#include "spotFinders/GaussSmoothing.h"

#include <iostream>
#include <iomanip>
#include <time.h>

#include <CImg.h>
#include <CImgBuffer/Image.h>
#include <dStorm/Image.h>

using namespace std;
using namespace dStorm;

class GaussSmoothingTester : private dStorm::GaussSmoother {
  private:
    static const int imw = 20, imh = 20;
    int xr, yr;
  public:
    GaussSmoothingTester(const dStorm::Config &conf) throw() 
    : GaussSmoother( conf, imw, imh )
    {
        xr = xkern.size()-1;
        yr = ykern.size()-1;
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
        dStorm::Image image(imw, imh, 0);
        for (int x = 0; x < imw; x++)
            for (int y = 0; y < imh; y++)
                image(x,y) = (x % 5 == 4 && y % 6 == 5) ? 1 : 0;

        for (int y = 0; y < imh; y++) {
            for (int x = 0; x < imw; x++)
                cout << setw(3) << image(x,y) << " ";
            cout << endl;
        }
        cout << endl;

        SmoothedImage result(imw, imh);
        this->prepare(image);
        result = getSmoothed();

        for (int y = 0; y < imh; y++) {
            for (int x = 0; x < imw; x++)
                cout << setw(3) << 
                        ((y >= yr && x >= xr && y < imh-yr && x < imw-xr) ?
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
            prepare(image);
        end = clock();
        
        cout << "Smoothing on 256x256 took " << 
                ((double)(end-begin) / n) / (CLOCKS_PER_SEC/1000) 
                << " ms." << endl;
    }

    int test() throw() {
        // printDelta();
        // printTime();
        
        return EXIT_SUCCESS;
    }
};

int main() {
    dStorm::Config conf;
    return GaussSmoothingTester(conf).test();
}
