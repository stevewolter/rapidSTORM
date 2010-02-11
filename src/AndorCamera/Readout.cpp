#include "Readout.h"
#include "SDK.h"
#include <string.h>
#include "Config.h"
#include "System.h"

using namespace simparm;

namespace AndorCamera {

using namespace States;
using namespace Phases;

void Readout::controlStateChanged(
    Phase phase, State, State to)

{
    if ( phase == Transition && to == Acquiring )
        SDK::SetFrameTransferMode( frame_transfer_mode() );
}

ImageReadout::ImageReadout(StateMachine &sm)
: Readout("ImageReadout", "Image Readout mode"),
  Node::Callback( simparm::Event::ValueChanged ),
  left("LeftCaptureBorder", "Leftmost column to capture", 0),
  top("TopCaptureBorder","Topmost row to capture", 0),
  right("RightCaptureBorder","Rightmost column to capture", 1023),
  bottom("BottomCaptureBorder","Bottommost row to capture", 1023),
  sm(sm)
{
    left.setMin( 0 );
    top.setMin( 0 );
    left.setMax( right() );
    top.setMax( bottom() );
    right.setMin( left() );
    bottom.setMin( top() );

    registerNamedEntries();
}
  
ImageReadout::ImageReadout(const ImageReadout&c)
: Readout(c),
  Node::Callback( simparm::Event::ValueChanged ),
  left(c.left),
  top(c.top),
  right(c.right),
  bottom(c.bottom),
  sm(c.sm)
{
    registerNamedEntries();
}

ImageReadout::~ImageReadout()
{
    sm.remove_managed_attribute( left.viewable );
    sm.remove_managed_attribute( top.viewable );
    sm.remove_managed_attribute( right.viewable );
    sm.remove_managed_attribute( bottom.viewable );
}

static void manage_att(StateMachine &sm, simparm::Entry &e) {
    sm.add_managed_attribute( e.viewable,
        Initialized, Acquiring );
}

void ImageReadout::registerNamedEntries() 
{
    manage_att( sm, left );
    manage_att( sm, right );
    manage_att( sm, top );
    manage_att( sm, bottom );

    Readout::registerNamedEntries(*this);
    receive_changes_from( left.value );
    receive_changes_from( right.value );
    receive_changes_from( top.value );
    receive_changes_from( bottom.value );

    push_back( left );
    push_back( right );
    push_back( top );
    push_back( bottom );
}

void ImageReadout::operator()(const simparm::Event& e)
{
    if ( &e.source == &left.value ) {
        right.setMin( left() );
    } else if ( &e.source == &right.value ) {
        left.setMax( right() );
    } else if ( &e.source == &top.value ) {
        bottom.setMin( top() );
    } else if ( &e.source == &bottom.value ) {
        top.setMax( bottom() );
    }
}

void ImageReadout::controlStateChanged(
    Phase phase, State from, State to
)
{
    Readout::controlStateChanged(phase, from, to);

    if ( phase == Transition && to == Acquiring ) 
    {
        left.editable = false;
        right.editable = false;
        top.editable = false;
        bottom.editable = false;
        SDK::SetReadMode( AndorCamera::Image );
        SDK::SetImageNoBinning( left(), right(), top(), bottom() );
    } else if ( phase == Transition && from == Acquiring ) {
        left.editable = true;
        right.editable = true;
        top.editable = true;
        bottom.editable = true;
    } else if ( phase == Review && from == Disconnected && to == Initialized ) {
        std::pair<int,int> s = SDK::GetDetector();
        right.setMax( s.first-1 );
        bottom.setMax( s.second-1 );
    }
}

}
