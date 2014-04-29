#include "guf/NaiveFitter.h"

#include "debug.h"
#include "engine/InputTraits.h"
#include "threed_info/No3D.h"

namespace dStorm {
namespace guf {

NaiveFitter::NaiveFitter(
    const Config& config, 
    const dStorm::engine::JobInfo& info,
    int kernels)
: lm(config.make_levmar_config()),
  step_limit(config.maximumIterationSteps())
{
    boost::optional<bool> consistently_no_3d = true;
    consistently_no_3d.reset();
    for (int i = 0; i < info.traits.plane_count(); ++i ) {
        for (Direction dir = Direction_First; dir != Direction_2D; ++dir) {
	    bool is_no3d = dynamic_cast< const threed_info::No3D* >( info.traits.plane(i).optics.depth_info(dir).get() );
            if (consistently_no_3d && *consistently_no_3d != is_no3d) {
                 throw std::runtime_error("3D information is inconsistent between planes");
            }
	    consistently_no_3d = is_no3d;
        }
    }

    for (int i = 0; i < info.traits.plane_count(); ++i ) {
        std::unique_ptr<FitFunctionFactory> creator =
             FitFunctionFactory::create(config, info.traits.plane(i), kernels);
	variable_map.add_function(creator->reduction_bitset());
        model_stack.push_back(creator->fit_position());
        function_creators.push_back(std::move(creator));
    }

    plane_combiner = boost::in_place<nonlinfit::sum::AbstractFunction>(variable_map);
}

double NaiveFitter::fit(fit_window::PlaneStack& data, bool mle) {
    typedef nonlinfit::AbstractFunction<double> AbstractFunction;
    std::vector<std::unique_ptr<nonlinfit::AbstractFunction<double>>> functions;
    for ( typename fit_window::PlaneStack::iterator b = data.begin(), i = b, e = data.end(); i != e; ++i ) {
        functions.push_back(function_creators[i-b]->create_function(*i, mle));
        plane_combiner->set_fitter( i-b, *functions.back() );
    }

    nonlinfit::terminators::StepLimit step_limit(this->step_limit);
    return lm.fit( *plane_combiner, step_limit );
}

}
}
