#ifndef DSTORM_FITTER_GUF_LOCALIZATION_CREATOR_H
#define DSTORM_FITTER_GUF_LOCALIZATION_CREATOR_H

#include <boost/optional/optional.hpp>
#include <dStorm/engine/JobInfo_decl.h>
#include <dStorm/Localization_decl.h>
#include "Config_decl.h"

namespace dStorm {
namespace fit_window {
class Plane;
class Stack;
}
namespace guf {

class MultiKernelModel;
class MultiKernelModelStack;

/** Functor initializing all localization fields from the supplied fit 
 *  position. */
class LocalizationCreator {
    const int fluorophore;
    const bool output_sigmas, laempi_fit;

    void write_parameters( Localization& loc, const MultiKernelModel&, double chi_sq, const fit_window::Plane& data ) const;
    void join_localizations( Localization& into, const std::vector<Localization>&, bool ) const;
    void compute_uncertainty( Localization& loc, const MultiKernelModel&, const fit_window::Plane& data ) const;

  public:
    LocalizationCreator( const Config& config, const dStorm::engine::JobInfo& optics );

    void operator()( Localization& loc, const MultiKernelModelStack&, double chi_sq, const fit_window::Stack& ) const;

};

}
}

#endif
