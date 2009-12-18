#include "SpotFitter.h"
#include "locprec/Fluorophore.h"
#include "locprec/NoiseGenerator.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_statistics_double.h>
#include "Variance.h"
#include "SigmaFitter.h"
#include <dStorm/ImageFitter.h>
#include <Eigen/Array>

using namespace dStorm;
using namespace libfitpp;
using namespace Eigen;

static const double sigmaTol = 4;

class UnjudgedFitResult {
  public:
    Localization localization;
    Matrix2d correlations;
    int photonCount;

    void print(bool trueFit, ostream &o) throw() {
        o << trueFit << " "
          << localization.getImageNumber() << " " 
             << localization.x() << " " << localization.y() << " "
             << localization.getStrength() << " "
             << photonCount << " "
          << correlations.row(0) << " " << correlations.row(1) << " "
          << endl;
    }


    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class Localizator {
  public:
    virtual ~Localizator() throw() {}
    virtual UnjudgedFitResult localize(double xc, double yc, 
                          const dStorm::Config &config,
                          const dStorm::Image& img) = 0;
};

template <typename Number>
Number sqr(const Number &i) throw() { return i*i; }

template <bool fixCorr>
class GaussFreeFitter : public Exponential2D<StormPixel,fixCorr?6:7>,
                        public Localizator
{
  public:
    GaussFreeFitter(const dStorm::Config &config) throw() 
    : Exponential2D<StormPixel,fixCorr?6:7>
        (2*config.fitWidth()+1, 2*config.fitHeight()+1)
    {
        for (int i = 0; i < 7; i++)
            this->setVariableness(i, (!fixCorr) || (i != this->SigmaXY));
    }

    UnjudgedFitResult localize(double xc, double yc, 
                          const dStorm::Config &config,
                          const dStorm::Image& img) throw() 
    {
        int xl,yl;
        setSigmaX(config.sigma_x());
        setSigmaY(config.sigma_y());
        setSigmaXY(config.sigma_xy());
        fitToImage<StormPixel,(fixCorr?6:7)>(img, *this, xc, yc, xl, yl);
        UnjudgedFitResult rv;
        rv.localization.setStrength( this->getAmplitude() );
        rv.localization.setX( this->getMeanX(), xl );
        rv.localization.setY( this->getMeanY(), yl );
        rv.localization.setImageNumber( img.getImageNumber() );

        Eigen::Matrix2d newCovar;
        for (int x = 0; x < 2; x++)
            for (int y = 0; y < 2; y++) {
                double v = (x==y) ? 1 : -this->getSigmaXY();
                v /= (x==0) ? this->getSigmaX() : this->getSigmaY();
                v /= (y==0) ? this->getSigmaX() : this->getSigmaY();
                newCovar(x,y) = v;
            }
        rv.correlations = newCovar;

        return rv;
    }

};

class GaussFixedFitter : public Localizator {
  private:
    std::auto_ptr<dStorm::SpotFitter> fitter;
    Matrix2d covar;
  public:
    GaussFixedFitter(const dStorm::Config &config) throw() 
    : fitter( dStorm::SpotFitter::factory(config) )
    {
        for (int x = 0; x < 2; x++)
            for (int y = 0; y < 2; y++) {
                double v = (x==y) ? 1 : -config.sigma_xy();
                v /= (x==0) ? config.sigma_x() : config.sigma_y();
                v /= (y==0) ? config.sigma_x() : config.sigma_y();
                covar(x,y) = v;
            }
    }
    ~GaussFixedFitter() throw() {}

    UnjudgedFitResult localize(double xc, double yc, 
                          const dStorm::Config &,
                          const dStorm::Image& img) throw() 
    {
        UnjudgedFitResult rv;
        fitter->prepareToFitImage(img);
        fitter->fitSpot( Spot(xc,yc), rv.localization );
        rv.localization.setImageNumber( img.getImageNumber() );
        rv.correlations = covar;
        return rv;
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class Config : public dStorm::Config, public locprec::FluorophoreConfig,
               public locprec::NoiseGeneratorConfig
{
  public:
    EntryBool measurePrecision, freeForm, fixedCorr, noNoise;
    EntryProgress progress;
    EntryFile output;

    Config() throw()
    : Set(),
      measurePrecision("MeasurePrecision", "Output precision estimate"),
      freeForm("FreeForm", "Localization with free form PSF"),
      fixedCorr("FixXY", "Fix and do not fit X/Y correlation term", false),
      noNoise("NoNoise", "Do not generate pixel noise"),
      progress("Progress", "Progress"),
      output("PrintRaw", "")
    {
    }

    virtual ~Config() throw() {}

    void registerEntries(Set &at) throw() {
        dStorm::Config::registerEntries(at);
        locprec::FluorophoreConfig::registerEntries(at);
        locprec::NoiseGeneratorConfig::registerEntries(at);
        at.register_entry(&measurePrecision);
        at.register_entry(&freeForm);
        at.register_entry(&fixedCorr);
        at.register_entry(&noNoise);
        at.register_entry(&progress);
        at.register_entry(&output);
    }
};

int main(int argc, char *argv[]) throw() {
    ::Config conf;
    conf.registerEntries(conf);
    conf.readConfig(argc, argv);
    int msx = conf.fitWidth(), msy = conf.fitHeight();
    int n = 100000;

    msx = max<int>(conf.fitWidth(), round(conf.sigmaX()*3));
    msy = max<int>(conf.fitHeight(), round(conf.sigmaY()*3));
    Variance sigmaX, sigmaY, ssX, ssY;

    conf.progress.makeASCIIBar(cerr);

    int numRuns = (conf.measurePrecision) ? 10 : 1;

    for (int j = 0; j < numRuns; j++) {
        gsl_rng *rng = gsl_rng_alloc(gsl_rng_mt19937);
        gsl_rng_set(rng, conf.random_seed() + j);
        dStorm::Image image(2*msx+1, 2*msy+1, 0);

        std::auto_ptr<locprec::NoiseGenerator<StormPixel> > noiseGen;
        if (! conf.noNoise)
            noiseGen = locprec::NoiseGenerator<StormPixel>::factory(conf, rng);
        std::auto_ptr<Localizator> fitter
            ((!conf.freeForm()) 
             ? (Localizator*)new GaussFixedFitter(conf)
             : (conf.fixedCorr) 
                ? (Localizator*)new GaussFreeFitter<true>(conf)
                : (Localizator*)new GaussFreeFitter<false>(conf));

        double fx = 0, fy = 0;
        do {
            fx = gsl_rng_uniform(rng)*2-1;
            fy = gsl_rng_uniform(rng)*2-1;
        } while ( fx*fx + fy*fy > 1*1 );
        locprec::Fluorophore fluorophore(msx+fx, msy+fy, n, conf);

        for (int i = 0; i < n; i++) {
            image.setImageNumber(i);
            if (noiseGen.get())
                noiseGen->pixelNoise(image.ptr(), image.size());
            else
                image.fill(0);
            fluorophore.glareInImage(rng, image, 0.1);
            int realPhotons = fluorophore.wasOnInImage(i);
            bool should = (realPhotons > 30);

            UnjudgedFitResult r = fitter->localize(msx, msy, conf, image);
            r.localization.setX( r.localization.x()-(msx+fx) );
            r.localization.setY( r.localization.y()-(msy+fy) );
            r.photonCount = realPhotons;
            r.print( should, conf.output.getOutputStream() );
        }
        conf.progress = (1.0 + j) / numRuns;
    }

    return 0;
}
