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

    set.push_back( stringent );
    set.push_back( doubleEnt );
    choiceEnt.addChoice(set);

    IO io(&cin, &cout);
    io.push_back(choiceEnt);
    io.processInput();
    return EXIT_SUCCESS;
}
