#include <dejagnu.h>
#include "ChoiceEntry_Impl.hh"
#include "Object.hh"

using namespace simparm;

class Choice {
    simparm::Object o;
    static int count;
  public:
    Choice(std::string ident) : o("Foo" + ident, "Bar") { ++count; }
    Choice(const Choice& o) : o(o.o) { ++count; }
    ~Choice() { --count; }
    Choice* clone() const { return new Choice(*this); }

    static bool all_destroyed() { return !count; }
};

int Choice::count = 0;

int main() {
}
