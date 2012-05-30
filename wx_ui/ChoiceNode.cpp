#include "ChoiceNode.h"
#include "WindowNode.h"
#include <wx/wx.h>
#include "wxDisplay/wxManager.h"

namespace simparm {
namespace wx_ui {

class ChoiceWidget : public wxChoice {
    void *shown_children;
    std::multimap< void*, wxWindow* > children;
    std::map< std::string, int > names;
    std::string current_selection;

    void ShowChildrenWx( wxCommandEvent& ) { ShowChildren(); }
    void ShowChildren() {
        int index = GetSelection();
        shown_children = ( index == wxNOT_FOUND ) ? NULL : GetClientData(index);
        for ( std::multimap< void*, wxWindow* >::const_iterator i = children.begin(); i != children.end(); ++i ) {
            i->second->Show( i->first == shown_children );
        }
        fit_all_parents();
    }

    void fit_all_parents() {
        GetContainingSizer()->Layout();
    }

public:
    ChoiceWidget( wxWindow *parent, std::string selection ) 
        : wxChoice( parent, wxID_ANY ),
          shown_children(NULL),
          current_selection(selection) {}

    void add_choice( void* ident, const std::string& name, std::string description ) {
        int index = Append( wxString( description.c_str(), wxConvUTF8 ), ident );
        names.insert( std::make_pair( name, index ) );
        ShowChildren();
        if ( current_selection == name )
            SetSelection( index );
    }
    
    void connect( void *ident, wxWindow* window ) {
        children.insert( std::make_pair(ident, window) );
        if ( ident != shown_children )
            window->Hide();
        fit_all_parents();
    }

    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(ChoiceWidget, wxChoice)
    EVT_CHOICE(wxID_ANY, ChoiceWidget::ShowChildrenWx)
END_EVENT_TABLE()

static void make_choice( 
    boost::shared_ptr< ChoiceWidget*> choice,
    LineSpecification line,
    std::string selection,
    std::string label,
    boost::shared_ptr< wxWindow* > parent_window
) {
    *choice = new  ChoiceWidget( *parent_window, selection );
    *line.label = new wxStaticText( *parent_window, wxID_ANY, wxString( label.c_str(), wxConvUTF8 ) );
    *line.contents = *choice;
}

void ChoiceNode::initialization_finished() {
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        boost::bind( &make_choice, choice, choice_line, *value->get_value(), description, Node::get_parent_window() ) );
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
        dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
            boost::bind( &add_choice, parent->choice, this, name, description ) );
    }
    void add_entry_line( const LineSpecification& s ) { 
        dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
            boost::bind( &connect_to_choice_line, parent->choice, this, s ) );
        Node::add_entry_line(s);
    }
    void add_full_width_line( WindowSpecification w ) {
        dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
            boost::bind( &connect_to_choice_full, parent->choice, this, w ) );
        Node::add_full_width_line(w);
    }

};

NodeHandle ChoiceNode::create_object( std::string name ) {
    return NodeHandle( new SubNode( boost::static_pointer_cast<ChoiceNode>( shared_from_this() ), name ) );
}

NodeHandle ChoiceNode::create_group( std::string name ) {
    return NodeHandle( new SubNode( boost::static_pointer_cast<ChoiceNode>( shared_from_this() ), name ) );
}

}
}

