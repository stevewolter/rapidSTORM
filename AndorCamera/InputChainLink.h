#ifndef DSTORM_ANDORCAMERA_INPUTCHAINLINK_H
#define DSTORM_ANDORCAMERA_INPUTCHAINLINK_H

#include <dStorm/input/chain/Link.h>
#include <simparm/Object.hh>
#include <simparm/Entry.hh>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/MetaInfo.h>

namespace dStorm {
namespace AndorCamera {

struct CameraConnection;

/** The Method class provides the configuration items for Andor camera
    *  acquisition that are acquisition-specific - control elements and
    *  acquisition area borders. All camera specific parameters are in
    *  AndorCamera::Config. */
class Method 
: public dStorm::input::chain::Terminus, public simparm::Object
{
  private:
    ContextRef last_context;
    TraitsRef published;

    AtEnd publish_meta_info();
  public:
    simparm::BoolEntry show_live_by_default;

  private:
    void registerNamedEntries();
    AtEnd context_changed( ContextRef, Link * );

  public:
    Method();
    Method(const Method &c);
    virtual ~Method();

    simparm::Node& getNode() { return *this; }
    dStorm::input::BaseSource* makeSource() ;

    Method* clone() const { return new Method(*this); }
    bool uses_input_file() const { return false; }
};


}
}

#endif
