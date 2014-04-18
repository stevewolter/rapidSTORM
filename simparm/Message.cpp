#include "simparm/Message.h"
#include "simparm/Node.h"
#include <stdexcept>
#include <sstream>

namespace simparm {

std::istream& operator>>( std::istream& i, Message::Severity& t)
{
    std::string r; i >> r;
    if ( r == "Debug" ) t = Message::Debug;
    else if ( r == "Question" ) t = Message::Question;
    else if ( r == "Info" ) t = Message::Info;
    else if ( r == "Warning" ) t = Message::Warning;
    else if ( r == "Error" ) t = Message::Error;
    else if ( r == "Critical" ) t = Message::Critical;
    else
        throw std::logic_error("Unknown message severity " + r + " read");
    return i;
}

std::ostream& operator<<( std::ostream& o, Message::Severity t ) {
    switch (t) {
        case Message::Debug: return (o << "Debug"); 
        case Message::Question: return (o << "Question"); 
        case Message::Info: return (o << "Info"); 
        case Message::Warning: return (o << "Warning"); 
        case Message::Error: return (o << "Error"); 
        case Message::Critical: return (o << "Critical"); 
        default:
        throw std::logic_error("No string constant for message severity");
    }
}

std::istream& operator>>( std::istream& i, Message::Options& t)
{
    std::string r; i >> r;
    if ( r == "JustOK" ) t = Message::JustOK;
    else if ( r == "OKCancel" ) t = Message::OKCancel;
    else if ( r == "YesNo" ) t = Message::YesNo;
    else if ( r == "YesNoCancel" ) t = Message::YesNoCancel;
    else
        throw std::logic_error("Unknown message option " + r + " read");
    return i;
}

std::ostream& operator<<( std::ostream& o, Message::Options t ) {
    switch (t) {
        case Message::JustOK: return (o << "JustOK"); 
        case Message::OKCancel: return (o << "OKCancel"); 
        case Message::YesNo: return (o << "YesNo"); 
        case Message::YesNoCancel: return (o << "YesNoCancel"); 
        default:
        throw std::logic_error("No string constant for message severity");
    }
}

std::istream& operator>>( std::istream& i, Message::Response& t)
{
    std::string r; i >> r;
    if ( r == "OKYes" ) t = Message::OKYes;
    else if ( r == "No" ) t = Message::No;
    else if ( r == "Cancel" ) t = Message::Cancel;
    else if ( r == "None" ) t = Message::None;
    else
        throw std::logic_error("Unknown message severity " + r + " read");
    return i;
}

std::ostream& operator<<( std::ostream& o, Message::Response t ) {
    switch (t) {
        case Message::OKYes: return (o << "OKYes"); 
        case Message::No: return (o << "No"); 
        case Message::Cancel: return (o << "Cancel"); 
        case Message::None: return (o << "None"); 
        default:
        throw std::logic_error("No string constant for message severity");
    }
}

Message::Message(
    std::string t,
    std::string m,
    Severity s,
    Options o )
: helpID(""),
  title(t),
  message(m),
  severity(s),
  options(o)
{
}

Message::~Message() {
}

std::ostream& operator<<( std::ostream& o, const Message& m)
{
    o << m.title << ": " << m.message << std::endl;
    return o;
}

Message::Response Message::send( NodeHandle n ) {
    return n->send( *this );
}

}
