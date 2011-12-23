#include "MetaInfo.h"
#include <dStorm/input/InputFileNameChange.h>
#include <dStorm/input/ResolutionChange.h>
#include <dStorm/input/BasenameChange.h>
#include <dStorm/signals/UseSpotFinder.h>
#include <dStorm/signals/UseSpotFitter.h>
#include <boost/ptr_container/ptr_vector.hpp>

namespace dStorm {
namespace input {
namespace chain {

using namespace signals;

struct MetaInfo::Signals
: public InputFileNameChange,
  public ResolutionChange,
  public BasenameChange,
  public UseSpotFinder,
  public UseSpotFitter
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
template ResolutionChange& MetaInfo::get_signal<ResolutionChange>();
template BasenameChange& MetaInfo::get_signal<BasenameChange>();
template UseSpotFinder& MetaInfo::get_signal<UseSpotFinder>();
template UseSpotFitter& MetaInfo::get_signal<UseSpotFitter>();

void MetaInfo::forward_connections( const MetaInfo& s )
{
    s._signals->listeners.push_back( 
        new boost::signals2::scoped_connection( 
            get_signal<InputFileNameChange>().connect( static_cast<InputFileNameChange&>(*s._signals) ) ) );
    s._signals->listeners.push_back( 
        new boost::signals2::scoped_connection( 
            get_signal<ResolutionChange>().connect( static_cast<ResolutionChange&>(*s._signals) ) ) );
    s._signals->listeners.push_back( 
        new boost::signals2::scoped_connection( 
            get_signal<BasenameChange>().connect( static_cast<BasenameChange&>(*s._signals) ) ) );
    s._signals->listeners.push_back( 
        new boost::signals2::scoped_connection( 
            get_signal<UseSpotFinder>().connect( static_cast<UseSpotFinder&>(*s._signals) ) ) );
    s._signals->listeners.push_back( 
        new boost::signals2::scoped_connection( 
            get_signal<UseSpotFitter>().connect( static_cast<UseSpotFitter&>(*s._signals) ) ) );
}

}
}
}
