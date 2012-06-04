#ifndef WX_UI_VISIBILITY_NODE_H
#define WX_UI_VISIBILITY_NODE_H

#include "Window.h"
#include "VisibilityControl.h"

namespace simparm {
namespace wx_ui {

class VisibilityNode {
    bool backend_visible;
    UserLevel my_user_level, desired_user_level;
    boost::signals2::signal< void(bool) > visibility_changed;
    boost::signals2::scoped_connection connection_to_visibility_control;

    void change_visibility( bool becomes_visible );
    void desired_user_level_changed( UserLevel previous, UserLevel current );
public:
    VisibilityNode( VisibilityControl& vc ) ;
    void add_group( boost::shared_ptr< Window > );
    void add_listener( boost::signals2::slot< void(bool) > s ) { visibility_changed.connect(s); }
    void set_visibility( bool v );
    void set_user_level( UserLevel );
    bool is_visible() const;
};

}
}

#endif
