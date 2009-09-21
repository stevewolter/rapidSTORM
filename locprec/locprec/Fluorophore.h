#ifndef LOCPREC_FLUOROPHORE_H
#define LOCPREC_FLUOROPHORE_H

#include <dStorm/engine/Image.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_matrix.h>
#include <simparm/Set.hh>
#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <simparm/Structure.hh>
#include <vector>
#include <Eigen/Core>
#include <iostream>

namespace locprec {

class _FluorophoreConfig : public simparm::Set {
  protected:
    void registerNamedEntries();
  public:
    _FluorophoreConfig();
    ~_FluorophoreConfig() {}

    simparm::UnsignedLongEntry countsPerPhoton;
    simparm::DoubleEntry averageActivationTime, averageDeactivationTime;
    simparm::DoubleEntry photonEmittanceRate;
    simparm::DoubleEntry sigmaX, sigmaY, corrXY;

};

typedef simparm::Structure<_FluorophoreConfig> FluorophoreConfig;

class Fluorophore {
  public:
    typedef Eigen::Vector2d Position;
  private:
    Position pos;
    bool isOn;
    std::vector<int> history;
    double restTime;
    double activateProb, deactivateProb, photonRate;
    int photonWeight;

    void initTimes( const FluorophoreConfig& c );
    
    Eigen::MatrixXd densities;
    Eigen::Vector2i pixel, range;
  public:
    Fluorophore(const Position& pos, int noImages,
                const FluorophoreConfig&);
    Fluorophore(std::istream&, const FluorophoreConfig&);
    ~Fluorophore();

    /** @return Total number of photons emitted in image. */
    int glareInImage(gsl_rng *, dStorm::Image &targetImage, int imNum,
                      double integrationTime);
    /** @return Total number of photons emitted in image. */
    int wasOnInImage(int index) const { return history[index]; }

    double x() const { return pos[0]; }
    double y() const { return pos[1]; }

    void recenter( Position new_position );

    friend std::ostream& operator<<(std::ostream&, const Fluorophore&);

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}

#endif
