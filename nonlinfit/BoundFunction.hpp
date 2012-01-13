#ifndef NONLINFIT_BOUNDFUNCTION_HPP
#define NONLINFIT_BOUNDFUNCTION_HPP

#include "BoundFunction.h"

namespace nonlinfit {

template <typename Function>
BoundFunction<Function>::BoundFunction()
: Function(m), mover(m)
{
    Function::set_data( d );
}

template <typename Function>
BoundFunction<Function>::BoundFunction( const Function& f, const Lambda& m, const Data& d )
: Function(f), m(m), d(d), mover(this->m)
{
    Function::set_data( d );
}

}

#endif
