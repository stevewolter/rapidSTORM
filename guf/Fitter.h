#include "Config.h"
#include "NaiveFitter.h"
#include <dStorm/engine/SpotFitter.h>
#include <dStorm/traits/optics.h>
#include <boost/smart_ptr/scoped_ptr.hpp>
#include <boost/optional/optional.hpp>
#include "InitialValueFinder.h"
#include "LocalizationCreator.h"
#include "LocalizationChecker.h"
#include "KernelCreator.h"
#include "fit_window/Stack.h"

namespace dStorm {
namespace guf {

class Config;

/** This class integrates all components of the GUF to fulfill the dStorm::engine::spot_fitter::Implementation
 *  contract. It provides the high-level glue code for calling the fit window selection, initialization, 
 *  fitting and localization creation routines. */
class Fitter 
: public engine::spot_fitter::Implementation
{
    const dStorm::engine::InputTraits& traits;
    dStorm::engine::JobInfo info;
    fit_window::StackCreator data_creator;
    InitialValueFinder initial_value_finder;
    boost::scoped_ptr<NaiveFitter > one_kernel_fitter, two_kernels_fitter;
    LocalizationCreator create_localization;
    LocalizationChecker is_good_localization;
    KernelCreator add_new_kernel;
    Eigen::Vector2i mask_size;
    const traits::Optics& first_plane_optics;
    bool mle, two_kernel_analysis;

  public:
    Fitter(
        const dStorm::engine::JobInfo& info,
        const Config& config );
    int fitSpot( const engine::FitPosition& spot, const engine::ImageStack &im,
                 iterator target );
};

}
}
