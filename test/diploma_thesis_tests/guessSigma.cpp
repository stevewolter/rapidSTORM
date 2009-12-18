#include "SigmaGuesser.h"
#include <dStorm/Car.h>
#include <dStorm/Transmission.h>
#include <locprec/NoiseSource.h>
#include <locprec/RegionSegmenterConfig.h>
#include <gsl/gsl_statistics_double.h>
#include <gsl/gsl_rng.h>

static double new_sigma_x, new_sigma_y, new_sigma_xy;
int totalSamples;
static void (*saveCallback)(double, double, double, int, bool) = NULL;

void set_new_sigmas(double x, double y, double xy, int n, bool converged) throw() {
    totalSamples += n;
    if (converged) {
        new_sigma_x = x;
        new_sigma_y = y;
        new_sigma_xy = xy;
    }
    if (saveCallback) saveCallback(x, y, xy, n, converged);
}

class Terminator : public dStorm::Passmission {
  public:
        ~Terminator() throw() {}
        std::auto_ptr<Passenger> clone() const throw() 
            { return std::auto_ptr<Passenger>(new Terminator()); }
        void announceStormSize(int , int , int ) throw() {}
        Result receiveLocalizations(const dStorm::Localization *, int , const dStorm::Image &) 
                    throw() 
        { 
          if (new_sigma_x < 0 && new_sigma_y < 0) 
            return KeepRunning;
          else
            return StopEngine;
        }
        void deleteAllResults() throw() {}
        void finish() throw() {}
        const char *getName() throw() { return "Terminator"; }
};

class Config : public dStorm::CarConfig, public locprec::RegionSegmenterConfig {
  public:
    void registerEntries(Set &registerAt) throw() {
        dStorm::CarConfig::registerEntries(registerAt);
        locprec::RegionSegmenterConfig::registerEntries(registerAt);
    }
};

int main(int argc, char *argv[]) throw() {
    EntryDouble sigmaStart, sigmaEnd, sigmaStep;
    EntryUnsignedLong trials;
    sigmaStart.setName("SigmaStart");
    sigmaEnd.setName("SigmaEnd");
    sigmaStep.setName("SigmaStep");
    trials.setName("Trials");
    sigmaStart = 1.3;
    sigmaEnd = 1.7;
    sigmaStep = 0.05;
    trials = 50;
    locprec::NoiseMethod<unsigned short>::registerMethod();
    Config config;
    config.registerEntries(config);
    config.register_entry(&sigmaStart);
    config.register_entry(&sigmaEnd);
    config.register_entry(&sigmaStep);
    config.register_entry(&trials);
    config.inputMethod = "Generated";
    config.fixSigma = false;
    config.outputFile = "/dev/null";
    config.readConfig(argc, argv);
    EntryUnsignedLong &randomSeed = 
        (EntryUnsignedLong&)*config["RandomSeed"];
    config.stderr_progress = false;
    int num = trials();
    double x[num], y[num], xy[num], disc[num];

    gsl_rng *rng = gsl_rng_alloc(gsl_rng_mt19937);
    gsl_rng_set(rng, 42);

    saveCallback = dStorm::SigmaGuesser::fitCallback;
    dStorm::SigmaGuesser::fitCallback = set_new_sigmas;

    bool failed = false;
    for (double sigma = sigmaStart(); sigma <= sigmaEnd(); 
                sigma += sigmaStep())
    {
        (EntryDouble&)(*config["OpticSigmaX"]) = sigma;
        (EntryDouble&)(*config["OpticSigmaY"]) = sigma;

        failed = false;
        int back = 0;

        EntryProgress progress;
        progress.makeASCIIBar(cerr);
        config.register_entry(&progress);
        for (int i = 0; i < num; i++) {
            config.sigma_x = sigma * 0.71;
            config.sigma_y = sigma * 1.41;
            config.sigma_xy = 0;

            new_sigma_x = new_sigma_y = new_sigma_xy = -5;
            totalSamples = 0;
            randomSeed = gsl_rng_get(rng);
            dStorm::Car car(config);
            car.addPassenger(new Terminator());
            car.run();
            if (new_sigma_x < 0 || new_sigma_y < 0 || new_sigma_xy < -2) { 
                i--; 
                cerr << "Fail for sigma " << sigma << endl;
            } else {
                x[back] = new_sigma_x;
                y[back] = new_sigma_y;
                xy[back] = new_sigma_xy;
                disc[back] = totalSamples;
                back++;
            }
            progress = double(i+1) / num;
        }

        if (failed) continue;
        double mx = gsl_stats_mean(x, 1, back),
            my = gsl_stats_mean(y, 1, back),
            mxy = gsl_stats_mean(xy, 1, back),
            md = gsl_stats_mean(disc, 1, back);
        double sdx = gsl_stats_sd_m(x, 1, back, mx),
            sdy = gsl_stats_sd_m(y, 1, back, my),
            sdxy = gsl_stats_sd_m(xy, 1, back, mxy),
            sdm = gsl_stats_sd_m(disc, 1, back, md);

        cout << sigma << " " 
            << mx << " " << gsl_stats_min(x, 1, back) << " "
                         << gsl_stats_max(x, 1, back) << " " << sdx << " "
            << my << " " << gsl_stats_min(y, 1, back) << " "
                         << gsl_stats_max(y, 1, back) << " " << sdy << " "
            << mxy << " " << gsl_stats_min(xy, 1, back) << " "
                         << gsl_stats_max(xy, 1, back) << " " << sdxy << " "
            << md << " " << gsl_stats_min(disc, 1, back) << " "
                         << gsl_stats_max(disc, 1, back) << " " << sdm
            << endl;
    }
    return 0;
}
