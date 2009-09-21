#include "Entry.hh"
#include "Set.hh"
#include "NumericEntry.hh"
#include "ChoiceEntry.hh"
#include "ChoiceEntry_Impl.hh"
#include "IO.hh"

#include <stdlib.h>

using namespace std;
using namespace simparm;

int main(int argc, char *[]) {
    string 
      commands = 
        "attach\n"
        "set ChoiceName = BN\n"
        "set ChoiceName = Set\n"
        "set ChoiceName in Set set StringName = \n"
        "set ChoiceName in Set set StringName = foo\n",
      expected_output =
        "desc I/O processor\n"
        "viewable true\n"
        "userLevel 10\n"
        "showTabbed false\n"
        "declare ChoiceEntry\n"
        "name ChoiceName\n"
        "desc ChoiceDesc\n"
        "viewable true\n"
        "userLevel 10\n"
        "help \n"
        "invalid false\n"
        "editable true\n"
        "outputOnChange true\n"
        "helpID 0\n"
        "value Set\n"
        "declare Set\n"
        "name Set\n"
        "desc SetDesc\n"
        "viewable true\n"
        "userLevel 10\n"
        "showTabbed false\n"
        "declare StringEntry\n"
        "name StringName\n"
        "desc StringDesc\n"
        "viewable true\n"
        "userLevel 10\n"
        "help \n"
        "invalid false\n"
        "editable true\n"
        "outputOnChange true\n"
        "helpID 0\n"
        "value StringVal\n"
        "end\n"
        "declare DoubleEntry\n"
        "name DoubleName\n"
        "desc DoubleDesc\n"
        "viewable true\n"
        "userLevel 10\n"
        "help \n"
        "invalid false\n"
        "editable true\n"
        "outputOnChange true\n"
        "helpID 0\n"
        "value 5.88\n"
        "increment 0\n"
        "has_min false\n"
        "min 0\n"
        "has_max false\n"
        "max 0\n"
        "end\n"
        "end\n"
        "declare Set\n"
        "name BN\n"
        "desc BD\n"
        "viewable true\n"
        "userLevel 10\n"
        "showTabbed false\n"
        "end\n"
        "end\n"
        "attach\n"
        "set ChoiceName value BN\n"
        "set ChoiceName value Set\n"
        "set ChoiceName forSet Set set StringName value \n"
        "set ChoiceName forSet Set set StringName value foo\n"
        "remove desc\n"
        "remove viewable\n"
        "remove userLevel\n"
        "remove showTabbed\n"
        "remove ChoiceName\n"
        ;
    stringstream str(commands), save;

    Set set("Set", "SetDesc");
    StringEntry stringent("StringName", "StringDesc", "StringVal");
    DoubleEntry doubleEnt("DoubleName", "DoubleDesc", 5.88);
    NodeChoiceEntry<Object> choiceEnt("ChoiceName", "ChoiceDesc");
    
    doubleEnt.max = 0;
    doubleEnt.min = 0;

    set.push_back( stringent );
    set.push_back( doubleEnt );
    choiceEnt.addChoice(set);
    choiceEnt.addChoice(new Set("BN", "BD"));

    NodeChoiceEntry<Object> deepCopy( choiceEnt ),
                            shallowCopy( choiceEnt, 
                               NodeChoiceEntry<Object>::NoCopy );

    if (argc == 1)
    {
        {
            IO io(&str, &save);
            io.push_back(choiceEnt);
            io.processInput();
        }

        if ( save.str() != expected_output) {
            string a = save.str(), b = expected_output;
            int line = 0;
            for (unsigned int i = 0; i < a.size() && i < b.size(); i++)
                if ( a[i] != b[i] ) {
                    cerr << "Difference in line " << line << "\n";
                    cerr << b.substr(max(int(i)-20, 0), 40) << " != "
                         << a.substr(max(int(i)-20, 0), 40) << "\n";
                    break;
                }
                else if (a[i] == '\n')
                    line++;
            return EXIT_FAILURE;
        } else
            return EXIT_SUCCESS;
    }
    else {
        IO io(&cin, &cout);
        io.push_back(choiceEnt);
        io.processInput();
        return EXIT_SUCCESS;
    }

}
