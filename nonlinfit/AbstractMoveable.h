#ifndef NONLINFIT_ABSTRACTMOVEABLE_H
#define NONLINFIT_ABSTRACTMOVEABLE_H

#include "nonlinfit/Evaluation.h"

namespace nonlinfit {

template <typename Number>
struct AbstractMoveable {
    typedef typename Evaluation<Number>::Vector Position;
    virtual ~AbstractMoveable() {}

    /** Store the variable values of the Lambda in the provided vector. */
    virtual void get_position( Position& ) const = 0;
    /** Change the variable values of the Lambda to the provided values. */
    virtual void set_position( const Position& ) = 0;
};

}

#endif
