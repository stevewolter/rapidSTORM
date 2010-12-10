#include "../locprec/pixelatedBessel.h"
#include <stdlib.h>
#include <iostream>
#include <math.h>

using namespace std;

int main(int argc, char *argv[]) 
{
    double args[5] = { 0, 0, 1, 1, 0 };
    for (int i = 0; i < 5; i++)
        if (argc > i+1) {
            args[i] = atof(argv[i+1]);
        }

    int lx = int(round(args[0])), ly = int(round(args[1]));
    for (double x = -6; x <= 6; x += 1) {
        for (int y = -6; y <= 6; y += 1)
            cout << lx+x << " " << ly + y << " " <<
               integratedBesselFunction(x+lx-args[0], y+ly-args[1],
                    args[2], args[3], args[4]) << endl;
    }

    return 0;
}
