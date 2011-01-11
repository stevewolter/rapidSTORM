#include "debug.h"
#include "Readout.h"
#include "SDK.h"
#include <string.h>
#include "Config.h"
#include "System.h"

#include "StateMachine_impl.h"
#include <simparm/EntryManipulators.hh>

using namespace simparm;

namespace AndorCamera {

using namespace States;
using boost::units::camera::pixel;

Readout::Readout(const std::string& name, const std::string& desc)
        : simparm::Object(name, desc),
          frame_transfer_mode("FrameTransferMode", 
                              "Enable frame transfer mode", true)
        { frame_transfer_mode.setUserLevel(simparm::Object::Expert); }
Readout::Readout(const Readout& c)
        : StateMachine::Listener(), simparm::Object(c), 
          frame_transfer_mode(c.frame_transfer_mode) {}

MK_EMPTY_RW(Readout)

template <>
class Readout::Token<Readying> 
: public States::Token
{
    simparm::EditabilityChanger c;

  public:
    Token(Readout& r) : c(r.frame_transfer_mode, !r.is_active) {
        if ( r.is_active ) {
            SDK::SetFrameTransferMode( r.frame_transfer_mode() );
        }
    }
};

ImageReadout::ImageReadout(StateMachine &sm)
: Readout("ImageReadout", "Image Readout mode"),
  Node::Callback( simparm::Event::ValueChanged ),
  StateMachine::StandardListener<ImageReadout>(*this),
  left("LeftCaptureBorder", "Leftmost column to capture", 0 * pixel),
  top("TopCaptureBorder","Topmost row to capture", 0 * pixel),
  right("RightCaptureBorder","Rightmost column to capture", 1023 * pixel),
  bottom("BottomCaptureBorder","Bottommost row to capture", 1023 * pixel),
  sm(sm)
{
    DEBUG(right.max());
    left.setMin( 0 * pixel );
    DEBUG(right.max());
    top.setMin( 0 * pixel );
    DEBUG(right.max());
    left.setMax( right() );
    DEBUG(right.max());
    top.setMax( bottom() );
    DEBUG(right.max());
    right.setMin( left() );
    DEBUG(right.max());
    bottom.setMin( top() );
    DEBUG(right.max());

    registerNamedEntries();
}
  
ImageReadout::ImageReadout(const ImageReadout&c)
: Readout(c),
  Node::Callback( simparm::Event::ValueChanged ),
  StateMachine::StandardListener<ImageReadout>(*this),
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
}

void ImageReadout::registerNamedEntries() 
{
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
        DEBUG(right.hasMin << " " << right.hasMax << " " << right.min() << " " << right.max());
        right.setMin( left() );
        DEBUG(right.hasMin << " " << right.hasMax << " " << right.min() << " " << right.max());
    } else if ( &e.source == &right.value ) {
        DEBUG(right.hasMin << " " << right.hasMax << " " << right.min() << " " << right.max());
        left.setMax( right() );
        DEBUG(right.hasMin << " " << right.hasMax << " " << right.min() << " " << right.max());
    } else if ( &e.source == &top.value ) {
        bottom.setMin( top() );
    } else if ( &e.source == &bottom.value ) {
        top.setMax( bottom() );
    }
}

template <int State>
struct ImageReadout::Token
: public Readout::Token<State>
{
    Token(ImageReadout& i);
    ~Token();
};

template <>
class ImageReadout::Token<Readying>
: public Readout::Token<Readying>
{
    std::auto_ptr<simparm::EditabilityChanger> l,r,t,b;
  public:
    Token(ImageReadout& i) 
        : Readout::Token<Readying>(i)
        {
            if ( i.is_active ) {
                l.reset( new simparm::EditabilityChanger(i.left, false));
                r.reset( new simparm::EditabilityChanger(i.right, false));
                t.reset( new simparm::EditabilityChanger(i.top, false));
                b.reset( new simparm::EditabilityChanger(i.bottom, false));
                SDK::SetReadMode( AndorCamera::Image );
                SDK::SetImageNoBinning( i.left() / pixel, i.right() / pixel, 
                                        i.top() / pixel, i.bottom() / pixel );
            }
        }
    ~Token() {}
};

template <>
ImageReadout::Token<Connected>::Token(ImageReadout& i)
: Readout::Token<Connected>(i)
{
    std::pair<int,int> s = SDK::GetDetector();
    DEBUG("Setting limits to " << s.first-1 << " " << s.second -1 );
    i.right.setMax( (s.first-1) * pixel );
    i.bottom.setMax( (s.second-1) * pixel );
}

template <int State>
ImageReadout::Token<State>::Token(ImageReadout& i) 
: Readout::Token<State>(i) {}
template <int State>
ImageReadout::Token<State>::~Token() {}

template class StateMachine::StandardListener<ImageReadout>;


}
