#include "InsertionPoint.h"
#include <dStorm/input/chain/Forwarder.h>

namespace dStorm {

class InsertionPoint
: public input::chain::Forwarder
{
    InsertionPlace p;

    InsertionPoint* clone() const { return new InsertionPoint(*this); }
    void insert_new_node( std::auto_ptr<Link> l, Place p ) {
        if ( p == this->p ) 
            Forwarder::insert_here( l );
        else
            Forwarder::insert_new_node( l, p );
    }

  public:
    InsertionPoint(InsertionPlace p) : p(p) {}
};

std::auto_ptr< input::chain::Link > make_insertion_place_link(InsertionPlace p) {
    return std::auto_ptr< input::chain::Link >( new InsertionPoint(p) );
}

}
