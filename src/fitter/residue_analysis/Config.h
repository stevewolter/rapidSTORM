#ifndef DSTORM_FITTER_RESIDUEANALYSISCONFIG_H
#define DSTORM_FITTER_RESIDUEANALYSISCONFIG_H

#include <simparm/Set.hh>
#include <simparm/NumericEntry.hh>

namespace dStorm {
namespace fitter {
namespace residue_analysis {

class Config
{
  public:
    inline Config();
    inline ~Config();
    inline void registerNamedEntries(simparm::Node&);

    simparm::BoolEntry do_double_spot_analysis;
    /** Threshold for the spot fitter for the degree of asymmetry
        *  in the residues from which on it will suscept multi-spots. */
    simparm::DoubleEntry asymmetry_threshold;
    /** Required distance between multi-spot peaks. */
    simparm::DoubleEntry required_peak_distance;

};

}
}
}

#endif
