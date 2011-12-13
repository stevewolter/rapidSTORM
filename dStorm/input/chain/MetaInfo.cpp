#include "MetaInfo.h"
#include <dStorm/input/InputFileNameChange.h>

namespace dStorm {
namespace input {
namespace chain {

struct MetaInfo::Signals
: public InputFileNameChange
{
};

MetaInfo::MetaInfo() 
: _signals(new Signals())
    {}

MetaInfo::~MetaInfo() 
    {}

template <typename Type>
Type& MetaInfo::get_signal() { return *_signals; }

template InputFileNameChange& MetaInfo::get_signal<InputFileNameChange>();

void MetaInfo::forward_connections( boost::shared_ptr<const MetaInfo> s )
{
    if ( s->_signals != _signals )
        _signals->connect(
            InputFileNameChange::slot_type( *_signals )
            .track( s->_signals ) );
}

}
}
}
