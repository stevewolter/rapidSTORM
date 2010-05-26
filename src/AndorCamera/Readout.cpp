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
        right.setMin( left() );
    } else if ( &e.source == &right.value ) {
        left.setMax( right() );
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
                SDK::SetImageNoBinning( i.left(), i.right(), 
                                        i.top(), i.bottom() );
            }
        }
    ~Token() {}
};

template <>
ImageReadout::Token<Connected>::Token(ImageReadout& i)
: Readout::Token<Connected>(i)
{
    std::pair<int,int> s = SDK::GetDetector();
    i.right.setMax( s.first-1 );
    i.bottom.setMax( s.second-1 );
}

template <int State>
ImageReadout::Token<State>::Token(ImageReadout& i) 
: Readout::Token<State>(i) {}
template <int State>
ImageReadout::Token<State>::~Token() {}

template class StateMachine::StandardListener<ImageReadout>;


}
