#include "simparm/wx_ui/gui_thread.h"
#include "GUIThread.h"

namespace simparm {
namespace wx_ui {

void run_in_GUI_thread( boost::function0<void> f, int priority ) {
    dStorm::GUIThread::get_singleton().run_wx_function( dStorm::GUIThread::Task(f,priority) );
}

}
}
