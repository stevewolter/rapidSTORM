#include "TextfieldNode.h"
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/multi_array.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

namespace simparm {
namespace wx_ui {

namespace bl = boost::lambda;

class TextCtrl : public wxPanel {
    static const int ID_BASE = 500;
    typedef std::vector< wxTextCtrl* > Texts;
    Texts texts;
    wxCheckBox* optional;
    boost::shared_ptr< BaseAttributeHandle > value;
    wxColour normal_bg, uncommitted_bg;
    const int columns;

public:
    TextCtrl( wxWindow* parent, boost::shared_ptr< BaseAttributeHandle > value, int rows, int columns ) 
    : wxPanel( parent, wxID_ANY ),
      optional( new wxCheckBox( this, wxID_ANY, wxT("") ) ),
      value(value),
      uncommitted_bg( 255, 200, 200 ),
      columns( columns )
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

        wxBoxSizer* outer_sizer = new wxBoxSizer( wxHORIZONTAL );
        outer_sizer->Add( optional );
        outer_sizer->Add( grid_sizer, 1, wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN );
        SetSizer( outer_sizer );
    }

    void enter_pressed(wxCommandEvent&) { 
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
        texts[index]->SetBackgroundColour( uncommitted_bg );
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
    create_static_text( my_line.adornment, unit );
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
