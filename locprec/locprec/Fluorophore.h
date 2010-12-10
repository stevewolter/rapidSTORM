#ifndef LOCPREC_FLUOROPHORE_H
#define LOCPREC_FLUOROPHORE_H

#include <dStorm/Image.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_matrix.h>
#include <ConfigSet.hh>
#include <ConfigEntry.hh>

namespace locprec {

class _FluorophoreConfig : public simparm::ConfigSet {
  public:
    _FluorophoreConfig() throw();
    ~_FluorophoreConfig() throw() {}

    simparm::ConfigEntryUnsignedLong countsPerPhoton;
    simparm::ConfigEntryDouble averageActivationTime, averageDeactivationTime;
    simparm::ConfigEntryDouble photonEmittanceRate;
    simparm::ConfigEntryDouble sigmaX, sigmaY, corrXY;

};

class FluorophoreConfig : public _FluorophoreConfig {
  protected:
    void registerNamedEntries() throw();
  
  public:
    FluorophoreConfig() throw();
    FluorophoreConfig(const FluorophoreConfig& c) throw();
};

class Fluorophore {
  private:
    double posX, posY;
    bool isOn;
    std::vector<int> history;
    double restTime;
    double activateProb, deactivateProb, photonRate;
    int photonWeight;
    
    gsl_matrix *densities;
    int xl, xr, yl, yr;
    Fluorophore(const Fluorophore&) throw();

  public:
    Fluorophore(double posX, double posY, int noImages,
                const FluorophoreConfig&) throw();
    ~Fluorophore() throw();

    void glareInImage(gsl_rng *, dStorm::Image &targetImage,
                      double integrationTime) throw();
    /** @return Total number of photons emitted in image. */
    int wasOnInImage(int index) const throw() { return history[index]; }

    double x() const throw() { return posX; }
    double y() const throw() { return posY; }
};

}

#endif
