#include "Message.hh"
#include "Object.hh"
#include "TriggerEntry.hh"
#include "IO.hh"
#include <dejagnu.h>

using namespace simparm;

struct MyListener : public Node::Callback {
    TriggerEntry &n;
    Message e2, e3, e4;
    MyListener( TriggerEntry& n ) : n(n),
        e2("Display Error title", "Displayed error"),
        e3("OKCancel", "OK or cancel?", Message::Error, Message::OKCancel),
        e4("YesNo", "Yes or no?", Message::Question, Message::YesNo)
        { e4.helpID = "#55"; receive_changes_from(n.value); }
    void operator()( const Event& ) {
        if ( n.triggered() ) {
            n.untrigger();
            n.send(e2);
            n.send(e3);
            n.send(e4);
        }
    }
};

int main() {
    TestState().untested("Error handling");
    return 0;
    Object o("objectName", "objectDesc");
    Message e1("stdio Error title", "stdio error");
    IO io(&std::cin, &std::cout);
    TriggerEntry t("Trigger", "Trigger");

    simparm::Attribute<std::string> hf("help_file", "foo.chm");
    t.push_back(hf);

    o.push_back(e1);
    MyListener l(t);
    io.push_back(t);
    //io.forkInput();
    std::cerr << "Got yes/no answer " << l.e4.wait_for_answer() << std::endl;
    std::cerr << "Got ok/cancel answer " << l.e3.wait_for_answer() << std::endl;

    return 0;

}
