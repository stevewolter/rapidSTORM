#ifndef DSTORM_FITTER_GUF_LOCALIZATION_CREATOR_H
#define DSTORM_FITTER_GUF_LOCALIZATION_CREATOR_H

#include <boost/optional/optional.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <dStorm/engine/JobInfo_decl.h>
#include <dStorm/Localization_decl.h>
#include "Config_decl.h"
#include "FitAnalysis.h"
#include "Data_fwd.h"

namespace dStorm {
namespace guf {

/** Functor initializing all localization fields from the supplied fit 
 *  position. */
class LocalizationCreator {
    const int fluorophore;
    const bool output_sigmas, laempi_fit;

    void write_parameters( Localization& loc, const FittedPlane&, double chi_sq, const DataPlane& data ) const;
    void join_localizations( Localization& into, const std::vector<Localization>&, bool ) const;
    void compute_uncertainty( Localization& loc, const FittedPlane&, const DataPlane& data ) const;

  public:
    LocalizationCreator( const Config& config, const dStorm::engine::JobInfo& optics );

    void operator()( Localization& loc, const FitPosition&, double chi_sq, const DataCube& ) const;

};

}
}

#endif
