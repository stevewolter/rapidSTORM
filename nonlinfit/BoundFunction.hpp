#ifndef NONLINFIT_BOUNDFUNCTION_HPP
#define NONLINFIT_BOUNDFUNCTION_HPP

#include "nonlinfit/BoundFunction.h"

namespace nonlinfit {

template <typename Function>
BoundFunction<Function>::BoundFunction()
: f(m)
{
    f.set_data( d );
}

template <typename Function>
BoundFunction<Function>::BoundFunction( const Function& f, const Lambda& m, const Data& d )
: f(f), m(m), d(d)
{
    f.set_data( d );
}

}

#endif
