#include "Message.hh"
#include <pthread.h>

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
        throw std::runtime_error("Unknown message severity " + r + " read");
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
        throw std::runtime_error("Unknown message option " + r + " read");
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
        throw std::runtime_error("Unknown message severity " + r + " read");
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

static std::string mkname(void *v) {
    std::stringstream ss;
    ss << size_t(v);
    return ss.str();
}

Message::Message(
    std::string t,
    std::string m,
    Severity s,
    Options o )
: Node("simparmMessage" + mkname(this)),
  help_file("help_file", ""),
  helpID("helpID", ""),
  title("title", t),
  message("message", m),
  severity("severity", s),
  options("options", o),
  response("response", None)
{
    push_back( title );
    push_back( message );
    push_back( severity );
    push_back( options );
    push_back( help_file );
    push_back( helpID );
    if ( o != JustOK )
        push_back( response );
}

Message::Message( const Message& e ) 
: Node(e),
  help_file(e.help_file),
  helpID(e.helpID),
  title(e.title),
  message(e.message),
  severity(e.severity),
  options(e.options),
  response(e.response)
{
    push_back( title );
    push_back( message );
    push_back( severity );
    push_back( options );
    push_back( help_file );
    push_back( helpID );
    if ( options() != JustOK )
        push_back( response );
}

Message::~Message() {
}

struct ErrorListener : public Node::Callback {
    pthread_mutex_t mutex;
    pthread_cond_t condition;

    Attribute<Message::Response>& response;

    ErrorListener(Attribute<Message::Response>& r) : response(r) {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&condition, NULL);

        receive_changes_from( response );
    }

    ~ErrorListener() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&condition);
    }

    void operator()( const Event& ) {
        pthread_mutex_lock(&mutex);
        pthread_cond_broadcast( &condition );
        pthread_mutex_unlock(&mutex);
    }

    void wait() { 
        pthread_mutex_lock(&mutex);
        while ( response() == Message::None ) 
            pthread_cond_wait( &condition, &mutex );
        pthread_mutex_unlock(&mutex);
    }
};

Message::Response Message::wait_for_answer() 
{
    if ( response() == None ) {
        ErrorListener(response).wait();
    }
    return response();
}

std::string 
Message::getTypeDescriptor() const
{
    return "Message";
}

std::ostream& operator<<( std::ostream& o, const Message& m)
{
    o << m.title() << ": " << m.message() << std::endl;
    return o;
}

void Message::set_response(Response s) { response = s; }

}
