#include "detach_window.h"
#include "wxDisplay/wxManager.h"
#include <wx/window.h>

namespace simparm {
namespace wx_ui {

class WaitableFlag {
    boost::mutex mutex;
    boost::condition condition;
    bool flagged;
public:
    WaitableFlag() : flagged(false) {}
    void wait() {
        boost::unique_lock< boost::mutex > lock(mutex);
        while ( ! flagged )
            condition.wait(lock);
    }
    void flag() { 
        boost::lock_guard< boost::mutex > lock(mutex);
        flagged = true;
    }
};

static void detach(
    WaitableFlag& did_it,
    boost::shared_ptr<wxWindow*> target
) {
    (*target)->Disable();
    did_it.flag();
}

void wait_for_window_detachment( boost::shared_ptr<wxWindow*> my_window ) 
{
    WaitableFlag flag;
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        boost::bind( &detach, boost::ref(flag), my_window ) );
    flag.wait();
}

}
}
