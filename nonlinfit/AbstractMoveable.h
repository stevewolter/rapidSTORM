#include "nonlinfit/Evaluation.h"

namespace nonlinfit {

struct AbstractMoveable {
    typedef Position_ Position;
    virtual ~AbstractMoveable() {}

    /** Store the variable values of the Lambda in the provided vector. */
    virtual void get_position( Position& ) const = 0;
    /** Change the variable values of the Lambda to the provided values. */
    virtual void set_position( const Position& ) = 0;
};

template <typename Moveable_>
struct get_abstract_moveable {
    typedef AbstractMoveable< typename Moveable_::Position > type;
};

template <typename Lambda_, class Number>
struct get_abstract_moveable_from_lambda {
    typedef AbstractMoveable< typename nonlinfit::get_evaluation<Lambda_,Number>::type::Vector > type;
};

template <typename Moveable_>
struct AbstractMoveableAdapter 
: public get_abstract_moveable<Moveable_>::type
{
    Moveable_ &m;
  public:
    AbstractMoveableAdapter( Moveable_& m ) : m(m) {}
    typedef typename Moveable_::Position Position;

    void get_position( Position& p ) const { m.get_position(p); }
    /** Change the variable values of the Lambda to the provided values. */
    void set_position( const Position& p ) { m.set_position(p); }

    typedef typename get_abstract_moveable<Moveable_>::type abstract_type;
    abstract_type& abstract() { return *this; }
};

template <typename Moveable_>
struct AbstractedMoveable
: public Moveable_,
  public AbstractMoveableAdapter<Moveable_>
{
    template <typename T>
    AbstractedMoveable( T& t ) : Moveable_(t), 
                                 AbstractMoveableAdapter<Moveable_>( static_cast<Moveable_&>(*this) ) {}

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
