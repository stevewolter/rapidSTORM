#define LOCPREC_FLUOROPHORE_CPP
#include "Fluorophore.h"

#include <CImgBuffer/Image.h>
#include "pixelatedBessel.h"
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf.h>

#include <iostream>

using namespace std;

namespace locprec {

#define NAME "Fluorophore"
#define DESC "Fluorophore characteristics"

_FluorophoreConfig::_FluorophoreConfig() throw() 
: simparm::ConfigObject(NAME, DESC),
  countsPerPhoton("ADCountsPerPhoton", "Camera A/D counts per photon", 16),
  averageActivationTime("AverageActivationTime", "Average time a deactivated "
                        "fluorophore stays deactivated", 2),
  averageDeactivationTime("AverageDeactivationTime", "Average time an activated "
                          "fluorophore stays activated", 0.1),
  photonEmittanceRate("PhotonEmittanceRate", "Photons a fluorophore emits per second",
                      10000),
  sigmaX("OpticSigmaX", "Simulated sigma in X direction", 1.8),
  sigmaY("OpticSigmaY", "Simulated sigma in Y direction", 1.8),
  corrXY("OpticSigmaXY", "Simulated correlation between X and Y "
                         "direction", 0)
{
}

FluorophoreConfig::FluorophoreConfig() throw()
: simparm::ConfigObject(NAME, DESC)
{ registerNamedEntries(); }

FluorophoreConfig::FluorophoreConfig(const FluorophoreConfig& c) throw()
: simparm::ConfigObject(c), _FluorophoreConfig(c)
{ registerNamedEntries(); }

void FluorophoreConfig::registerNamedEntries() throw() {
    register_entry(&countsPerPhoton);
    register_entry(&averageActivationTime);
    register_entry(&averageDeactivationTime);
    register_entry(&photonEmittanceRate);
    register_entry(&sigmaX);
    register_entry(&sigmaY);
    register_entry(&corrXY);
}

Fluorophore::Fluorophore(double posX, double posY, int noImages,
                         const FluorophoreConfig& config)
                         throw()
: posX(posX), posY(posY), isOn(false), restTime(0)
{
    photonWeight = config.countsPerPhoton();
    activateProb = config.averageActivationTime();
    deactivateProb = config.averageDeactivationTime();
    photonRate = config.photonEmittanceRate();
    double sigmaX = config.sigmaX(), sigmaY = config.sigmaY(),
           corrXY = config.corrXY();

    history.resize(noImages, false);

    xr = int(ceil(5 * sigmaX));
    yr = int(ceil(5 * sigmaY));
    densities = gsl_matrix_calloc(xr*2+1, yr*2+1);

    xl = int(round(posX)), yl = int(round(posY));
#define USE_BESSEL
#ifdef USE_ERF
    for (int dx = -xr; dx <= +xr; dx++) {
        double txl = (dx+xl-0.5 - posX) / (2 * sigmaX),
               txh = (dx+xl+0.5 - posX) / (2 * sigmaX);
        double fac = 0.25 * (gsl_sf_erfc(txl) - gsl_sf_erfc(txh));

        for (int dy = -yr; dy <= +yr; dy++) {
            double tyl = (dy+yl-0.5 - posY) / (2 * sigmaY),
                   tyh = (dy+yl+0.5 - posY) / (2 * sigmaY);
            double val = fac * (gsl_sf_erfc(tyl) - gsl_sf_erfc(tyh));

            gsl_matrix_set( densities, dx+xr, dy+yr, val );
        }
    }
#endif
#ifdef USE_BESSEL
    const double naturalBesselSigma = 1.285, naturalBesselOffset = 3.4E-2;
    const double xsc = (sigmaX - naturalBesselOffset)/naturalBesselSigma, 
                 ysc = (sigmaY - naturalBesselOffset)/naturalBesselSigma;
    const double xo = xl-posX, yo = yl-posY;

    double total = 0;
    for (int dx = -xr; dx <= +xr; dx++) {
        for (int dy = -yr; dy <= +yr; dy++) {
            double val = 
                integratedBesselFunction( dx+xo, dy+yo, xsc, ysc, corrXY );
            total += val;
            gsl_matrix_set( densities, dx+xr, dy+yr, val);
        }
    }
    for (int dx = 0; dx <= 2*xr; dx++)
        for (int dy = 0; dy <= 2*yr; dy++) {
            double val = gsl_matrix_get( densities, dx, dy ) / total;
            gsl_matrix_set( densities, dx, dy, val );
        }
#endif
}

Fluorophore::~Fluorophore() throw() {
    gsl_matrix_free(densities);
}

void Fluorophore::glareInImage(gsl_rng *rng, 
         dStorm::Image &targetImage, double integrationTime) throw() 
{
    unsigned int number = targetImage.getImageNumber();
    history[number] = false;
    if (number == 0) /* Do initialization */ {
        double activationHurdle = 
            (gsl_rng_uniform(rng) * (activateProb + deactivateProb));
        isOn = (activationHurdle < deactivateProb);
        restTime = gsl_ran_exponential(rng, 
            (isOn) ? deactivateProb : activateProb);
    }

    while (integrationTime > 0) {
        if (isOn) {
            double glareTime = min<double>(restTime, integrationTime);

            double totalPhotons = glareTime * photonRate;
            double photonPart;
            unsigned int photonCount, photonSum = 0;

            for (int x = -xr; x <= xr; x++) {
                if (x+xl < 0)                            continue;
                else if (x+xl >= int(targetImage.width)) break;

                for (int y = -yr; y <= yr; y++) {
                    if (y+yl < 0)                             continue;
                    else if (yl+y >= int(targetImage.height)) break;

                    photonPart = gsl_matrix_get( densities, x+xr, y+yr );
                    photonCount = gsl_ran_binomial(rng, photonPart,
                                            totalPhotons);
                    photonSum += photonCount;
                    targetImage(xl+x, yl+y) += photonWeight * photonCount;
                }
            }

            history[number] = photonSum;
        }

        if (restTime > integrationTime) {
            restTime -= integrationTime;
            integrationTime = 0;
        } else {
            integrationTime -= restTime;
            isOn = ! isOn;
            restTime = gsl_ran_exponential(rng, 
                (isOn) ? deactivateProb : activateProb);
        }
    }
}

}
