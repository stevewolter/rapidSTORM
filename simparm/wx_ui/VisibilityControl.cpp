#include "config_file.h"
#include "VisibilityControl.h"

namespace simparm {
namespace wx_ui {

VisibilityControl::VisibilityControl() 
{
    long value;
    dStorm::get_wxConfig()->Read( wxT("UserLevel"), &value, simparm::Beginner );
    l = static_cast<simparm::UserLevel>(value);
}

VisibilityControl::~VisibilityControl() {
    dStorm::get_wxConfig()->Write( wxT("UserLevel"), l );
}

void VisibilityControl::set_user_level( UserLevel current )
{ 
    UserLevel prev = l;
    l = current;
    s( prev, current );
}

}
}
