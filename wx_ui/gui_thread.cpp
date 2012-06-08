#include "gui_thread.h"
#include <dStorm/GUIThread.h>

namespace simparm {
namespace wx_ui {

void run_in_GUI_thread( boost::function0<void> f ) {
    dStorm::GUIThread::get_singleton().run_wx_function( f );
}

}
}
