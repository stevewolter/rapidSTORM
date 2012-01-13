#ifndef NONLINFIT_TERMINATORS_STEPLIMIT_H
#define NONLINFIT_TERMINATORS_STEPLIMIT_H

namespace nonlinfit {
namespace terminators {

/** Continue fitting until the a pre-set number of positions has
 *  been tried. */
class StepLimit {
    int steps_left;
  public:
    StepLimit( int limit ) : steps_left(limit) {}

    void matrix_is_unsolvable() {}
    template <typename Position>
    void improved( const Position&, const Position& ) { --steps_left; }
    void failed_to_improve( bool ) { --steps_left; }
    bool should_continue_fitting() const { return steps_left > 0; }
};

}
}

#endif
