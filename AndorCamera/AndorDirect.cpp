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
    (std::auto_ptr<CameraConnection> connection, bool )
: Set("AndorDirect", "Direct acquisition"),
  CamSource( static_cast<simparm::Node&>(*this) ,
    BaseSource::Flags() /*.set(BaseSource::TimeCritical) */ ),
  connection(connection),
  has_ended(false),
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
    TraitsPtr rv( new TraitsPtr::element_type() );
    connection->start_acquisition( *rv );
#if 0
    rv->size.x() = acquisition.getWidth();
    rv->size.y() = acquisition.getHeight();
    DEBUG("Acquisition has a length set: " << acquisition.hasLength());
    if ( acquisition.hasLength() ) {
        DEBUG("Acquisition length is " << acquisition.getLength() << " frames");
        rv->image_number().range().second = acquisition.getLength() - 1 * camera::frame;
    }
#endif
    traits = rv;
    assert( rv.get() ); /* Make sure noone changed type to auto_ptr */
    return rv;
}

}
}
