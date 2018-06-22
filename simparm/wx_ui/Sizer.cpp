/* wxWidgets must be included first in Windows due to conflicts with Boost's
 * handling of windows.h. */
#include <wx/gbsizer.h>

#include "simparm/wx_ui/Sizer.h"
#include "simparm/wx_ui/lambda.h"
#include "simparm/wx_ui/gui_thread.h"

namespace simparm {
namespace wx_ui {

static const int Border = 2;

Sizer::Sizer() : row( new int(0) ) {}

GUIHandle<wxSizer> Sizer::create_sizer() {
    GUIHandle<wxSizer> rv;
    run_in_GUI_thread( (
        *bl::constant( rv ) =
        *bl::constant( sizer ) =
            bl::bind( bl::new_ptr<wxGridBagSizer>() ),
        bl::bind( &wxGridBagSizer::SetEmptyCellSize, *bl::constant(sizer), wxSize(0,0) ),
        bl::bind( &wxGridBagSizer::AddGrowableCol, *bl::constant(sizer), 1, 0 )
    ) );
    return rv;
}

static void add_entry_line( 
    boost::shared_ptr<wxGridBagSizer*> sizer, 
    boost::shared_ptr<Window> label,
    boost::shared_ptr<Window> contents,
    boost::shared_ptr<Window> adornment,
    boost::shared_ptr<int> row
) {
    if ( *label )
        (*sizer)->Add( *label, wxGBPosition(*row,0), wxGBSpan(), wxALL | wxALIGN_CENTER_VERTICAL, Border );
    if ( *adornment ) {
        (*sizer)->Add( *contents, wxGBPosition(*row,1), wxGBSpan(), wxGROW | wxALL, Border );
        (*sizer)->Add( *adornment, wxGBPosition(*row,2), wxGBSpan(), wxALIGN_CENTER | wxALL, Border );
    } else {
        (*sizer)->Add( *contents, wxGBPosition(*row,1), wxGBSpan(1,2), wxGROW | wxALL, Border );
    }
    (*sizer)->Layout();
    ++ *row;
}

static void add_full_width_line(
    boost::shared_ptr<wxGridBagSizer*> sizer, 
    boost::shared_ptr<Window> window,
    boost::shared_ptr<int> row,
    int proportion
) {
    (*sizer)->Add( *window, wxGBPosition(*row,0), wxGBSpan(1,3), wxGROW | wxEXPAND );
    if ( proportion ) (*sizer)->AddGrowableRow( *row );
    (*sizer)->Layout();
    ++ *row;
}

static void add_full_width_sizer(
    boost::shared_ptr<wxGridBagSizer*> sizer, 
    boost::shared_ptr<wxSizer*> child, 
    boost::shared_ptr<int> row,
    int proportion
) {
    (*sizer)->Add( *child, wxGBPosition(*row,0), wxGBSpan(1,3), wxGROW | wxEXPAND );
    if ( proportion ) (*sizer)->AddGrowableRow( *row );
    (*sizer)->Layout();
    ++ *row;
}

void Sizer::add_entry_line( LineSpecification& line ) {
    run_in_GUI_thread(
        boost::bind( &wx_ui::add_entry_line, sizer, line.label, line.contents, line.adornment, row ) );
}

void Sizer::add_full_width_line( WindowSpecification& w ) {
    run_in_GUI_thread(
        boost::bind( &wx_ui::add_full_width_line, sizer, w.window, row, w.proportion ) );
}

void Sizer::add_full_width_sizer( SizerSpecification& w ) {
    run_in_GUI_thread(
        boost::bind( &wx_ui::add_full_width_sizer, sizer, w.sizer, row, w.proportion ) );
}

boost::function0<void> Sizer::relayout_function() {
    return bl::bind( &wxSizer::Layout, *bl::constant(sizer) );
}

}
}

