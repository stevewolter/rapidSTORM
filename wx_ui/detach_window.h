#ifndef SIMPARM_WX_WAIT_FOR_WINDOW_DETACHMENT_H
#define SIMPARM_WX_WAIT_FOR_WINDOW_DETACHMENT_H

#include <boost/smart_ptr/shared_ptr.hpp>

class wxWindow;

namespace simparm {
namespace wx_ui {

void wait_for_window_detachment( boost::shared_ptr<wxWindow*> );

}
}

#endif
