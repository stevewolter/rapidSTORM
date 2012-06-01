#include "ChoiceNode.h"
#include "WindowNode.h"
#include <wx/wx.h>
#include "lambda.h"
#include "VisibilityControl.h"

namespace simparm {
namespace wx_ui {

class ChoiceWidget : public wxChoice {
    void *shown_children;
    std::multimap< void*, wxWindow* > children;
    std::map< std::string, int > name_positions;
    std::map< void*, std::string > names;
    std::string current_selection;
    boost::shared_ptr< BaseAttributeHandle > value;
    boost::function0<void> relayout;
    boost::shared_ptr< VisibilityControl > vc;

    void GUI_changed_choice( wxCommandEvent& ) { 
        ShowChildren(); 
        int index = GetSelection();
        if ( GetSelection() != wxNOT_FOUND )
            value->set_value( names[ GetClientData(index) ] );
        else
            value->set_value( "" );
    }
    void ShowChildren() {
        int index = GetSelection();
        shown_children = ( index == wxNOT_FOUND ) ? NULL : GetClientData(index);
        for ( std::multimap< void*, wxWindow* >::const_iterator i = children.begin(); i != children.end(); ++i ) {
            vc->set_frontend_visibility( i->second, i->first == shown_children );
        }
        relayout();
    }

public:
    ChoiceWidget( 
        wxWindow *parent, 
        boost::shared_ptr< BaseAttributeHandle > value, 
        boost::function0<void> relayout ,
        boost::shared_ptr< VisibilityControl > vc
    ) 
        : wxChoice( parent, wxID_ANY ),
          shown_children(NULL),
          current_selection(*value->get_value()),
          value(value),
          relayout( relayout ),
          vc( vc )
        {}

    void add_choice( void* ident, const std::string& name, std::string description ) {
        int index = Append( wxString( description.c_str(), wxConvUTF8 ), ident );
        name_positions.insert( std::make_pair( name, index ) );
        names.insert( std::make_pair( ident, name ) );
        ShowChildren();
        if ( current_selection == name )
            SetSelection( index );
    }
    
    void connect( void *ident, wxWindow* window ) {
        children.insert( std::make_pair(ident, window) );
        if ( ident != shown_children )
            vc->set_frontend_visibility( window, false );
        relayout();
    }

    void select_choice( std::string name ) {
        SetSelection( name_positions[name] );
        ShowChildren();
    }

    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(ChoiceWidget, wxChoice)
    EVT_CHOICE(wxID_ANY, ChoiceWidget::GUI_changed_choice)
END_EVENT_TABLE()

void ChoiceNode::initialization_finished() {
    LineSpecification choice_line;
    create_static_text( choice_line.label, description );
    run_in_GUI_thread(
        *bl::constant(choice_line.contents) =
        *bl::constant(choice) =
        bl::bind( bl::new_ptr< ChoiceWidget >(), 
                  *bl::constant( Node::get_parent_window() ),
                  value_handle, 
                  Node::get_relayout_function(),
                  get_visibility_control() ) );
    Node::add_entry_line( choice_line );
}

static void add_choice(
    boost::shared_ptr<ChoiceWidget*> choice,
    void* subchoice_ident,
    std::string name,
    std::string description
) {
    (*choice)->add_choice( subchoice_ident, name, description );
}

static void connect_to_choice_line(
    boost::shared_ptr<ChoiceWidget*> choice,
    void* ident,
    const LineSpecification& s )
{
    if ( *s.label )
        (*choice)->connect( ident, *s.label );
    (*choice)->connect( ident, *s.contents );
    if ( *s.adornment )
        (*choice)->connect( ident, *s.adornment );
}

static void connect_to_choice_full(
    boost::shared_ptr<ChoiceWidget*> choice,
    void* ident,
    const WindowSpecification& s )
{
    (*choice)->connect( ident, *s.window );
}

class ChoiceNode::SubNode : public Node {
    boost::shared_ptr<ChoiceNode> parent;
    std::string name, description;
public:
    SubNode( boost::shared_ptr<ChoiceNode> parent, std::string name )
        : Node(parent), parent(parent), name(name) {}

    virtual void set_description( std::string d ) { description = d; }
    void initialization_finished() {
        Node::initialization_finished();
        run_in_GUI_thread(
            boost::bind( &add_choice, parent->choice, this, name, description ) );
    }
    void add_entry_line( const LineSpecification& s ) { 
        run_in_GUI_thread(
            boost::bind( &connect_to_choice_line, parent->choice, this, s ) );
        Node::add_entry_line(s);
    }
    void add_full_width_line( WindowSpecification w ) {
        run_in_GUI_thread(
            boost::bind( &connect_to_choice_full, parent->choice, this, w ) );
        Node::add_full_width_line(w);
    }

};

void ChoiceNode::add_attribute( simparm::BaseAttribute& a ) {
    if ( a.get_name() == "value" ) {
        value_handle.reset( new BaseAttributeHandle(a) );
        connection = a.notify_on_non_GUI_value_change( 
            boost::bind( &ChoiceNode::user_changed_choice, this ) );
    }
}

void ChoiceNode::user_changed_choice() {
    run_in_GUI_thread( 
        bl::bind( &ChoiceWidget::select_choice, *bl::constant(choice), *value_handle->get_value() ) );
}

NodeHandle ChoiceNode::create_object( std::string name ) {
    return NodeHandle( new SubNode( boost::static_pointer_cast<ChoiceNode>( shared_from_this() ), name ) );
}

NodeHandle ChoiceNode::create_group( std::string name ) {
    return NodeHandle( new SubNode( boost::static_pointer_cast<ChoiceNode>( shared_from_this() ), name ) );
}

}
}

