#ifndef WX_UI_RUN_IN_GUI_THREAD_H
#define WX_UI_RUN_IN_GUI_THREAD_H

#include <boost/function/function0.hpp>

namespace simparm {
namespace wx_ui {

void run_in_GUI_thread( boost::function0<void>, int priority = 0 );

}
}

#endif
