#define CImgBuffer_ANDORDIRECT_CPP
#include "AndorDirect.h"
#include <AndorCamera/Control.h>
#include "Image.h"
#include <string.h>
#include "AndorDirect.h"
#include <sstream>
#include <iomanip>
#include <CImg.h>
#include <foreach.h>
#include "ViewportSelector.h"
#include "Buffer.h"

#define CHECK(x) checkAndorCode( x, __LINE__ )

using namespace std;
using namespace CImgBuffer;
using namespace AndorCamera;

namespace CImgBuffer {
namespace AndorDirect {

typedef CImgBuffer::Source< CamImage > MySource;

auto_ptr< MySource > AndorDirect::Config::makeSource() 
    throw(std::exception) 
{
    control->ensureInitialized();
    return auto_ptr<MySource>(new Source(
        left(), top(), right()-left()+1, bottom()-top()+1,
        numImages()));
}

AndorDirect::Source::Source
    (int l, int t, int w, int h, int numImages) 
    throw(std::exception)
: MySource (true, false),
  ost::Thread("Andor camera acquirer"),
    width(w), height(h), numImages(numImages),
    acquisition(l, l+w-1, t, t+h-1, numImages)
{
}

AndorDirect::Source::~Source() throw(std::exception) {
    STATUS( "Destructing source" );
}

void Source::run() throw() { acquire(); }

#define MUST_CONVERT
#define AcquisitionType WORD
#define GetImages GetImages16

void Source::acquire() throw()
{
    long lastValidImage = -1;
    try {
        acquisition.start();

        /* cancelAcquisition is set in a different thread by stopPushing(). */
        while ( ! cancelAcquisition && acquisition.hasMoreImages() ) {
            auto_ptr<CamImage> image
                (new CamImage(width, height, lastValidImage+1));
            long imNum = acquisition.getNextImage( image->ptr() );
            if (imNum >= 0) lastValidImage = imNum; else break;
            pushTarget->accept( imNum, image );
        }
    } catch (const std::exception& e) {
        cerr << PACKAGE_NAME << ": Image acquisition error: " 
             << e.what() << endl;
    }
    if ( ! cancelAcquisition ) {
        lastValidImage++;
        if (  lastValidImage < numImages ) {
            cerr << "Warning: Camera acquisition failed. Padding " << (numImages-lastValidImage) << " images" << endl;
        }
        while ( lastValidImage < numImages ) {
            pushTarget->accept( lastValidImage, auto_ptr<CamImage>(NULL) );
            lastValidImage++;
        }
    }
}

int Source::dimx() throw() { return width; }
int Source::dimy() throw() { return height; }

int Source::image_quantity() throw() { return numImages; }

void Source::startPushing(Drain<CamImage> *target) 
    throw(std::exception) 
{
    if (this->pushTarget != NULL)
        throw runtime_error
            ("This AndorDirect source already has a target.");

    pushTarget = target;
    cancelAcquisition = false;

    Thread::start();
}

void Source::stopPushing(Drain<CamImage> *target) throw(std::exception) {
    if (pushTarget != target)
        throw std::logic_error(
                    "This AndorDirect source does not push to the "
                    "specified target.");
    cancelAcquisition = true;
    Thread::join();
    pushTarget = NULL;
}

std::ostream& Source::writeAcquisitionInfo(std::ostream &o)
    throw()
{
    return o << AndorCamera::Config(*control);
}

Config::Config(CImgBuffer::Config<CamImage>& src) throw(std::exception) 
: papa(src),
  pause("PauseCamera", "Pause"),
  imageFile("SaveAcquiredImageFile", "Save camera snapshot to"),
  save("SaveAcquiredImage", "Save camera snapshot")
{
    PROGRESS("Making AndorDirect config");
    divider.setName("DividerStopAimCamera");

    aim.setName("AimCamera");
    aim.setDesc("Take aim");
    aim.setUserLevel(ConfigEntry::Beginner);
    stopAim.setName("StopAimCamera");
    stopAim.setDesc("Leave aiming mode");

    stopAim.setUserLevel(ConfigEntry::Beginner);
    divider.setUserLevel(ConfigEntry::Beginner);
    pause.setUserLevel(ConfigEntry::Beginner);
    imageFile.setUserLevel(ConfigEntry::Beginner);
    save.setUserLevel(ConfigEntry::Beginner);

    stopAim.setViewable(false);
    divider.setViewable(false);
    pause.setViewable(false);
    imageFile.setViewable(false);
    save.setViewable(false);

    numImages.setName("ImageNumber");
    numImages.setDesc("Number of images to take");
    numImages.setUserLevel(ConfigEntry::Beginner);
    numImages = 4000;

    left.setName("LeftCaptureBorder");
    left.setDesc("Leftmost column to capture");
    left = 0;
    top.setName("TopCaptureBorder");
    top.setDesc("Topmost row to capture");
    top = 0;
    right.setName("RightCaptureBorder");
    right.setDesc("Rightmost column to capture");
    right = 1023;
    bottom.setName("BottomCaptureBorder");
    bottom.setDesc("Bottommost row to capture");
    bottom = 1023;

    LOCKING("Receiving attach event");
    gotEvent(papa.attached);
    init();

    PROGRESS("Made AndorDirect config");
}

CImgBuffer::AndorDirect::Config::Config(const Config &c, 
    CImgBuffer::Config<CamImage>& src) throw(std::exception) 
: InputConfig< CamImage >(c),
  ConfigEventHandler(c),
  AndorCamera::ControlStateListener(),
  papa(src),
  numImages(c.numImages),
  left(c.left),
  top(c.top),
  right(c.right),
  bottom(c.bottom),
  aim(c.aim),
  stopAim(c.stopAim)
{
    init();
}

CImgBuffer::AndorDirect::Config::~Config() throw() {
    if (initialized) resetDimensions();
}

void CImgBuffer::AndorDirect::Config::init() throw() {
    resetDimensions();
    connect(aim);
    connect(stopAim);
    connect(left);
    connect(right);
    connect(top);
    connect(bottom);
    connect(papa.attached);
    registerEntries(*this);
}

void CImgBuffer::AndorDirect::Config::registerEntries(ConfigSet &c)
throw() 
{
    c.register_entry(&left);
    c.register_entry(&top);
    c.register_entry(&right);
    c.register_entry(&bottom);
    c.register_entry(&numImages);
    c.register_entry(&aim);
    c.register_entry(&stopAim);
    c.register_entry(&divider);
    c.register_entry(&pause);
    c.register_entry(&imageFile);
    c.register_entry(&save);
}

void CImgBuffer::AndorDirect::Config::gotEvent(const ConfigEntry &src) throw() {
    if ( &src == &aim && aim.triggered() ) {
        startAiming();
    } else if ( &src == &stopAim && stopAim.triggered() ) {
        stopAiming();
    } else if ( &src == &left ) {
        if (right.min() != left() )
            right.setMin( left() );
    } else if ( &src == &right ) {
        if (left.max() != right() )
            left.setMax( right() );
    } else if ( &src == &top ) {
        if (bottom.min() != top() )
            bottom.setMin( top() );
    } else if ( &src == &bottom ) {
        if (top.max() != bottom() )
            top.setMax( bottom() );
    } else if ( &src == &papa.attached && papa.attached ) {
        control->copyIO(papa);
    }
}

void CImgBuffer::AndorDirect::Config::startAiming() throw() {
    aim.setViewable( false );
    stopAim.setViewable( true );
    divider.setViewable( true );
    pause.setViewable( true );
    imageFile.setViewable( true );
    save.setViewable( true );
    control->ensureInitialized();
    new ViewportSelector(*this);
}

void CImgBuffer::AndorDirect::Config::stopAiming() throw() {
    aim.setViewable( true );
    stopAim.setViewable( false );
    divider.setViewable( false );
    pause.setViewable( false );
    imageFile.setViewable( false );
    save.setViewable( false );
}

void CImgBuffer::AndorDirect::Config::controlStateChanged(AndorCamera::State newState) 
    throw(std::exception)
{
    if (newState >= AndorCamera::Initialized && !initialized) {
        initializeDimensions();
    } else if (newState < AndorCamera::Initialized && initialized) {
        resetDimensions();
    }
}

void CImgBuffer::AndorDirect::Config::initializeDimensions() throw() {
    AndorCamera::Control::Dimension detectorSize = 
        control->getDetectorSize();

    right.setMax(detectorSize.w-1);
    bottom.setMax(detectorSize.h-1);
    initialized = true;
}

void CImgBuffer::AndorDirect::Config::resetDimensions() throw() {
    right.setMax(1023);
    bottom.setMax(1023);
    left.setMax(right());
    top.setMax(bottom());
    initialized = false;
}

void Config::printAdditionalConfigSets(ostream &o) throw() 
    { o << "[" << control->getName() << "]" << endl;
        o << *control; }

}

}
