#ifndef DSTORM_ANDORCAMERA_INPUTCHAINLINK_H
#define DSTORM_ANDORCAMERA_INPUTCHAINLINK_H

#include <dStorm/input/chain/Link.h>
#include <simparm/Object.hh>
#include <simparm/Entry.hh>
#include <simparm/TriggerEntry.hh>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/MetaInfo.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <memory>

namespace dStorm {
namespace AndorCamera {

struct CameraConnection;
struct Display;

/** The Method class provides the configuration items for Andor camera
    *  acquisition that are acquisition-specific - control elements and
    *  acquisition area borders. All camera specific parameters are in
    *  AndorCamera::Config. */
class Method 
: public dStorm::input::chain::Terminus, public simparm::Object, public simparm::Node::Callback
{
  private:
    ContextRef last_context;
    TraitsRef published;

    boost::mutex active_selector_mutex;
    boost::condition_variable active_selector_changed;
    std::auto_ptr<Display> active_selector;
    simparm::TriggerEntry select_ROI, view_ROI;

    AtEnd publish_meta_info();
    simparm::BoolEntry show_live_by_default;

    void registerNamedEntries();
    AtEnd context_changed( ContextRef, Link * );

    void operator()( const simparm::Event& );

  public:
    Method();
    Method(const Method &c);
    virtual ~Method();

    simparm::Node& getNode() { return *this; }
    dStorm::input::BaseSource* makeSource() ;

    Method* clone() const { return new Method(*this); }
    bool uses_input_file() const { return false; }

    void set_display( std::auto_ptr< Display > );
};


}
}

#endif
