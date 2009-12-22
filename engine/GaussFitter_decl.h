#include "SpotFitter.h"
#include <dStorm/engine/Config_decl.h>

namespace dStorm {
namespace engine {
    template <bool Free_Sigmas, bool Residue_Analysis = false, bool Corr = Free_Sigmas>
    class GaussFitter;

    SpotFitter* select_gauss_fitter (const Config& config);
}
}
