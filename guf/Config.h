#ifndef DSTORM_GUF_CONFIG_H
#define DSTORM_GUF_CONFIG_H

#include "guf/Config_decl.h"

#include "UnitEntries/Nanometre.h"
#include <simparm/Group.h>
#include <nonlinfit/levmar/Config.h>
#include "fit_window/Config.h"

namespace dStorm {
namespace guf {

/** This class collects configuration options for the GUF fitter. */
struct Config
{
    simparm::Group name_object;
    fit_window::Config fit_window_config;
    simparm::BoolEntry allow_disjoint, double_computation;
    static const int maximum_plane_count = 9;

    Config();
    void attach_ui( simparm::NodeHandle at );
    dStorm::FloatNanometreEntry theta_dist, negligible_x_step;
    simparm::Entry<double> marquardtStartLambda, maximumIterationSteps, relative_epsilon;
    simparm::BoolEntry free_sigmas, output_sigmas, laempi_fit, disjoint_amplitudes, two_kernel_fitting,
       mle_fitting;

    nonlinfit::levmar::Config make_levmar_config() const;

    static std::string getName() { return "GUF"; }
};

}
}

#endif
