#ifndef SIMPARM_CALLBACK_H
#define SIMPARM_CALLBACK_H
#include "Callback.hh"

namespace simparm {

class TreeListener 
: public Listener {
  protected:
    TreeListener() {}
    TreeListener( const TreeListener& ) : simparm::Listener() {}
    void receive_changes_from_subtree( simparm::Node& );
    void add_new_children(const Event&);
};

}
#endif
