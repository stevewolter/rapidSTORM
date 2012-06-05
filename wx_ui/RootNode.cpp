#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "RootNode.h"
#include "ScrolledTabNode.h"
#include "wxDisplay/wxManager.h"
#include "wxDisplay/App.h"
#include "VisibilityControl.h"
#include "lambda.h"
#include <wx/notebook.h>
#include <boost/smart_ptr/make_shared.hpp>
#include <wx/helpbase.h>
#include <wx/cshelp.h>
#include <wx/wxhtml.h>
#include <wx/msgdlg.h>
#include <fstream>

namespace simparm {
namespace wx_ui {

class HelpResolver {
    std::auto_ptr<wxHtmlHelpController> help_controller;
    std::map< wxWindow*, std::string > context_help_ids;
    std::map< std::string, std::string > help_alias;

    void read_alias_table() {
        std::ifstream i("doc/alias.h");
        while ( i ) {
            std::string line;
            std::getline( i, line );
            if ( ! i || line == "" ) break;
            if ( line.substr(0,5) != "HELP_" ) continue;
            size_t separator = line.find("=");
            if ( separator == std::string::npos || separator < 5 ) continue;
            std::string help_id = line.substr(5, separator-5);
            std::string location = line.substr(separator+1);
            help_alias[ help_id ] = location;
        }
    }

public:
    HelpResolver()
    : help_controller( new wxHtmlHelpController() )
    {
        help_controller->AddBook( wxFileName( wxT("doc"), wxT("manual.hhp") ) );
        read_alias_table();
    }
    void associate_window_with_help( wxWindow* window, std::string help_id )
        { context_help_ids[window] = help_id; }
    void show_help( wxWindow* window ) {
        std::string help_id = context_help_ids[window];
        if ( help_id != "" ) {
            if ( help_id[0] == '#' ) help_id = help_id.substr(1);
            std::string help_location = help_alias[ help_id ];
            help_controller->Display( wxString(help_location.c_str(), wxConvUTF8) );
        } else
            wxMessageBox(_("There is no help for this element."));
    }
    void show_manual() { help_controller->DisplayContents(); }
};

class RootFrame
: public wxFrame {
    boost::shared_ptr< Node > root_node;
    boost::shared_ptr< VisibilityControl > ul_control;
    wxBoxSizer *column;
    std::vector< boost::shared_ptr< void > > to_delete;
    HelpResolver help;

    enum EventID {
        UL_BEGINNER, UL_INTERMEDIATE, UL_EXPERT,
        CONTEXT_HELP, MANUAL
    };

    void invalidate_best_sizes( wxWindow* window ) {
        wxNotebook* nb = dynamic_cast<wxNotebook*>(window);
        if ( nb )
            nb->InvalidateBestSize();
        wxWindowList& children = window->GetChildren();
        for ( wxWindowList::iterator i = children.begin(); i != children.end(); ++i )
            invalidate_best_sizes( *i );
    }

    void relayout( wxWindow* window ) {
        window->Layout();
        wxWindowList& children = window->GetChildren();
        for ( wxWindowList::iterator i = children.begin(); i != children.end(); ++i )
            relayout( *i );
    }

    void fit_inside( wxWindow* window ) {
        wxScrolledWindow* sw = dynamic_cast<wxScrolledWindow*>(window);
        if ( sw ) sw->FitInside();
        wxWindowList& children = window->GetChildren();
        for ( wxWindowList::iterator i = children.begin(); i != children.end(); ++i )
            fit_inside( *i );
    }

    void change_user_level( UserLevel l ) {
        ul_control->set_user_level( l );
        invalidate_best_sizes( this );
        relayout( this );
        fit_inside( this );
    }
    void user_level_beginner(wxCommandEvent&) { change_user_level( Beginner ); }
    void user_level_intermediate(wxCommandEvent&) { change_user_level( Intermediate ); }
    void user_level_expert(wxCommandEvent&) { change_user_level( Expert ); }

    void context_help(wxCommandEvent&) { wxContextHelp contextHelp(this); }
    void show_manual(wxCommandEvent&) { help.show_manual(); }

    void show_help( wxHelpEvent& ev ) {
        wxWindow* w = dynamic_cast< wxWindow* >( ev.GetEventObject() );
        if ( w ) help.show_help( w );
    }

public:
    RootFrame( boost::shared_ptr<Node> root_node, boost::shared_ptr< VisibilityControl > vc )
        : wxFrame( NULL, wxID_ANY, wxT( PACKAGE_STRING ) ),
          root_node( root_node ), 
          ul_control( vc ),
          column( new wxBoxSizer(wxVERTICAL) )
    {
        wxMenuBar* menu = new wxMenuBar();
        wxMenu* user_level = new wxMenu();
        user_level->AppendRadioItem( UL_BEGINNER, _("Beginner") );
        user_level->AppendRadioItem( UL_INTERMEDIATE, _("Intermediate") );
        user_level->AppendRadioItem( UL_EXPERT, _("Expert") );
        menu->Append( user_level, _("User level") );
        wxMenu* help = new wxMenu();
        help->Append( MANUAL, _("Manual") );
        help->Append( CONTEXT_HELP, _("What's this?") );
        menu->Append( help, _("Help") );
        SetMenuBar( menu );

        SetSizer(column);
    }

    ~RootFrame() {
    }

    void add_window( wxWindow* window ) {
        column->Add( window, 1, wxEXPAND );
        Layout();
    }

    void delete_on_death( boost::shared_ptr<void> object ) {
        to_delete.push_back( object );
    }

    void attach_context_help( wxWindow* window, std::string context_help_id ) 
        { help.associate_window_with_help( window, context_help_id ); }

    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(RootFrame, wxFrame)
    EVT_MENU(UL_BEGINNER, RootFrame::user_level_beginner)
    EVT_MENU(UL_INTERMEDIATE, RootFrame::user_level_intermediate)
    EVT_MENU(UL_EXPERT, RootFrame::user_level_expert)
    EVT_MENU(CONTEXT_HELP, RootFrame::context_help)
    EVT_MENU(MANUAL, RootFrame::show_manual)
    EVT_HELP(wxID_ANY, RootFrame::show_help)
END_EVENT_TABLE()

static void create_root_frame( 
    boost::shared_ptr<Node> root_node, 
    boost::shared_ptr<RootFrame*> frame_storage,
    boost::shared_ptr<VisibilityControl> vc,
    boost::shared_ptr<Window> root_window_view
) {
    *frame_storage = new RootFrame(root_node, vc);
    *root_window_view = *frame_storage;
}

MainConfig::MainConfig( dStorm::MainThread& main_thread, const dStorm::job::Config& c, boost::shared_ptr<Node> n )
: main_thread( main_thread ),
  config(c),
  starter( &main_thread, boost::shared_ptr<simparm::Node>(), config )
{
    boost::shared_ptr< Node > main_part( new ScrolledTabNode( n ) );
    main_part->initialization_finished();

    starter.set_attachment_point( main_part );
    config.attach_ui( main_part );
    starter.attach_ui( config.user_interface_handle() );
}

RootNode::RootNode( dStorm::MainThread& main_thread ) 
: my_frame( new RootFrame*(NULL) ),
  window_view( new Window() ),
  vc( new VisibilityControl() ),
  main_thread( main_thread )
{
    main_thread.register_unstopable_job();
}

RootNode::~RootNode() {
    main_thread.unregister_unstopable_job();
}

void RootNode::initialization_finished() {
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        boost::bind( &create_root_frame, shared_from_this(), my_frame, vc, window_view ) );
}

boost::shared_ptr<RootNode> RootNode::create( dStorm::MainThread& m, const dStorm::job::Config& c ) {
    boost::shared_ptr<RootNode> rv( new RootNode(m) );
    rv->initialization_finished();
    boost::shared_ptr< MainConfig > main_config( new MainConfig(m, c, rv) );
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        bl::bind( &RootFrame::delete_on_death, *bl::constant(rv->my_frame), main_config ) );
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        bl::bind( &wxFrame::Show, *bl::constant(rv->my_frame), true ) );
    return rv;
}

void RootNode::add_entry_line( LineSpecification& ) { throw std::logic_error("Unexpected top level element"); }

static void register_main_window(
    boost::shared_ptr<RootFrame*> frame,
    boost::shared_ptr<Window> subwindow
) {
    (*frame)->add_window( *subwindow );
}

void RootNode::add_full_width_line( WindowSpecification& w ) {
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
         boost::bind( &register_main_window, my_frame, w.window ) );
}

NodeHandle RootNode::create_object( std::string ) {
    return NodeHandle( new WindowNode( shared_from_this() ) );
}

NodeHandle RootNode::create_group( std::string ) {
    return NodeHandle( new WindowNode( shared_from_this() ) );
}

Node::Relayout RootNode::get_relayout_function() {
    throw std::logic_error("Root node reached in relayout, not implemented here, expected scrolled window child");
}

void RootNode::attach_context_help( boost::shared_ptr<Window> window, std::string context_help_id ) {
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        bl::bind( &RootFrame::attach_context_help, *bl::constant(my_frame), *bl::constant(window), context_help_id )
    );
}

void show_message( const Message& m, boost::shared_ptr<RootFrame*> parent ) {
    int style = wxOK;
    switch ( m.severity ) {
        case Message::Question: style |= wxICON_QUESTION; break;
        case Message::Debug:
        case Message::Info:     style |= wxICON_INFORMATION; break;
        case Message::Warning:  style |= wxICON_EXCLAMATION; break;
        case Message::Error:
        case Message::Critical: style |= wxICON_ERROR; break;
    }

    wxMessageDialog( *parent, 
        wxString(m.message.c_str(), wxConvUTF8),
        wxString(m.title.c_str(), wxConvUTF8),
        style, wxDefaultPosition ).ShowModal();
}

Message::Response RootNode::send( Message& m ) const {
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread( boost::bind(&show_message, m, my_frame) );
    return Message::OKYes;
}

}
}
