#include <memory>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <Variance.h>
#include "FitJudger.h"

#include <gsl/gsl_multimin.h>

double RawFit::maxCor00 = 0, RawFit::minCor00 = 1000000;
int RawFit::numTrue = 0;

class DistanceVariance : public dStorm::Variance,
                         public unary_function<void,const RawFit&> 
{
  private:
    Vector2d center;
  public:
    DistanceVariance(const Vector2d& center) throw() : center(center) {}
    void operator()(const RawFit& fit) throw() {
        this->addValue( sqrt(sqr(fit.center.x() - center.x()) +
                       sqr(fit.center.y() - center.y())) );
    }
};

struct OptimalSD {
    int fp, fn;
    double sd;

    OptimalSD(int fp, int fn, double sd) : fp(fp), fn(fn), sd(sd) {}
};

template <typename Data>
OptimalSD optimizeSD(Data& vec, double amp_thres, double maxDev,
                            int target) 
{
    double too_high = 1000.0, too_low = 1.0;
    int bestResult = 0;
    double bestGuess = 0;
    while (true) {
        double guess = (too_high + too_low)/2;
        FitJudger judger(amp_thres, guess, maxDev);
        int result = count_if(vec.begin(), vec.end(), isFN(judger) );
        if (result < target && result > bestResult) {
            bestResult = result;
            bestGuess = guess;
        }

        if ( abs(too_low - too_high) < 0.001 || result == target) {
            if (bestResult > 0 && result > target) {
                result = bestResult;
                guess = bestGuess;
            }
            return OptimalSD(
                     count_if(vec.begin(), vec.end(), isFP(judger)),
                     result, guess);
        } else if (result > target)
            too_low = guess;
        else
            too_high = guess;

    }
}

struct Info {
    std::list<RawFit, Eigen::aligned_allocator<RawFit> >* fits;
    int target;
    double md;
};
double gsl_optimizeSD (const gsl_vector * x, void * params) {
    const Info& i = *(const Info*)params;
    OptimalSD result = optimizeSD( *i.fits, gsl_vector_get(x, 0), 
                                   i.md, i.target );
    double score = result.fp + 1E10 * max(0, result.fn - i.target);
    return score;
}

template <typename Data>
pair<int,Vector2d> optimizeAT(Data& vec, double md, int target) 
{
    Vector2d rv = Vector2d::Zero();
    double too_high = 0, too_low = 10000;
    while (true) {
        double guess = (too_high + too_low)/2;
        FitJudger judger(guess, 5, md);
        int result = count_if(vec.begin(), vec.end(), isFN(judger));

        if ( abs(too_low - too_high) < 0.001 || result == target) {
            rv(1) = guess;
            return make_pair(count_if(vec.begin(), vec.end(),
                                      isFN(judger)), rv);
        }
        else if (result > target)
            too_low = guess;
        else
            too_high = guess;

    }
}

template <typename Data> 
pair<int,Vector2d> optimizeSD_MD(Data& vec, double amp_thres, int target) {
    int optFN = 100000; Vector2d optAT;
    for (double md = 1; md <= 4; md += 0.25) {
        pair<int,double> res = optimizeSD(vec, amp_thres, md, target);
        if ( res.first < optFN) {
            optAT(1) = md;
            optAT(0) = res.second;
            optFN = res.first;
        }
    }
    return make_pair(optFN, optAT);
}

template <typename Data>
double searchATStart(Data &vec, int target) {
    int bestResult = 0;
    double bestAT = 0;

    for (double at = 0; at <= 10000; at += 500) {
        FitJudger judger(at, 2, 2);
        int result = count_if(vec.begin(), vec.end(), isFN(judger) );
        if (bestResult < result && result < target) {
            bestResult = result;
            bestAT = at;
        }
    }

    return bestAT;
}

int main(int argc, char *argv[]) throw() {
    assert( argc == 2 );
    std::list<RawFit, Eigen::aligned_allocator<RawFit> > fits, tpFits;
    while (cin) {
        fits.push_back(RawFit(cin));
    }
    fits.pop_back();

    int fixSigma = ! RawFit::variableStandardDeviation();
    double fnProb = atof(argv[1]);
    int target = (int)round( RawFit::numberOfTrueFits() * fnProb );
    std::auto_ptr<FitJudger> judger;
    /*pair<int,Vector3d> p = optimizeAT(fits, target, fixSigma);
    judger.reset(new FitJudger(p.second(0), p.second(1), p.second(2)));*/

    int n = (fixSigma) ? 0 : 1;
    double sd = 2, md = 4, amp = 0;
    if (n > 0) {
        gsl_multimin_fminimizer* minimizer = 
            gsl_multimin_fminimizer_alloc(gsl_multimin_fminimizer_nmsimplex,
                                        n);
        gsl_vector* x = gsl_vector_alloc(n), *ss = gsl_vector_alloc(n);
        gsl_vector_set(x, 0, searchATStart( fits, target ) );
        gsl_vector_set(ss, 0, 10);
        
        Info info;
        info.fits = &fits;
        info.target = target;
        info.md = md;
        gsl_multimin_function min_func;
        min_func.f = gsl_optimizeSD;
        min_func.n = n;
        min_func.params = &info;
        gsl_multimin_fminimizer_set (minimizer, &min_func, x, ss);

        int lastBetterStep = 0;
        double minimumAtLastBetterStep = 1000000;
        for (int i = 0; i < 100; i++) {
            gsl_multimin_fminimizer_iterate(minimizer);

            double minimum = gsl_multimin_fminimizer_minimum(minimizer);
            if (minimum + 0.1 < minimumAtLastBetterStep) {
                lastBetterStep = i;
                minimumAtLastBetterStep = minimum;
            }
            x = gsl_multimin_fminimizer_x(minimizer);
            if ( lastBetterStep + 10 < i )
                break;
        }

        gsl_vector* m = gsl_multimin_fminimizer_x(minimizer);
        sd = (!fixSigma) ? optimizeSD(fits, gsl_vector_get(m, 0), info.md, target).sd : 2;
        //double bestMD = ((fixSigma) ? optimizeMD(fits, gsl_vector_get(m,0), target).second(1) : 1);
        amp = gsl_vector_get(m,0);
    } else {
        pair<int,Vector2d> r = optimizeAT(fits, md, target);
        amp = r.second(1);
    }
        
    cout << amp << " " << md << " " << sd << endl;
    
    exit(0);
}
