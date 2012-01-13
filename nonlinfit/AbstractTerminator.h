#ifndef NONLINFIT_ABSTRACTTERMINATOR_H
#define NONLINFIT_ABSTRACTTERMINATOR_H

#include <Eigen/Core>

namespace nonlinfit {

template <class Position>
struct AbstractTerminator {
    virtual ~AbstractTerminator() {}
    virtual void matrix_is_unsolvable() = 0;
    virtual void failed_to_improve( bool valid_position ) = 0;
    virtual void improved( const Position& new_position, const Position& shift ) = 0;
    virtual bool should_continue_fitting() const = 0;
};

template <class Terminator, class Position>
class AbstractTerminatorAdaptor
: public AbstractTerminator<Position> 
{
    Terminator& t;
  public:
    AbstractTerminatorAdaptor( Terminator& t ) : t(t) {}
    void matrix_is_unsolvable() 
        { t.matrix_is_unsolvable(); }
    void failed_to_improve( bool valid_position ) 
        { t.failed_to_improve(valid_position); }
    void improved( const Position& new_position, const Position& shift )
        { t.improved( new_position, shift ); }
    bool should_continue_fitting() const
        { return t.should_continue_fitting(); }
};

}

#endif
