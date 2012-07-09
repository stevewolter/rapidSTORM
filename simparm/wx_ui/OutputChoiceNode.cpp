#include <wx/button.h>
#include <wx/choicdlg.h>

#include "OutputChoiceNode.h"
#include "Node.h"
#include "AttributeHandle.h"
#include "lambda.h"
#include "gui_thread.h"

#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/foreach.hpp>

namespace simparm {
namespace wx_ui {

class OutputChoiceButton;

class OutputChoiceNode : public InnerNode {
    class SubNode;
    boost::optional<LineSpecification> my_line;
    GUIHandle< OutputChoiceButton > choice;
    std::string description;

    boost::shared_ptr< BaseAttributeHandle > value_handle;

public:
    OutputChoiceNode( boost::shared_ptr<Node> n, std::string name )
        : InnerNode(n, name) {}
    virtual void set_description( std::string d ) { description = d; }
    void initialization_finished();
    NodeHandle create_object( std::string name );
    NodeHandle create_group( std::string name );
    void add_attribute( simparm::BaseAttribute& a );
};

class OutputChoiceButton : public wxButton {
    std::vector< std::string > available;
    std::map< std::string, std::string > idents;
    boost::shared_ptr< BaseAttributeHandle > value;

    void clicked( wxCommandEvent& ) {
        std::sort( available.begin(), available.end() );
        wxArrayString choices;
        BOOST_FOREACH( const std::string& s, available )
            choices.Add( wxString( s.c_str(), wxConvUTF8 ) );
        wxMultiChoiceDialog dialog(this, _("Select output modules that should be added"),
                                         _("Output module creation"), choices );
        int answer = dialog.ShowModal();
        if ( answer == wxID_OK ) {
            wxArrayInt selections = dialog.GetSelections();
            for (size_t selection = 0; selection < selections.Count(); ++selection) {
                std::string description( choices.Item( selections.Item( selection ) ).mb_str() );
                value->set_value( idents[description] );
            }
        }
    }

public:
    OutputChoiceButton( 
        wxWindow *parent, 
        boost::shared_ptr< BaseAttributeHandle > value
    ) 
        : wxButton( parent, wxID_ANY, _("Add output module ...") ), value(value) {}

    void add_choice( void*, const std::string& name, std::string description ) {
        available.push_back( description );
        idents[description] = name;
    }
    
    void remove_choice( void*, const std::string&, std::string description ) {
        boost::range::remove_erase( available, description );
    }
    
    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(OutputChoiceButton, wxButton)
    EVT_BUTTON(wxID_ANY, OutputChoiceButton::clicked)
END_EVENT_TABLE()

void OutputChoiceNode::initialization_finished() {
    my_line = boost::in_place( get_relayout_function() );
    create_static_text( my_line->label, description );
    run_in_GUI_thread(
        *bl::constant(my_line->contents) =
        *bl::constant(choice) =
        bl::bind( bl::new_ptr< OutputChoiceButton >(), 
                  *bl::constant( InnerNode::get_parent_window() ),
                  value_handle ) );
    attach_help( my_line->label );
    attach_help( my_line->contents );
    InnerNode::add_entry_line( *my_line );
}

class OutputChoiceNode::SubNode : public InnerNode {
    boost::shared_ptr<OutputChoiceNode> parent;
    std::string name, description;
    VisibilityNode visibility;
    bool showed_choice;

public:
    SubNode( boost::shared_ptr<OutputChoiceNode> parent, std::string name )
        : InnerNode(parent, name), parent(parent), name(name), visibility( *parent->get_visibility_control() ), showed_choice(false) {}
    ~SubNode() {
        if ( showed_choice )
            run_in_GUI_thread(
                bl::bind( &OutputChoiceButton::remove_choice, *bl::constant(parent->choice), this, name, description ) );
    }

    virtual void set_description( std::string d ) { description = d; }
    virtual void set_visibility( bool b ) { visibility.set_visibility(b); show_or_hide_choice();  }
    virtual void set_user_level( UserLevel u ) { visibility.set_user_level(u); show_or_hide_choice(); }
    void initialization_finished() {
        visibility.add_listener( boost::bind( &SubNode::visibility_changed, this, _1 ) );
        show_or_hide_choice();
    }

    void visibility_changed( bool ) { show_or_hide_choice(); }
    void show_or_hide_choice() {
        if ( visibility.is_visible() && ! showed_choice ) {
            run_in_GUI_thread(
                bl::bind( &OutputChoiceButton::add_choice, *bl::constant(parent->choice), this, name, description ) );
        }
        if ( ! visibility.is_visible() && showed_choice ) {
            run_in_GUI_thread(
                bl::bind( &OutputChoiceButton::remove_choice, *bl::constant(parent->choice), this, name, description ) );
        }
        showed_choice = visibility.is_visible();
    }
    void add_entry_line( LineSpecification& ) {}
    void add_full_width_line( WindowSpecification& ) {}
    void bind_visibility_group( boost::shared_ptr<Window> ) {}
};

void OutputChoiceNode::add_attribute( simparm::BaseAttribute& a ) {
    if ( a.get_name() == "value" ) {
        value_handle.reset( new BaseAttributeHandle(a, get_protocol()) );
    }
}

NodeHandle OutputChoiceNode::create_object( std::string name ) {
    return NodeHandle( new SubNode( boost::static_pointer_cast<OutputChoiceNode>( shared_from_this() ), name ) );
}

NodeHandle OutputChoiceNode::create_group( std::string name ) {
    return NodeHandle( new SubNode( boost::static_pointer_cast<OutputChoiceNode>( shared_from_this() ), name ) );
}

NodeHandle create_output_choice_node( boost::shared_ptr<Node> n, std::string name ) {
    return NodeHandle( new OutputChoiceNode( n, name ) );
}

}
}

