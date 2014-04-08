#ifndef DSTORM_FITJUDGERFACTORY_H
#define DSTORM_FITJUDGERFACTORY_H

#include <memory>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/camera/intensity.hpp>
#include <simparm/Choice.h>

#include "engine/JobInfo_decl.h"
#include "engine/Image.h"
#include "engine/FitJudger.h"
#include "output/Basename.h"
#include "output/Traits_decl.h"

namespace dStorm {
namespace engine {

class InputPlane;

struct FitJudgerFactory : public simparm::Choice {
    typedef boost::units::quantity< boost::units::camera::intensity > CountsPerPhoton;
    virtual ~FitJudgerFactory() {}
    FitJudgerFactory* clone() const = 0;
    virtual std::auto_ptr<FitJudger> make_fit_judger( const InputPlane& ) const = 0;
    virtual void set_variables( output::Basename& ) const = 0;
};

std::auto_ptr< FitJudgerFactory > make_fixed_threshold_judger();
std::auto_ptr< FitJudgerFactory > make_square_root_ratio_judger();

}
}

#include "make_clone_allocator.hpp"
DSTORM_MAKE_BOOST_CLONE_ALLOCATOR(dStorm::engine::FitJudgerFactory)

#endif
