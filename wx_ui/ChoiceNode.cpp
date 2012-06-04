#include "ChoiceNode.h"
#include "WindowNode.h"
#include <wx/wx.h>
#include "lambda.h"
#include "VisibilityControl.h"

namespace simparm {
namespace wx_ui {

class ChoiceWidget : public wxChoice {
    void *shown_children;
    std::multimap< void*, boost::shared_ptr<Window> > children;
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
        void *new_shown_children = ( index == wxNOT_FOUND ) ? NULL : GetClientData(index);
        if ( shown_children == new_shown_children ) return;

        for ( std::multimap< void*, boost::shared_ptr<Window> >::const_iterator i = children.begin(); i != children.end(); ++i ) {
            if ( i->first == new_shown_children )
                i->second->change_frontend_visibility( true );
            else if ( i->first == shown_children )
                i->second->change_frontend_visibility( false );
        }
        shown_children = new_shown_children;
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
    
    void remove_choice( void* ident, const std::string& name, std::string description ) {
        int index = FindString( wxString( description.c_str(), wxConvUTF8 ) );
        assert( index != wxNOT_FOUND );
        Delete( index );
        //name_positions.insert( std::make_pair( name, index ) );
        names.erase( ident );
        ShowChildren();
    }
    
    void connect( void *ident, boost::shared_ptr<Window> window ) {
        children.insert( std::make_pair(ident, window) );
        if ( ident != shown_children )
            window->change_frontend_visibility( false );
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
    LineSpecification choice_line( get_relayout_function() );
    create_static_text( choice_line.label, description );
    run_in_GUI_thread(
        *bl::constant(choice_line.contents) =
        *bl::constant(choice) =
        bl::bind( bl::new_ptr< ChoiceWidget >(), 
                  *bl::constant( InnerNode::get_parent_window() ),
                  value_handle, 
                  InnerNode::get_relayout_function(),
                  get_visibility_control() ) );
    InnerNode::add_entry_line( choice_line );
}

static void add_choice(
    boost::shared_ptr<ChoiceWidget*> choice,
    void* subchoice_ident,
    std::string name,
    std::string description
) {
    (*choice)->add_choice( subchoice_ident, name, description );
}

static void remove_choice(
    boost::shared_ptr<ChoiceWidget*> choice,
    void* subchoice_ident,
    std::string name,
    std::string description
) {
    (*choice)->remove_choice( subchoice_ident, name, description );
}


static void connect_to_choice(
    boost::shared_ptr<ChoiceWidget*> choice,
    void* ident,
    boost::shared_ptr<Window> s )
{
    (*choice)->connect( ident, s );
}

static void connect_to_choice(
    boost::shared_ptr<ChoiceWidget*> choice,
    void* ident,
    const WindowSpecification& s )
{
    (*choice)->connect( ident, s.window );
}

class ChoiceNode::SubNode : public InnerNode {
    boost::shared_ptr<ChoiceNode> parent;
    std::string name, description;
    bool showed_choice;

public:
    SubNode( boost::shared_ptr<ChoiceNode> parent, std::string name )
        : InnerNode(parent), parent(parent), name(name), showed_choice(false) {}

    virtual void set_description( std::string d ) { description = d; }
    virtual void set_visibility( bool b ) { InnerNode::set_visibility(b); show_or_hide_choice();  }
    virtual void set_user_level( UserLevel u ) { InnerNode::set_user_level(u); show_or_hide_choice(); }
    void initialization_finished() {
        notify_on_visibility_change( boost::bind( &SubNode::visibility_changed, this, _1 ) );
        InnerNode::initialization_finished();
        show_or_hide_choice();
    }

    void visibility_changed( bool ) { show_or_hide_choice(); }
    void show_or_hide_choice() {
        if ( is_visible() && ! showed_choice ) 
            run_in_GUI_thread(
                boost::bind( &add_choice, parent->choice, this, name, description ) );
        if ( ! is_visible() && showed_choice )
            run_in_GUI_thread(
                boost::bind( &remove_choice, parent->choice, this, name, description ) );
        showed_choice = is_visible();
    }
    void add_entry_line( LineSpecification& s ) { 
        run_in_GUI_thread(
            bl::bind( &ChoiceWidget::connect, *bl::constant( parent->choice ), this, s.label ) );
        run_in_GUI_thread(
            bl::bind( &ChoiceWidget::connect, *bl::constant( parent->choice ), this, s.contents ) );
        run_in_GUI_thread(
            bl::bind( &ChoiceWidget::connect, *bl::constant( parent->choice ), this, s.adornment ) );
        InnerNode::add_entry_line(s);
    }
    void add_full_width_line( WindowSpecification& w ) {
        run_in_GUI_thread(
            bl::bind( &ChoiceWidget::connect, *bl::constant( parent->choice ), this, w.window ) );
        InnerNode::add_full_width_line(w);
    }
    void bind_visibility_group( boost::shared_ptr<Window> window ) {
        run_in_GUI_thread(
            bl::bind( &ChoiceWidget::connect, *bl::constant( parent->choice ), this, window ) );
        InnerNode::bind_visibility_group(window);
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

