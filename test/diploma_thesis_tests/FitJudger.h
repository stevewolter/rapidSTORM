#include <iostream>
#include <functional>
#include <Eigen/Core>
#include <Eigen/Array>
#include <limits>

using namespace std;
using namespace Eigen;

template <class T> T sqr(const T& x) { return x*x; } 

class RawFit {
  public:
    int isTrue;
    int imNum;
    double amp;
    int photonCount;
    Vector2d center;
    Matrix2d correlation;

    static double maxCor00, minCor00;
    static int numTrue;
    
  public:
    RawFit(istream &from) {
        from >> isTrue >> imNum;
        from >> center(0) >> center(1);
        from >> amp >> photonCount;
        from >> correlation(0,0) >> correlation(0,1)
             >> correlation(1,0) >> correlation(1,1);

        maxCor00 = max(maxCor00, correlation(0,0));
        minCor00 = min(minCor00, correlation(0,0));
        if (isTrue) numTrue++;
    }

    void print(ostream &o) throw() {
        o << isTrue << "-"<< imNum << "-" << center.transpose() << "-" <<
                correlation.row(0) << "-" << correlation.row(1) << endl;
    }

    static bool variableStandardDeviation() throw() 
        { return (maxCor00 - minCor00) > 0.1; }

    static int numberOfTrueFits() throw() { return numTrue; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class FitJudger {
  private:
    double amp_thres;
    double sigmaTol;
    double maxDistance;
    Matrix2d origSigma;
    Matrix2d upperThres, lowerThres;

  public:
    FitJudger(double amp_thres, double sigmaTol, double maxDist) throw() 
    : amp_thres(amp_thres), sigmaTol(sigmaTol), maxDistance(maxDist)
    {
        origSigma << 1.0/(1.2*1.2), 0, 0, 1.0/(1.2*1.2);

        upperThres = (origSigma * sigmaTol).cwise().max
                        ( origSigma + Matrix2d::Constant(0.05 * (sigmaTol-1)) );
        lowerThres = (origSigma / sigmaTol).cwise().min
                        ( origSigma + Matrix2d::Constant(-0.05 * (sigmaTol-1)) );
    }

    bool goodFit(const RawFit &rf) const throw() {
        bool amp_thres_exceeded = rf.amp > amp_thres;
        bool dist_OK = sqr(rf.center.x())+sqr(rf.center.y()) 
                           < sqr(maxDistance);
        bool corr_OK = !RawFit::variableStandardDeviation() ||
                ((rf.correlation.cwise() <= upperThres).all() &&
                (rf.correlation.cwise() >= lowerThres).all());
        return amp_thres_exceeded && dist_OK && corr_OK;
    }

    bool trueFit(const RawFit &rf) const throw() { return rf.isTrue; }

    bool truePositive(const RawFit& rf) const throw() 
        { return goodFit(rf) && trueFit(rf); }
    bool falsePositive(const RawFit& rf) const throw() 
        { return goodFit(rf) && !trueFit(rf); }
    bool falseNegative(const RawFit& rf) const throw() 
        { return !goodFit(rf) && trueFit(rf); }
    bool trueNegative(const RawFit& rf) const throw() 
        { return !goodFit(rf) && !trueFit(rf); }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class isTP : public unary_function<bool, const RawFit&> {
  private:
    const FitJudger& judger;
  public:
    isTP(const FitJudger& j) : judger(j) {}
    bool operator()(const RawFit& f) const throw() 
        { return judger.truePositive(f); }
};

class isNotTP : public unary_function<bool, const RawFit&> {
  private:
    const FitJudger& judger;
  public:
    isNotTP(const FitJudger& j) : judger(j) {}
    bool operator()(const RawFit& f) const throw() 
        { return !judger.truePositive(f); }
};

class isTN : public unary_function<bool, const RawFit&> {
  private:
    const FitJudger& judger;
  public:
    isTN(const FitJudger& j) : judger(j) {}
    bool operator()(const RawFit& f) const throw() 
        { return judger.trueNegative(f); }
};

class isFP : public unary_function<bool, const RawFit&> {
  private:
    const FitJudger& judger;
  public:
    isFP(const FitJudger& j) : judger(j) {}
    bool operator()(const RawFit& f) const throw() 
        { return judger.falsePositive(f); }
};

class isFN : public unary_function<bool, const RawFit&> {
  private:
    const FitJudger& judger;
  public:
    isFN(const FitJudger& j) : judger(j) {}
    bool operator()(const RawFit& f) const throw() 
        { return judger.falseNegative(f); }
};

