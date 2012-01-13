#ifndef NONLINFIT_XS_HPP
#define NONLINFIT_XS_HPP

#include "Xs.h"
#include <iostream>

namespace nonlinfit {

template <int Dim, typename Length>
inline std::ostream& operator<<(std::ostream& o, Xs<Dim,Length>)  { return (o << "x" << Dim); }

}

#endif
