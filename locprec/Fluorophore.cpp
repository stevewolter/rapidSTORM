#define LOCPREC_FLUOROPHORE_CPP
#include "Fluorophore.h"

#include "pixelatedBessel.h"
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf.h>
#include <CImg.h>

#include <iostream>
#include <iomanip>

using namespace std;

namespace locprec {

#define NAME "Fluorophore"
#define DESC "Fluorophore characteristics"

_FluorophoreConfig::_FluorophoreConfig() 
: simparm::Set(NAME, DESC),
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

void _FluorophoreConfig::registerNamedEntries() {
    register_entry(&countsPerPhoton);
    register_entry(&averageActivationTime);
    register_entry(&averageDeactivationTime);
    register_entry(&photonEmittanceRate);
    register_entry(&sigmaX);
    register_entry(&sigmaY);
    register_entry(&corrXY);
}

void Fluorophore::initTimes( const FluorophoreConfig& config ) {
    photonWeight = config.countsPerPhoton();
    activateProb = config.averageActivationTime();
    deactivateProb = config.averageDeactivationTime();
    photonRate = config.photonEmittanceRate();
}

Fluorophore::Fluorophore(const Position& pos, int/* noImages*/,
                         const FluorophoreConfig& config)

: pos(pos), isOn(false), restTime(0)
{
    initTimes(config);
    double sigmaX = config.sigmaX(), sigmaY = config.sigmaY(),
           corrXY = config.corrXY();

    //history.resize(noImages, false);

    range[0] = int(ceil(5 * sigmaX));
    range[1] = int(ceil(5 * sigmaY));
    densities = Eigen::MatrixXd(range[0]*2+1, range[1]*2+1);

    for (int r = 0; r < pixel.rows(); r++)
        for (int c = 0; c < pixel.cols(); c++)
            pixel(r,c) = round(pos(r,c));
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

            densities( dx+xr, dy+yr ) = val;
        }
    }
#endif
#ifdef USE_BESSEL
    const double naturalBesselSigma = 1.285, naturalBesselOffset = 3.4E-2;
    const double xsc = (sigmaX - naturalBesselOffset)/naturalBesselSigma, 
                 ysc = (sigmaY - naturalBesselOffset)/naturalBesselSigma;
    const Position offset = pixel.cast<double>() - pos;

    double total = 0;
    for (int dx = -range[0]; dx <= +range[0]; dx++) {
        for (int dy = -range[1]; dy <= +range[1]; dy++) {
            double val = 
                integratedBesselFunction( 
                    dx+offset[0], dy+offset[1], xsc, ysc, corrXY );
            total += val;
            densities( dx+range[0], dy+range[1] ) = val;
        }
    }
    densities /= total;
#endif
}

Fluorophore::~Fluorophore() {
}

int Fluorophore::glareInImage(gsl_rng *rng, 
         dStorm::Image &targetImage, int imNum,
         double integrationTime) 
{
    //history[imNum] = 0;
    if (imNum == 0) /* Do initialization */ {
        double activationHurdle = 
            (gsl_rng_uniform(rng) * (activateProb + deactivateProb));
        isOn = (activationHurdle < deactivateProb);
        restTime = gsl_ran_exponential(rng, 
            (isOn) ? deactivateProb : activateProb);
    }

    unsigned int sum_of_photons = 0;
    while (integrationTime > 0) {
        if (isOn) {
            double glareTime = min<double>(restTime, integrationTime);

            double totalPhotons = glareTime * photonRate;
            double photonPart;
            unsigned int photonCount, photonSum = 0;

            for (int x = -range[0]; x <= range[0]; x++) {
                int xi = x+pixel[0]; 
                if (xi < 0)                            continue;
                else if (xi >= int(targetImage.width)) break;

                for (int y = -range[1]; y <= range[1]; y++) {
                    int yi = y+pixel[1];
                    if (yi < 0)                             continue;
                    else if (yi >= int(targetImage.height)) break;

                    photonPart = densities( x+range[0], y+range[1] );
                    photonCount = gsl_ran_binomial(rng, photonPart,
                                            totalPhotons);
                    photonSum += photonCount;
                    targetImage(xi, yi) += photonWeight * photonCount;
                }
            }

            sum_of_photons += photonSum;
            //history[imNum] = photonSum;
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
    return sum_of_photons;
}

void Fluorophore::recenter( Position np )
{
    Position diff_in_offsets = 
        (np - np.cast<int>().cast<double>()) - 
           (pos - pos.cast<int>().cast<double>());
    if ( diff_in_offsets.squaredNorm() > 0.01 ) {
        std::stringstream error;
        error << "Offset in fluorophore positions is too large: "
              << diff_in_offsets.transpose();
        throw std::logic_error(error.str());
    }

    pos = np;
    for (int r = 0; r < pixel.rows(); r++)
        for (int c = 0; c < pixel.cols(); c++)
            pixel(r,c) = round(pos(r,c));
}

std::ostream& operator<<(std::ostream& o, const Fluorophore& f) {
    return (o << f.pos << " " << f.range << "\n" << f.densities << "\n");
}

template <typename T, int W, int H>
std::istream& operator>>(std::istream& i, Eigen::Matrix<T,W,H>& m) {
    for (int r = 0; r < m.rows(); r++)
      for (int c = 0; c < m.cols(); c++)
        i >> m(r,c);
    return i;
}

Fluorophore::Fluorophore(std::istream& i, const FluorophoreConfig& c) {
    initTimes(c);

    i >> pos;
    for (int j = 0; j < pos.rows(); j++)
        pixel[j] = round(pos[j]);
    i >> range;
    densities = Eigen::MatrixXd(range[0]*2+1, range[1]*2+1);
    i >> densities;
}

}
