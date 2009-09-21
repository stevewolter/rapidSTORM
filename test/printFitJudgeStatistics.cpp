#include "FitJudger.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <list>
#include <algorithm>
#include <memory>
#include <gsl/gsl_statistics_double.h>

double RawFit::maxCor00 = 0, RawFit::minCor00 = 1000000;
int RawFit::numTrue = 0;

class SD {
  private:
    double *x;
    int pos, sz;

  public:
    SD(int sz) : x(new double[sz]), pos(0), sz(sz) {}
    SD(const SD &corr) : x(new double[corr.sz]), pos(0), sz(sz) {}
    ~SD() throw() { print(cout); delete[] x; }

    void addValue(double val) {
        if (pos < sz) {
            x[pos] = val;
            pos++;
        }
    }
    ostream& print(ostream &o) throw() {
        return (pos) ? (o << gsl_stats_sd(x, 1, pos) << " ") : o;
    }
};

class CSD : public SD, public unary_function<void, const RawFit&> {
    int axis;

  public:
    CSD(int axis, int sz) : SD(sz), axis(axis) {}

    void operator()(const RawFit& f) throw() { addValue(f.center(axis)); }
};

class Correlation {
  private:
    double *x;
    int pos, sz;

  public:
    Correlation(int sz) : x(new double[sz*2]), pos(0), sz(sz) {}
    Correlation(const Correlation &corr) : x(new double[2*corr.sz]),
                                 pos(0), sz(sz) {}
    ~Correlation() throw() { print(cout); delete[] x; }

    void addValue(double val1, double val2) {
        if (pos < 2*sz) {
            x[pos] = val1;
            pos++;
            x[pos] = val2;
            pos++;
        }
    }
    ostream& print(ostream &o) throw() {
        return
            (pos) ? (o << gsl_stats_covariance(x, 2, x+1, 2, pos/2) /
             (gsl_stats_sd(x, 2, pos/2) * gsl_stats_sd(x+1, 2, pos/2))
             << " ") : o;
    }
};

class AC : public unary_function<void, const RawFit&>, public Correlation{
  public:
    AC(int sz) : Correlation(sz) {}

    void operator()(const RawFit& f) throw() { 
        addValue(f.amp, f.photonCount);
    }
};

int main(int argc, char *argv[]) {
    assert( argc == 3 );

    double parameters[3];
    ifstream parFile(argv[1], ios::in), dataFile(argv[2], ios::in);
    parFile >> parameters[0] >> parameters[1] >> parameters[2];
    FitJudger judge(parameters[0], parameters[2], parameters[1]);

    std::list<RawFit, Eigen::aligned_allocator<RawFit> > fits, tpFits;
    while (dataFile) {
        fits.push_back(RawFit(dataFile));
    }
    fits.pop_back();

    remove_copy_if( fits.begin(), fits.end(), back_inserter(tpFits), 
        isNotTP(judge));
    cout << tpFits.size() << " "
         << count_if( fits.begin(), fits.end(), isFP(judge) ) << " "
         << count_if( fits.begin(), fits.end(), isTN(judge) ) << " "
         << count_if( fits.begin(), fits.end(), isFN(judge) ) << " ";

    for_each( tpFits.begin(), tpFits.end(), AC(tpFits.size()) );
    for_each( tpFits.begin(), tpFits.end(), CSD(0, tpFits.size()) );
    for_each( tpFits.begin(), tpFits.end(), CSD(1, tpFits.size()) );

    cout << endl;

    exit(0);
}
