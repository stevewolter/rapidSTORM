#ifndef DSTORM_GUF_CONFIG_H
#define DSTORM_GUF_CONFIG_H

#include "Config_decl.h"

#include <dStorm/UnitEntries/Nanometre.h>
#include <simparm/Set.hh>
#include <nonlinfit/levmar/Config.h>
#include "fit_window/Config.h"

namespace dStorm {
namespace guf {

/** This class collects configuration options for the GUF fitter. */
struct Config
{
    simparm::Set name_object;
    fit_window::Config fit_window_config;
    static const int maximum_plane_count = 9;

    Config();
    void attach_ui( simparm::Node& at );
    dStorm::FloatNanometreEntry theta_dist, negligible_x_step;
    simparm::Entry<double> marquardtStartLambda, maximumIterationSteps;
    simparm::BoolEntry free_sigmas, output_sigmas, laempi_fit, disjoint_amplitudes, two_kernel_fitting,
       mle_fitting;

    nonlinfit::levmar::Config make_levmar_config() const;

    static std::string getName() { return "GUF"; }
};

}
}

#endif
