#include "ProgressNode.h"
#include <wx/wx.h>
#include <boost/lexical_cast.hpp>
#include "lambda.h"
#include "gui_thread.h"

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

void ProgressNode::display_value() {
    if ( determinate ) {
        int percentage = round(boost::lexical_cast<double>( *value->get_value() ) * 100.0);
        run_in_GUI_thread(
            bl::bind( &wxGauge::SetValue, *bl::constant(my_gauge), percentage ) );
    } else {
        run_in_GUI_thread( bl::bind( &wxGauge::Pulse, *bl::constant(my_gauge) ) );
    }
}

void ProgressNode::set_determinate_mode() {
    determinate = indeterminate->get_value().get_value_or("false") == "false";
    display_value();
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
