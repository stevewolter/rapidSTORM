#ifndef SIMPARM_ERROR_HH
#define SIMPARM_ERROR_HH

#include <iostream>
#include "NodeHandle.h"

namespace simparm {

class Message {
  public:
    enum Severity { Question,
                    Debug, Info, Warning, Error, Critical};
    enum Options { JustOK, OKCancel, YesNo, YesNoCancel };
    enum Response { OKYes, No, Cancel, None };

    Message(std::string title, std::string message,
          Severity severity = Error,
          Options possible_answers = JustOK );
    ~Message();
    Message* clone() const { return new Message(*this); }

    std::string helpID;
    std::string title, message;
    Severity severity;
    Options options;

    Response send( NodeHandle );

    friend std::ostream& operator<<( std::ostream&, const Message&);
};

std::ostream& operator<<( std::ostream&, Message::Severity);
std::ostream& operator<<( std::ostream&, Message::Options);
std::ostream& operator<<( std::ostream&, Message::Response);

std::istream& operator>>( std::istream&, Message::Severity&);
std::istream& operator>>( std::istream&, Message::Options&);
std::istream& operator>>( std::istream&, Message::Response&);

}

#endif
