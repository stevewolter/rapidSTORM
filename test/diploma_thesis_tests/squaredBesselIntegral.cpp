#include "../locprec/pixelatedBessel.h"
#include <stdlib.h>
#include <iostream>
#include <math.h>

using namespace std;

int main(int argc, char *argv[]) 
{
    double args[4] = { 0, 0, 1, 1 };
    for (int i = 0; i < 4; i++)
        if (argc > i+1) {
            args[i] = atof(argv[i+1]);
        }

    double accum;
    int lx = int(round(args[0])), ly = int(round(args[1]));
    for (double x = -6 * args[2]; x <= 6 * args[2]; x += 1) {
        for (int y = -6 * args[3]; y <= 6 * args[3]; y += 1)
            accum += integratedBesselFunction(x+lx-args[0], y+ly-args[1],
                    args[2], args[3]);
    }

    cout << "Mean squared signal is " 
         << accum / (13 * 13 * args[2] * args[3]) << endl;

    return 0;
}
