#include "BasenameWatcher.h"
#include "Config.h"
#include "Method.h"
#include "debug.h"

namespace dStorm {
namespace input {
BasenameWatcher::BasenameWatcher(
    MethodChoice& choice,
    simparm::Attribute<std::string>& output
) : simparm::Node::Callback( simparm::Event::ValueChanged ),
    choice( choice ),
    output( output )
{
    attach();
    receive_changes_from(choice.value);
}

void BasenameWatcher::attach() {
    if ( choice.isValid() ) {
        DEBUG("Switching to choice " << choice().getName() << " with value " << choice().output_file_basename());
        current = &choice().output_file_basename;
        output = (*current)();
        receive_changes_from( *current );
    } else {
        current = NULL;
    }
}

void BasenameWatcher::operator()( 
    const simparm::Event& e
) {
    if ( &e.source == &choice.value ) {
        DEBUG("Base changed value to " << ((choice.isValid()) ? choice().getName() : "invalid"));
        if ( current != NULL )
            stop_receiving_changes_from(*current);
        attach();
    } else {
        DEBUG("Propagating basename " << (*current)());
        output = (*current)();
    }
}

}
}
