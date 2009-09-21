#include "spotFinders/Reconstruction.hh"
#include "spotFinders/Reconstruction.cc"

#include <iostream>
#include <iomanip>

using namespace std;
using namespace dStorm;
using namespace cimg_library;

const int n = 6;

int marker[][n] = {
    { 10, 11, 10, 11, 10, 1 },
    { 14, 13, 12, 12, 10, 1 },
    { 11, 12, 16, 15, 10, 1 },
    { 10, 16, 16, 16, 16, 1 },
    { 9,   8,  7,  6,  5, 1 },
    { 1,   1,  1,  1,  1, 1 }
};

int mask[n][n];

int normResult[][n] = {
   { 16,   16,   16,   16,   16,16 },
   { 16,   15,   14,   14,   12,16 },
   { 16,   14,   16,   16,   12,16 },
   { 16,   16,   16,   16,   16,16 },
   { 16,   10,    9,    8,    7,16 },
   { 16,   16,   16,   16,   16,16 }
};

int main(int argc, char *argv[]) throw() {
    CImg<int> imark(n, n), imask(n,n), iresult(n,n);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) 
            if (i == 0 || j == 0 || i == n-1 || j == n-1)
                mask[i][j] = 20;
            else
                mask[i][j] = marker[i][j] + 2;

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

        CImg<int> test(n,n);
        ReconstructionByDilationII<int>(imark, imask, test);
        bool isEqual = (iresult == test);
        if (!isEqual)
            cimg_forY(test, y)  {
                cimg_forX(test, x) cout << setw(4) << test(x,y);
                cout << "    ";
                cimg_forX(iresult, x) cout << setw(4) << iresult(x,y);
                cout << endl;
            }
        return ( (isEqual) ? 0 : 1 );
    }
    return 0;
}
