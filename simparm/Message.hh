#ifndef SIMPARM_ERROR_HH
#define SIMPARM_ERROR_HH

#include "Node.hh"
#include "Attribute.hh"
#include <iostream>

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
    Message(const Message&);
    ~Message();
    Message* clone() const { return new Message(*this); }

    Response wait_for_answer();
    void set_response(Response s);

    std::string getTypeDescriptor() const;

    Attribute<std::string> help_file, helpID;

    std::string get_message() { return message(); }

  private:
    Attribute<std::string> title, message;
    Attribute<Severity> severity;
    Attribute<Options> options;
    Attribute<Response> response;
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
