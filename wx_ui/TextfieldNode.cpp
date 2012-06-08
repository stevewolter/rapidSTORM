#include "TextfieldNode.h"
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/bmpbuttn.h>
#include <wx/filedlg.h>
#include <wx/bitmap.h>
#include <wx/dcmemory.h>

#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/multi_array.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "wx_ui/stock-hchain-24-broken.xpm"
#include "wx_ui/stock-hchain-24.xpm"
#include "gui_thread.h"

namespace simparm {
namespace wx_ui {

namespace bl = boost::lambda;

class TextCtrl : public wxPanel {
    static const int ID_BASE = 500, ID_CHAIN = 499, ID_UNCHAIN = 498;
    typedef std::vector< wxTextCtrl* > Texts;
    Texts texts;
    wxCheckBox* optional;
    wxButton *chain, *unchain;
    boost::shared_ptr< BaseAttributeHandle > value;
    wxColour normal_bg, uncommitted_bg;
    const int rows, columns;

    bool chained;

    wxBitmap set_background( wxBitmap btnBmp ) {
        wxColour bkgrClr = GetBackgroundColour();

        wxBitmap prepBtnBmp(btnBmp.GetWidth(), btnBmp.GetHeight());
        wxMemoryDC memDC(prepBtnBmp);
        memDC.SetBackground(*wxTheBrushList->FindOrCreateBrush(bkgrClr));
        memDC.Clear();
        memDC.DrawBitmap(btnBmp, 0, 0, true);
        memDC.SelectObject(wxNullBitmap);
        return prepBtnBmp;
    }

    void do_chain( wxCommandEvent& ) {
        chained = true;
        chain->Hide();
        unchain->Show();
        Layout();
    }

    void do_unchain( wxCommandEvent& ) {
        chained = false;
        chain->Show();
        unchain->Hide();
        Layout();
    }

public:
    TextCtrl( wxWindow* parent, boost::shared_ptr< BaseAttributeHandle > value, int rows, int columns ) 
    : wxPanel( parent, wxID_ANY ),
      optional( new wxCheckBox( this, wxID_ANY, wxT("") ) ),
      chain(NULL), unchain(NULL),
      value(value),
      uncommitted_bg( 255, 200, 200 ),
      rows(rows),
      columns( columns ),
      chained( false )
    {
        texts.resize( rows * columns );
        for (Texts::iterator i = texts.begin(); i != texts.end(); ++i)
            *i = new wxTextCtrl( this, ID_BASE + (i - texts.begin()), wxT(""), 
                wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

        normal_bg = texts.front()->GetBackgroundColour();
        optional->Show( value->value_is_optional() );
        ChangeValue( value->get_value() );

        wxGridSizer* grid_sizer = NULL;
        if ( rows > 1 && columns == 1 ) 
             grid_sizer = new wxGridSizer( columns, rows, 3, 3 );
        else
             grid_sizer = new wxGridSizer( rows, columns, 3, 3 );

        for (Texts::iterator i = texts.begin(); i != texts.end(); ++i)
            grid_sizer->Add( *i, 1, wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN );

        if ( rows > 1 || columns > 1 ) {
            chain = new wxBitmapButton( this, ID_CHAIN, set_background( wxBitmap(stock_hchain_24_broken) ) );
            unchain = new wxBitmapButton( this, ID_UNCHAIN, set_background( wxBitmap(stock_hchain_24) ) );
            unchain->Hide();
        }

        wxBoxSizer* outer_sizer = new wxBoxSizer( wxHORIZONTAL );
        outer_sizer->Add( optional );
        outer_sizer->Add( grid_sizer, 1, wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN );
        if ( chain ) outer_sizer->Add( chain );
        if ( unchain ) outer_sizer->Add( unchain );
        SetSizer( outer_sizer );
    }

    void enter_pressed(wxCommandEvent&) { 
        commit_text(); 
    }
    void commit_text() {
        std::stringstream o;
        int field = 0;
        for (Texts::iterator i = texts.begin(); i != texts.end(); ++i) {
            if ( i != texts.begin() ) {
                if ( field % columns == 0 )
                    o << ",";
                else
                    o << " ";
            }
            o << (*i)->GetValue().mb_str();
            ++field;
        }
        bool success = value->set_value( o.str() );
        
        for (Texts::iterator i = texts.begin(); i != texts.end(); ++i) {
            (*i)->SetBackgroundColour( (success) ? normal_bg : uncommitted_bg );
            (*i)->ClearBackground();
        }
    }

    void text_changed(wxCommandEvent& ev) {
        int index = ev.GetId() - ID_BASE;
        if ( chained ) {
            wxString common_value = texts[index]->GetValue();
            for (Texts::iterator i = texts.begin(); i != texts.end(); ++i) {
                (*i)->SetBackgroundColour( uncommitted_bg );
                if ( common_value != (*i)->GetValue() )
                    (*i)->ChangeValue( common_value );
            }
        } else {
            texts[index]->SetBackgroundColour( uncommitted_bg );
        }
    }

    void checkbox_marked( wxCommandEvent& ) {
        if ( optional->GetValue() ) {
            for (Texts::iterator i = texts.begin(); i != texts.end(); ++i) {
                (*i)->Show();
                (*i)->SetBackgroundColour( uncommitted_bg );
                (*i)->ClearBackground();
            }
        } else {
            for (Texts::iterator i = texts.begin(); i != texts.end(); ++i)
                (*i)->Hide();
            value->unset_value();
        }
    }

    void ChangeValue( boost::optional< std::string > v ) {
        if ( ! v ) {
            for (Texts::iterator i = texts.begin(); i != texts.end(); ++i)
                (*i)->Hide();
        } else if ( rows == 1 && columns == 1 ) {
            texts.front()->ChangeValue( wxString( v->c_str(), wxConvUTF8 ) );
        } else {
            typedef boost::char_separator<char> Sep;
            typedef boost::tokenizer<Sep> Tok;
            Tok tokens( *v, Sep(" ,") );
            Texts::iterator j = texts.begin();
            for ( Tok::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j ) {
                assert( j != texts.end() );
                (*j)->ChangeValue( wxString( i->c_str(), wxConvUTF8 ) );
            }
        }
    }

    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(TextCtrl, wxPanel)
EVT_TEXT        (wxID_ANY, TextCtrl::text_changed )
EVT_TEXT_ENTER  (wxID_ANY, TextCtrl::enter_pressed )
EVT_CHECKBOX    (wxID_ANY, TextCtrl::checkbox_marked )
EVT_BUTTON      (ID_CHAIN, TextCtrl::do_chain )
EVT_BUTTON      (ID_UNCHAIN, TextCtrl::do_unchain )
END_EVENT_TABLE()

class FileSelectionButton : public wxButton {
    TextCtrl *text_ctrl;
    void select_file( wxCommandEvent& ) {
        wxFileDialog dialog(this);
        int response = dialog.ShowModal();
        if ( response == wxID_OK ) {
            wxString file = dialog.GetPath();
            text_ctrl->ChangeValue( boost::optional<std::string>( file.mb_str() ) );
            text_ctrl->commit_text();
        }
    }
public:
    FileSelectionButton( wxWindow* parent, TextCtrl* text_ctrl )
        : wxButton( parent, wxID_ANY, wxT("Select") ), text_ctrl(text_ctrl) {}

    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(FileSelectionButton, wxButton)
EVT_BUTTON      (wxID_ANY, FileSelectionButton::select_file )
END_EVENT_TABLE()


void TextfieldNode::display_value() {
    run_in_GUI_thread( bl::bind( &TextCtrl::ChangeValue, *bl::constant(my_window), value_handle->get_value() ) );
}

void TextfieldNode::initialization_finished() {
    LineSpecification my_line( get_relayout_function() );
    create_static_text( my_line.label, description );
    run_in_GUI_thread( 
        *bl::constant( my_line.contents ) = 
        *bl::constant( my_window ) = 
            bl::bind( bl::new_ptr< TextCtrl >(), *bl::constant(get_parent_window()), value_handle, rows, columns ) );
    if ( mode == Value )
        create_static_text( my_line.adornment, unit );
    else
        run_in_GUI_thread(
            *bl::constant( my_line.adornment ) =
            bl::bind( bl::new_ptr< FileSelectionButton >(), *bl::constant(get_parent_window()), *bl::constant(my_window) )
        );
    attach_help( my_line.label );
    attach_help( my_line.contents );
    add_entry_line( my_line );
}

TextfieldNode::~TextfieldNode() {
    value_handle->detach();
}

void TextfieldNode::add_attribute( simparm::BaseAttribute& a ) {
    if ( a.get_name() == "value" ) {
        value_handle.reset( new BaseAttributeHandle(a) );
        connection = a.notify_on_non_GUI_value_change( boost::bind( &TextfieldNode::display_value, this ) );
    }
    if ( a.get_name() == "unit_symbol" ) unit = *a.get_value();
    if ( a.get_name() == "rows" ) rows = boost::lexical_cast<int>( *a.get_value() );
    if ( a.get_name() == "columns" ) columns = boost::lexical_cast<int>( *a.get_value() );
}

}
}
