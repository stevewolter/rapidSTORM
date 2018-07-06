#include "input/MetaInfo.h"
#include "signals/InputFileNameChange.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace dStorm {
namespace input {

using namespace signals;

struct MetaInfo::Signals
: public InputFileNameChange
{
    boost::ptr_vector< boost::signals2::scoped_connection > listeners;
};

MetaInfo::MetaInfo() 
: _signals(new Signals())
    {}

MetaInfo::MetaInfo(const MetaInfo& o) 
: _signals(new Signals()),
  _traits(o._traits),
  suggested_output_basename( o.suggested_output_basename ),
  forbidden_filenames( o.forbidden_filenames ),
  accepted_basenames( o.accepted_basenames )
{
    forward_connections(o);
}

MetaInfo::~MetaInfo() 
    {}

template <typename Type>
Type& MetaInfo::get_signal() { return *_signals; }

template InputFileNameChange& MetaInfo::get_signal<InputFileNameChange>();

void MetaInfo::forward_connections( const MetaInfo& s )
{
    s._signals->listeners.push_back( 
        new boost::signals2::scoped_connection( 
            get_signal<InputFileNameChange>().connect( static_cast<InputFileNameChange&>(*s._signals) ) ) );
}

}
}
