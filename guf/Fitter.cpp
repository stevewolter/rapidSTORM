#include <Eigen/StdVector>
#include "debug.h"
#include "guf/Fitter.h"
#include "guf/Config.h"
#include <boost/static_assert.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>
#include "image/crop.h"
#include "image/constructors.h"
#include "engine/Spot.h"
#include "Localization.h"
#include <boost/bind/bind.hpp>
#include <nonlinfit/levmar/exceptions.h>
#include "fit_window/fit_position_out_of_range.h"
#include "engine/InputTraits.h"
#include "engine/Image.h"
#include "fit_window/Plane.h"

#include <nonlinfit/plane/DisjointData.h>
#include <nonlinfit/plane/JointData.h>

namespace dStorm {
namespace guf {

using guf::Spot;

Fitter::Fitter(
    const dStorm::engine::JobInfo& info,
    const Config& config)
: info(info),
  data_creator( config.fit_window_config, this->info.traits, desired_fit_window_widths(config), config.double_computation() ? 0 : 1 ),
  initial_value_finder( config, this->info ),
  one_kernel_fitter( config, info, 1 ),
  create_localization( config, this->info ),
  is_good_localization( config, this->info ),
  add_new_kernel(),
  first_plane_optics(this->info.traits.optics(0)),
  mle( config.mle_fitting() ), 
  two_kernel_analysis( config.two_kernel_fitting() )
{
    if (config.two_kernel_fitting()) {
        two_kernels_fitter = boost::in_place(config, info, 2);
    }
}

int Fitter::fitSpot(
    const engine::FitPosition& spot, 
    const engine::ImageStack &im,
    iterator target 
) {
    try {
        fit_window::PlaneStack data =
            data_creator.cut_region_of_interest( im, spot );

        DEBUG("Fitting at " << spot.transpose() );
        Spot centroid_storage;
        Spot* centroid = (two_kernel_analysis) ? &centroid_storage : nullptr;
        MultiKernelModelStack& one_kernel = one_kernel_fitter.fit_position();
        double mle_result = 0;
        double improvement = 0;
        double r_value = 0;
        initial_value_finder( one_kernel, spot, data );
        double lsq_result = one_kernel_fitter.fit( data, false, centroid, &r_value );
        if ( ! is_good_localization( one_kernel, spot ) ) { DEBUG("No good spot"); return -1; }
        if ( mle )
            mle_result = one_kernel_fitter.fit( data, true, centroid, &r_value );
        if ( two_kernel_analysis ) {
            try {
                MultiKernelModelStack& two_kernel_model = two_kernels_fitter->fit_position();
                add_new_kernel( two_kernel_model, one_kernel, *centroid);
                double two_kernel_result = two_kernels_fitter->fit( data, false, nullptr, &r_value );
                if ( is_good_localization( two_kernel_model, spot ) )
                    improvement = 1.0 - two_kernel_result / lsq_result;
            } catch ( const nonlinfit::levmar::SingularMatrix&) {
                improvement = 0;
            } catch ( const nonlinfit::levmar::InvalidStartPosition& s ) {
                assert( false );
                improvement = 0;
            } catch ( const fit_window::fit_position_out_of_range& ) {
                throw std::logic_error("Fit position out of range for two-kernel fit although it was in range for one-kernel");
            }
        } 
            
        double result = (mle) ? mle_result : lsq_result;
        Localization& loc = *target;
        create_localization( loc, one_kernel, result, data );
        loc.frame_number = im.frame_number();
        loc.two_kernel_improvement = improvement;
        loc.coefficient_of_determination = r_value;
        return 1;
    } catch ( const fit_window::fit_position_out_of_range& ) {
        DEBUG("Fit position is out of range");
        return 0;
    } catch (const nonlinfit::levmar::InvalidStartPosition&) {
        DEBUG("Invalid start position");
        return 0;
    } catch (const nonlinfit::levmar::SingularMatrix&) {
        DEBUG("Singular fit matrix");
        return 0;
    }
}

}
}
