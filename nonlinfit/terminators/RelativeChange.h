#ifndef NONLINFIT_TERMINATORS_RELATIVECHANGE_H
#define NONLINFIT_TERMINATORS_RELATIVECHANGE_H

namespace nonlinfit {
namespace terminators {

/** Continue fitting until the relative change falls below a threshold. */
class RelativeChange {
    double limit; 
    bool converged;
  public:
    RelativeChange( double limit ) : limit(limit), converged(false) {}

    void matrix_is_unsolvable() {}
    template <typename Position>
    void improved( const Position& current, const Position& shift ) {
        converged = ( (shift.array().abs() / current.array()).maxCoeff() < limit );
    }
    void failed_to_improve( bool ) {}
    bool should_continue_fitting() const { return ! converged; }
};

}
}

#endif
