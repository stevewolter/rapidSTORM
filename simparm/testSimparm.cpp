#include "Entry.hh"
#include "Set.hh"
#include "Entry.hh"
#include "ChoiceEntry.hh"
#include "ChoiceEntry_Impl.hh"
#include "IO.hh"

#include <stdlib.h>

using namespace std;
using namespace simparm;

int main(int, char *[]) {
    Set set("Set", "SetDesc");
    StringEntry stringent("StringName", "StringDesc", "StringVal");
    Entry<double> doubleEnt("DoubleName", "DoubleDesc", 5.88);
    ChoiceEntry<Object> choiceEnt("ChoiceName", "ChoiceDesc");
    
    doubleEnt.max = 4;
    doubleEnt.min = 0;

    IO io(&cin, &cout);
    simparm::NodeRef r = set.attach_ui(io);
    stringent.attach_ui( r );
    doubleEnt.attach_ui( r );
    choiceEnt.addChoice(set);

    choiceEnt.attach_ui( io );
    io.processInput();
    return EXIT_SUCCESS;
}
