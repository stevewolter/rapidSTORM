#ifndef DSTORM_FITJUDGER_H
#define DSTORM_FITJUDGER_H

namespace dStorm {
namespace engine {

struct FitJudger {
    virtual ~FitJudger() {}
    virtual FitJudger* clone() const = 0;
    virtual bool is_above_background( double signal_integral_in_photons, double background_in_photons ) const = 0;
};

}
}

#include <dStorm/make_clone_allocator.hpp>
DSTORM_MAKE_BOOST_CLONE_ALLOCATOR(dStorm::engine::FitJudger)

#endif
