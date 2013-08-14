#ifndef DSTORM_GUF_FITTERMINATOR_H
#define DSTORM_GUF_FITTERMINATOR_H

#include "debug.h"
#include "Config.h"
#include "gaussian_psf/parameters.h"
#include <nonlinfit/index_of.h>
#include <nonlinfit/TermParameter.h>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>
#include "dStorm/LengthUnit.h"

namespace dStorm {
namespace guf {

template <typename Function>
class FitTerminator {
    double minimum_change;
    double relative_change;
    bool converged;

    struct BoundChecker {
        typedef void result_type;

        template <class Kernel, int Dim, typename Position>
        void operator()( nonlinfit::TermParameter<Kernel, gaussian_psf::Mean<Dim> > param,
                         FitTerminator& t, const Position& pos, const Position& shift ) {
            typedef nonlinfit::TermParameter<Kernel, gaussian_psf::Mean<Dim> > MyParameter;
            const int index = nonlinfit::index_of< 
                typename Function::Variables, MyParameter >::value;
            double value( shift[ index ] );
            DEBUG( "Checking convergence of mean " << Dim << " at index " << index << " with change " << value << " against threshold " << t.minimum_change );
            bool this_coordinate_converged = abs(value) < t.minimum_change;
            t.converged = t.converged && this_coordinate_converged;
        }

        template <typename Parameter, typename Position>
        void operator()( Parameter p, FitTerminator& t, const Position& pos, const Position& shift ) {
            const int index = nonlinfit::index_of< 
                typename Function::Variables, Parameter >::value;
            DEBUG( "Checking convergence of parameter at index " << index << 
                   " with relative change " << fabs(shift[index] / pos[index]) << 
                   " against threshold " << 1E-3 );
            t.converged = t.converged && ( fabs(shift[index] / pos[index]) < t.relative_change );
        }
    };

  public:
    FitTerminator( const Config& config ) 
        : minimum_change(ToLengthUnit(config.negligible_x_step())), 
          relative_change( config.relative_epsilon() ), 
          converged(false) {}

    void matrix_is_unsolvable() {}
    template <typename Position>
    void improved( const Position& pos, const Position& shift ) {
        converged = true;
        boost::mpl::for_each< typename Function::Variables >(
            boost::bind( BoundChecker(), _1, boost::ref(*this), boost::cref(pos), boost::cref(shift) ) );
        DEBUG("Function converged: " << converged);
    }
    void failed_to_improve( bool ) {}
    bool should_continue_fitting() const { return !converged; }
};

}
}

#endif
