#ifndef NONLINFIT_ABSTRACTTERMINATOR_H
#define NONLINFIT_ABSTRACTTERMINATOR_H

#include <Eigen/Core>
#include "nonlinfit/Evaluation.h"

namespace nonlinfit {

struct Terminator {
    typedef typename Evaluation<double>::Vector Position;
    virtual ~Terminator() {}
    virtual void matrix_is_unsolvable() = 0;
    virtual void failed_to_improve( bool valid_position ) = 0;
    virtual void improved( bool negligibly ) = 0;
    virtual bool should_continue_fitting() const = 0;
};

}

#endif
