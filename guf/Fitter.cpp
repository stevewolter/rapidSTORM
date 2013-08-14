#include <Eigen/StdVector>
#include "debug.h"
#include "Fitter.h"
#include "Config.h"
#include <boost/static_assert.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>
#include <dStorm/image/crop.h>
#include <dStorm/image/constructors.h>
#include <dStorm/engine/Spot.h>
#include <dStorm/Localization.h>
#include <boost/units/cmath.hpp>
#include <boost/bind/bind.hpp>
#include <nonlinfit/levmar/exceptions.h>
#include "fit_window/fit_position_out_of_range.h"
#include "fit_window/Centroid.h"
#include <dStorm/engine/InputTraits.h>
#include <dStorm/engine/Image.h>
#include "fit_window/Stack.hpp"

#include "EvaluationTags.h"
#include <nonlinfit/plane/DisjointData.hpp>
#include <nonlinfit/plane/JointData.hpp>

namespace dStorm {
namespace guf {

using guf::Spot;

Fitter::Fitter(
    const dStorm::engine::JobInfo& info,
    const Config& config
)
: traits(info.traits),
  info(info),
  data_creator( config.fit_window_config, this->info, evaluation_tags(), MaxWindowWidth ),
  initial_value_finder( config, this->info ),
  one_kernel_fitter( NaiveFitter::create<1>(config, info) ),
  two_kernels_fitter( ( config.two_kernel_fitting() ) ? NaiveFitter::create<2>(config, info).release() : NULL ),
  create_localization( config, this->info ),
  is_good_localization( config, this->info ),
  add_new_kernel(),
  first_plane_optics(this->info.traits.optics(0)),
  mle( config.mle_fitting() ), 
  two_kernel_analysis( config.two_kernel_fitting() )
{
}

int Fitter::fitSpot(
    const engine::FitPosition& spot, 
    const engine::ImageStack &im,
    iterator target 
) {
    try {
        boost::scoped_ptr< fit_window::Stack > 
            data( data_creator.set_image( im, spot ) );

        DEBUG("Fitting at " << spot.transpose() );
        MultiKernelModelStack& one_kernel = one_kernel_fitter->fit_position();
        double mle_result = 0;
        double improvement = 0;
        initial_value_finder( one_kernel, spot, *data );
        double lsq_result = one_kernel_fitter->fit( *data, false );
        if ( ! is_good_localization( one_kernel, spot ) ) { DEBUG("No good spot"); return -1; }
        if ( mle )
            mle_result = one_kernel_fitter->fit( *data, true );
        if ( two_kernel_analysis ) {
            try {
                MultiKernelModelStack& two_kernel_model = two_kernels_fitter->fit_position();
                Spot centroid = data->residue_centroid().current_position();
                add_new_kernel( two_kernel_model, one_kernel, centroid);
                double two_kernel_result = two_kernels_fitter->fit( *data, false );
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
        create_localization( loc, one_kernel, result, *data );
        loc.frame_number = im.frame_number();
        loc.two_kernel_improvement = improvement;
        if ( loc.children.is_initialized() )
            for ( std::vector<Localization>::iterator i = loc.children->begin(); i != loc.children->end(); ++i )
                i->frame_number = im.frame_number();
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
