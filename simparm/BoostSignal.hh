#include "Callback.hh"
#include <boost/signals2/slot.hpp>
#include <boost/signals2/signal.hpp>

namespace simparm {

class BoostSignalAdapter 
: public simparm::Listener 
{
    boost::signals2::signal<void (void)> signal;
public:
    BoostSignalAdapter( Event::Cause limit_to = Event::All )
        : simparm::Listener( limit_to ) {}
    BoostSignalAdapter( const BoostSignalAdapter& o ) 
        : simparm::Listener(o), signal() {}
    void operator()( const Event& ) { signal(); }
    void connect( const boost::signals2::signal<void (void)>::slot_type& slot ) { signal.connect(slot); }
};

}
