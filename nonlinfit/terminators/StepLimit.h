#ifndef NONLINFIT_TERMINATORS_STEPLIMIT_H
#define NONLINFIT_TERMINATORS_STEPLIMIT_H

#include "nonlinfit/Terminator.h"

namespace nonlinfit {
namespace terminators {

/** Continue fitting until the a pre-set number of positions has
 *  been tried. */
class StepLimit : public Terminator {
    int steps_left;
    bool converged;
  public:
    StepLimit( int limit ) : steps_left(limit), converged(false) {}

    void matrix_is_unsolvable() {}
    void improved( bool negligible_step ) { --steps_left; converged = negligible_step; }
    void failed_to_improve( bool ) { --steps_left; converged = false; }
    bool should_continue_fitting() const { return steps_left > 0 && !converged; }
};

}
}

#endif
