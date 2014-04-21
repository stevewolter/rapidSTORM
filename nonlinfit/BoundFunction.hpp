#ifndef NONLINFIT_BOUNDFUNCTION_HPP
#define NONLINFIT_BOUNDFUNCTION_HPP

#include "nonlinfit/BoundFunction.h"

namespace nonlinfit {

template <typename Function>
BoundFunction<Function>::BoundFunction()
: function(m), mover(m)
{
    function.set_data( d );
}

template <typename Function>
BoundFunction<Function>::BoundFunction( const Function& f, const Lambda& m, const Data& d )
: function(f), m(m), d(d), mover(this->m)
{
    function.set_data( d );
}

}

#endif
