#define CImgBuffer_ANDORDIRECT_CPP

#include "debug.h"

#include "AndorDirect.h"
#include <string.h>
#include <sstream>
#include <iomanip>
#include <dStorm/output/Basename.h>
#include <dStorm/input/Source_impl.h>
#include <boost/utility.hpp>

#include "LiveView.h"
#include "CameraConnection.h"

#define CHECK(x) checkAndorCode( x, __LINE__ )

using namespace std;
using namespace dStorm::input;
using namespace dStorm;
using namespace simparm;

namespace dStorm {
namespace AndorCamera {

Source::Source
    (std::auto_ptr<CameraConnection> connection, bool show_live, LiveView::Resolution res )
: Set("AndorDirect", "Direct acquisition"),
  CamSource( static_cast<simparm::Node&>(*this), BaseSource::Flags() ),
  connection(connection),
  has_ended(false), show_live(show_live), resolution(res),
  status("CameraStatus", "Camera status")
{
    status.editable = false;

    push_back( status );
    DEBUG("Built AndorDirect source " << this);
}

Source::~Source() {
    DEBUG( "Destructing source " << this );
}

Source::TraitsPtr Source::get_traits() 
{
    CamTraits cam_traits;
    connection->start_acquisition( cam_traits, status );
    TraitsPtr rv( new TraitsPtr::element_type(cam_traits) );
    live_view.reset( new LiveView(show_live, resolution ) );
    traits = rv;
    assert( rv.get() ); /* Make sure noone changed type to auto_ptr */
    return rv;
}

}
}
