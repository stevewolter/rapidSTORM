#ifndef DSTORM_ENGINE_GAUSSFITTERCONFIG_H
#define DSTORM_ENGINE_GAUSSFITTERCONFIG_H

#include <simparm/Set.hh>
#include <simparm/NumericEntry.hh>
#include <fitter/MarquardtConfig.h>
#include <fitter/residue_analysis/Config.h>

namespace dStorm {
namespace gauss_2d_fitter {

class Config
: public fitter::MarquardtConfig, 
  public fitter::residue_analysis::Config
{
  public:
    Config();
    ~Config();
    void registerNamedEntries();

    /** Limit up to which X-Y correlation is considered negligible. */
    simparm::DoubleEntry sigma_xy_negligible_limit;
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
