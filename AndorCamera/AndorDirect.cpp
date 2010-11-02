#define CImgBuffer_ANDORDIRECT_CPP

#include "debug.h"

#include "Config.h"
#include "AndorDirect.h"
#include <string.h>
#include <sstream>
#include <iomanip>
#include <dStorm/output/Basename.h>
#include <dStorm/input/Source_impl.h>
#include "ViewportSelector.h"
#include <boost/utility.hpp>

#include "LiveView.h"

#define CHECK(x) checkAndorCode( x, __LINE__ )

using namespace std;
using namespace dStorm::input;
using namespace dStorm;
using namespace simparm;

namespace AndorCamera {

Source::Source
    (boost::shared_ptr<LiveView> live_view, CameraReference& c) 

: Set("AndorDirect", "Direct acquisition"),
  CamSource( static_cast<simparm::Node&>(*this),
    BaseSource::Flags().set(BaseSource::TimeCritical) ),
  control(c),
  is_initialized( initMutex),
  initialized(false),
  error_in_initialization(false),
  acquisition(control),
  status(acquisition.status),
  live_view(live_view)
{
    status.editable = false;

    push_back( c->config() );
    push_back( status );
    push_back( *live_view );
    DEBUG("Built AndorDirect source");
}

Source::~Source() {
    DEBUG( "Destructing source" );
}

#define MUST_CONVERT
#define AcquisitionType WORD
#define GetImages GetImages16

void Source::waitForInitialization() const {
    DEBUG("Trying to get initialization wait mutex");
    ost::MutexLock lock(initMutex);
    while ( !initialized ) {
        DEBUG("Waiting for acquisition initialization");
        is_initialized.wait();
        DEBUG("Waited for acquisition initialization");
    }
    if ( error_in_initialization )
        throw std::runtime_error(
            "An error in image acquisition prevents running a job");
}

Source::TraitsPtr Source::get_traits() 
{
    DEBUG("Waiting for camera initialization to get traits");
    waitForInitialization();
    DEBUG("Got camera initialization");
    TraitsPtr rv( new TraitsPtr::element_type() );
    rv->size.x() = acquisition.getWidth();
    rv->size.y() = acquisition.getHeight();
    DEBUG("Acquisition has a length set: " << acquisition.hasLength());
    if ( acquisition.hasLength() ) {
        DEBUG("Acquisition length is " << acquisition.getLength() << " frames");
        rv->last_frame = acquisition.getLength() - 1 * cs_units::camera::frame;
    }
    return rv;
}

}
