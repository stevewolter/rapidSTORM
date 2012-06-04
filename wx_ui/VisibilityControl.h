#ifndef SIMPARM_WX_UI_VISIBILITY_CONTROL_H
#define SIMPARM_WX_UI_VISIBILITY_CONTROL_H

#include <boost/signals2/signal.hpp>
#include <simparm/UserLevel.h>

namespace simparm {
namespace wx_ui {

class VisibilityControl {
    UserLevel l;
    boost::signals2::signal< void (UserLevel,UserLevel) > s;
public:
    VisibilityControl() : l(Beginner) {}
    void set_user_level( UserLevel current ) { 
        UserLevel prev = l;
        l = current;
        s( prev, current );
    }
    UserLevel current_user_level() const { return l; }
    boost::signals2::connection connect( boost::signals2::slot< void (UserLevel,UserLevel) > slot )
        { return s.connect( slot ); }
};

}
}

#endif
