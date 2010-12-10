#include "ConfigEntry.hh"
#include "ConfigSet.hh"
#include "ConfigIO.hh"

#include <unistd.h>

using namespace std;
using namespace simparm;

int main(int argc, char *[]) throw() {
    bool waiter = false;
    istream *str;
    if (argc == 1) {
        string commands = "attach\n";
        for (int i = 0; i < 10; i++)
            commands += "sleep\n";
        str = new stringstream(commands);
        waiter = true;
    } else
        str = &cin;

    ConfigSet set("Set", "SetDesc");
    ConfigEntryString stringent("StringName", "StringDesc", "StringVal");
    ConfigEntryDouble doubleEnt("DoubleName", "DoubleDesc", 5.88);
    ConfigEntryTypedChoice<ConfigObject>
        choiceEnt("ChoiceName", "ChoiceDesc");

    set.push_back( stringent );
    set.push_back( doubleEnt );
    choiceEnt.addChoice(new ConfigSet("BN", "BD"));

    {
        ConfigIO io(str, &cout);
        io.push_back(choiceEnt);

        io.forkInput();

        if (waiter) sleep(2);

        if (waiter) sleep(1);
        choiceEnt.addChoice(set);
        if (waiter) sleep(1);
        choiceEnt.choose(set.name());
        if (waiter) sleep(1);
        choiceEnt.choose("BN");
        if (waiter) sleep(1);
        choiceEnt.choose(set.name());
        if (waiter) sleep(1);
        stringent = "foo";
    }

    return 0;
}
