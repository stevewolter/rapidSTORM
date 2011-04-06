#include "residue_analysis/impl.h"
#include "fitter/residue_analysis/main.h"
#include "no_analysis/impl.h"

namespace dStorm {
namespace fitter {
namespace residue_analysis {
#define INSTANCE(x) \
    template class CommonInfo< \
        gauss_2d_fitter::residue_analysis::CommonInfo<fitpp::Exponential2D:: x > >;
INSTANCE(FreeForm);
INSTANCE(FreeForm_NoCorrelation);
INSTANCE(FixedForm);
#undef INSTANCE
}
}

namespace gauss_2d_fitter {
namespace no_analysis {
#define INSTANCE(x) \
    template class CommonInfo<2, fitpp::Exponential2D:: x >;
INSTANCE(FreeForm);
INSTANCE(FreeForm_NoCorrelation);
INSTANCE(FixedForm);
#undef INSTANCE
}
namespace residue_analysis {

#define INSTANCE(x) \
    template class CommonInfo< fitpp::Exponential2D:: x >;
INSTANCE(FreeForm);
INSTANCE(FreeForm_NoCorrelation);
INSTANCE(FixedForm);
#undef INSTANCE

}
}
}
