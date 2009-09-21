#include <CImgBuffer/Image.h>
#include <iostream>
#include <iomanip>
#include <dStorm/Image.h>
#include <CImg.h>
#include "spotFinders/averageSmooth.h"
#include "engine/CandidateTree.h"
#include <time.h>

using namespace std;
using namespace dStorm;
using namespace cimg_library;

static const int xsz = 10, ysz = 128, xms = 2, yms = 2;

typedef dStorm::CandidateTree<dStorm::SmoothedPixel> Candidates;

void print(const CImg<SmoothedPixel>& img, const Candidates* maxes,
int xb, int yb) throw() 
{
    bool inmax[img.width][img.height];
    cimg_forXY(img, x, y) inmax[x][y] = false;

    if (maxes)
        for (Candidates::const_iterator i = maxes->begin();
                                      i != maxes->end(); i++)
        {
            cout << "Maximum with strength " << i->first << " " 
            << i->second.x() << " " << i->second.y() << "\n";
            inmax[i->second.x()][i->second.y()]
                = true;
        }

    for (int y = 0; y < yb; y++) {
        for (int x = 0; x < xsz; x++)
            cout << setw(2) << 0 << " ";
        cout << "\n";
    }

    for (int y = yb; y < ysz-yb; y++) {
        for (int x = 0; x < xb; x++)
            cout << setw(2) << 0 << " ";
        for (int x = xb; x < xsz-xb; x++) {
            if (inmax[x][y]) cout << "\033[1m";
            cout << setw(2) << img(x,y) << " ";
            cout << "\033[0m";
        }
        for (int x = 0; x < xb; x++)
            cout << setw(2) << 0 << " ";
        cout << "\n";
    }
    for (int y = 0; y < yb; y++) {
        for (int x = 0; x < xsz; x++)
            cout << setw(2) << 0 << " ";
        cout << "\n";
    }
}

bool testMaximumSearch(int (*pixelFunc)(int x, int y)) {
    CImg<SmoothedPixel> in(xsz, ysz), buf(xsz, ysz);
    Candidates maxes(xms, yms, 1, 1);
    maxes.setLimit(20);

    for (int x = 0; x < xsz; x++)
        for (int y = 0; y < ysz; y++)
            in(x, y) = pixelFunc(x,y);

    maxes.fill(in);
    print(in, &maxes, 0, 0);
    cout << "\n";
    return true;
}

void timeTest(int (*pixelFunc)(int x, int y)) {
    clock_t b,e;
    CImg<SmoothedPixel> in(xsz, ysz), buf(xsz, ysz);
    Candidates maxes(xms, yms, 1, 1);
    maxes.setLimit(20);

    for (int x = 0; x < xsz; x++)
        for (int y = 0; y < ysz; y++)
            in(x, y) = pixelFunc(x,y);

    int n = 1000;
    b = clock();
    for (int i = 0; i < n; i++) 
        maxes.fill(in);
    e = clock();
        
    cout << "Took " << (e-b) / (n*double(CLOCKS_PER_SEC)) << " seconds."
         << endl;
}

int spot(int x, int y) throw() {
    int xa = 3 * xsz / 4, ya = 3*ysz / 4, xb = xsz / 4, yb = ysz / 4;

    int a = 2;
    if ( abs(x-xa) < 2 ) a++;
    if ( abs(x-xb) < 2 ) a++;
    if ( abs(x-xa) < 1 ) a++;
    if ( abs(x-xb) < 1 ) a++;
    if ( abs(y-ya) < 2 ) a++;
    if ( abs(y-yb) < 2 ) a++;
    if ( abs(y-ya) < 1 ) a++;
    if ( abs(y-yb) < 1 ) a++;
    return a;
}

int random(int, int) throw() {
    return rand() % 100;
}

int main() throw() {
    bool working = (testMaximumSearch(spot) && testMaximumSearch(random));

    if (working) timeTest(random);

    return (working) ? EXIT_SUCCESS : EXIT_FAILURE;
}
