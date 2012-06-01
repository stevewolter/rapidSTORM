#ifndef SIMPARM_WX_UI_VISIBILITY_CONTROL_H
#define SIMPARM_WX_UI_VISIBILITY_CONTROL_H

#include <map>

class wxWindow;

namespace simparm {
namespace wx_ui {

class VisibilityControl 
{
    struct Visibility {
        int user_level;
        bool backend_visible, frontend_visible;
        Visibility() : user_level(0), backend_visible( true), frontend_visible(true) {}
        Visibility( int user_level, bool backend_visible ) 
            : user_level(user_level), backend_visible(backend_visible), frontend_visible(true) {}
        bool should_be_visible( int global_user_level ) const {
            return user_level <= global_user_level && backend_visible && frontend_visible; 
        }
    };
    std::map< wxWindow*, Visibility > visibilities;
    int global_user_level;
public:
    VisibilityControl() : global_user_level(10) {}
    void register_window( boost::shared_ptr<wxWindow*> w, int user_level, bool backend_visible ) { 
        if ( *w ) {
            Visibility v( user_level, backend_visible );
            //(*w)->Show( v.should_be_visible(global_user_level) );
            visibilities[*w] = v;
        }
    }
    void unregister_window( wxWindow* w ) {
        visibilities.erase( w );
    }

    void set_user_level( int global_user_level ) {
        this->global_user_level = global_user_level;
#if 0
        for ( std::map< wxWindow*, Visibility >::const_iterator i = visibilities.begin(); i != visibilities.end(); ++i )
            i->first->Show( i->second.should_be_visible( global_user_level ) );
#endif
    }

    void set_frontend_visibility( wxWindow* w, bool visibility ) {
#if 0
        visibilities[ w ].frontend_visible = visibility;
        w->Show( visibilities[ w ].should_be_visible(global_user_level) );
#endif
    }
};

}
}

#endif
