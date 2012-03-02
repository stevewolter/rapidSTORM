#ifndef DSTORM_GUF_PSF_IS_PLANE_DEPENDENT_H
#define DSTORM_GUF_PSF_IS_PLANE_DEPENDENT_H

#include <nonlinfit/TermParameter.h>
#include "guf/constant_background.hpp"
#include "parameters.h"

namespace dStorm {
namespace guf {
namespace PSF {

class is_plane_independent 
{
    bool laempi, disamp;
  public:
    typedef bool result_type;

    is_plane_independent( bool disjunct_means, bool disjunct_amplitudes )
        : laempi( disjunct_means ), disamp( disjunct_amplitudes ) {}

    template <int Dim>
    bool operator()( Mean<Dim> ) { return !laempi; }
    bool operator()( Amplitude ) { return !disamp; }
    bool operator()( Prefactor ) { return false; }
    bool operator()( constant_background::Amount ) { return false; }
    template <int Dim>
    bool operator()( BestSigma<Dim> ) { return false; }

    template <class SubFunction, typename Base>
    bool operator()( nonlinfit::TermParameter<SubFunction,Base> ) { return operator()(Base()); }

    template <typename Parameter> 
    bool operator()( Parameter ) { return true; }
};

struct is_fluorophore_dependent 
{
    typedef bool result_type;

    template <int Dim>
    bool operator()( BestSigma<Dim> ) { return true; }
    bool operator()( Prefactor ) { return true; }

    template <class SubFunction, typename Base>
    bool operator()( nonlinfit::TermParameter<SubFunction,Base> ) { return operator()(Base()); }

    template <typename Parameter> 
    bool operator()( Parameter ) { return false; }
};
}
}
}

#endif
