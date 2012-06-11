#ifndef SIMPARM_WX_UI_GUIHANDLE_H
#define SIMPARM_WX_UI_GUIHANDLE_H

#include <boost/smart_ptr/shared_ptr.hpp>

namespace simparm {
namespace wx_ui {

template <typename GUIType>
struct GUIHandle : public boost::shared_ptr<GUIType*> {
    GUIHandle() : boost::shared_ptr<GUIType*>( new GUIType*() ) {}
};

}
}

#endif


