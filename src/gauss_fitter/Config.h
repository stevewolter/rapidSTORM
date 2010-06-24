#ifndef DSTORM_ENGINE_GAUSSFITTERCONFIG_H
#define DSTORM_ENGINE_GAUSSFITTERCONFIG_H

#include <simparm/Set.hh>
#include <simparm/NumericEntry.hh>
#include <dStorm/fitter/MarquardtConfig.h>

namespace dStorm {
namespace 2d_fitter {

class Config : public fitter::MarquardtConfig {
  public:
    Config();
    ~Config();
    void registerNamedEntries();

    /** Limit up to which X-Y correlation is considered negligible. */
    simparm::DoubleEntry sigma_xy_negligible_limit;
    /** Threshold for the spot fitter for the degree of asymmetry
        *  in the residues from which on it will suscept multi-spots. */
    simparm::DoubleEntry asymmetry_threshold;
    /** Required distance between multi-spot peaks. */
    simparm::DoubleEntry required_peak_distance;

    /** If this option is set, the sigma_x/y/xy parameters are fitted
        *  to the spots. */
    simparm::BoolEntry freeSigmaFitting;
    /** If this option is set, the sigma_xy parameter is not fitted
        *  to the spots. */
    simparm::BoolEntry fixCorrelationTerm;
};

}
}

#endif
