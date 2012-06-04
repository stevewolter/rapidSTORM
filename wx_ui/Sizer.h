#ifndef SIMPARM_WX_UI_SIZER_H
#define SIMPARM_WX_UI_SIZER_H

#include "GUIHandle.h"
#include "Node.h"

class wxGridBagSizer;
class wxSizer;

namespace simparm {
namespace wx_ui {

class Sizer {
protected:
    GUIHandle<wxGridBagSizer> sizer;
    boost::shared_ptr<int> row;

public:
    Sizer();
    GUIHandle<wxSizer> create_sizer();
    void add_entry_line( LineSpecification& );
    void add_full_width_line( WindowSpecification& w );
    void add_full_width_sizer( SizerSpecification& w );
};

}
}

#endif

