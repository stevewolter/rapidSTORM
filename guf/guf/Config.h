#ifndef DSTORM_GUF_CONFIG_H
#define DSTORM_GUF_CONFIG_H

#include "Config_decl.h"

#include <dStorm/UnitEntries/Nanometre.h>
#include <simparm/Set.hh>
#include <nonlinfit/levmar/Config.h>

namespace dStorm {
namespace guf {

struct Config
: public simparm::Set
{
    static const int maximum_plane_count = 9;

    Config();
    void registerNamedEntries();
    dStorm::FloatNanometreEntry theta_dist, negligible_x_step;
    simparm::Entry<double> marquardtStartLambda, maximumIterationSteps;
    simparm::BoolEntry free_sigmas, output_sigmas, laempi_fit, disjoint_amplitudes, two_kernel_fitting,
       mle_fitting, allow_disjoint, double_computation;

    nonlinfit::levmar::Config make_levmar_config() const;
};

}
}

#endif
