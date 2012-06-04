#include "ProgressNode.h"
#include <wx/wx.h>
#include "wxDisplay/wxManager.h"
#include <boost/lexical_cast.hpp>

namespace simparm {
namespace wx_ui {

static void make_progress_bar( 
    boost::shared_ptr<Window> text_object, 
    boost::shared_ptr<wxGauge*> my_gauge, 
    boost::shared_ptr< Window > parent_window
) {
    *my_gauge = new wxGauge( *parent_window, wxID_ANY, 100.0 );
    *text_object = *my_gauge;
}

static void display_progress( boost::shared_ptr<wxGauge*> gauge, double value ) {
    (*gauge)->SetValue( round(value*100.0) );
}

void ProgressNode::display_value() {
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        boost::bind( &display_progress, my_gauge, boost::lexical_cast<double>( *value->get_value() ) ) );
}

void ProgressNode::initialization_finished() {
    my_line = boost::in_place( get_relayout_function() );
    create_static_text( my_line->label, description );
    run_in_GUI_thread(
        boost::bind( &make_progress_bar, my_line->contents, my_gauge, get_parent_window() ) );
    add_entry_line( *my_line );
}

}
}
