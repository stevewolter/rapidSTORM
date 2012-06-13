#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "RootNode.h"
#include "ScrolledTabNode.h"
#include "App.h"
#include "VisibilityControl.h"
#include "lambda.h"
#include <wx/notebook.h>
#include <boost/smart_ptr/make_shared.hpp>
#include <wx/helpbase.h>
#include <wx/cshelp.h>
#include <wx/wxhtml.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <fstream>
#include "gui_thread.h"
#include "config_file.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include "alignment_fitter.h"

namespace simparm {
namespace wx_ui {

class HelpResolver {
    std::auto_ptr<wxHtmlHelpController> help_controller;
    std::map< wxWindow*, std::string > context_help_ids;
    std::map< std::string, std::string > help_alias;

    boost::filesystem::path help_file() const {
        return dStorm::program_data_path() / "manual.zip";
    }

    void read_alias_table() {
        help_alias.clear();
        boost::filesystem::ifstream i(dStorm::program_data_path() / "alias.h");
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

    void init_help_controller()
    {
        if ( help_controller.get() ) return;
        help_controller.reset( new wxHtmlHelpController() );
        wxString book_name( (help_file().string()).c_str(), wxConvUTF8 );
        help_controller->AddBook( wxFileName( book_name ) );
        read_alias_table();
    }

public:
    HelpResolver() {}
    void associate_window_with_help( wxWindow* window, std::string help_id )
        { context_help_ids[window] = help_id; }
    void show_help( wxWindow* window ) {
        init_help_controller();
        std::string help_id = context_help_ids[window];
        if ( help_id != "" ) {
            if ( help_id[0] == '#' ) help_id = help_id.substr(1);
            std::string help_location = help_alias[ help_id ];
            help_controller->Display( wxString(help_location.c_str(), wxConvUTF8) );
        } else
            wxMessageBox(_("There is no help for this element."));
    }
    void show_manual() { 
        init_help_controller();
        help_controller->DisplayContents(); 
    }
};

class RootFrame
: public wxFrame {
    boost::shared_ptr< Node > root_node;
    boost::shared_ptr< VisibilityControl > ul_control;
    wxBoxSizer *column;
    std::auto_ptr< ConfigTemplate > make_dstorm, make_alignment_fitter;
    HelpResolver help;
    boost::shared_ptr< ScrolledTabNode > main_window;

    enum EventID {
        UL_BEGINNER, UL_INTERMEDIATE, UL_EXPERT,
        CONTEXT_HELP, MANUAL,
        SAVE_CONFIG, 
        RAPIDSTORM_LOAD_CONFIG, RAPIDSTORM_MINIMAL, RAPIDSTORM_DEFAULT,
        ALIGNMENT_MINIMAL,
    };

    void change_user_level( UserLevel l ) {
        ul_control->set_user_level( l );
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

    void save_config(wxCommandEvent&) {
        wxFileDialog dialog(this, _("Set config file name"), wxT(""), wxT(""), wxT("*.txt"), wxFD_SAVE);
        int response = dialog.ShowModal();
        if ( response == wxID_OK ) {
            wxString file = dialog.GetPath();
            main_window->serialize_current_tab( std::string(file.mb_str()) );
        }
    }

    void load_config(wxCommandEvent&) {
        wxFileDialog dialog(this, _("Choose config file"), wxT(""), wxT(""), wxT("*.txt"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        int response = dialog.ShowModal();
        if ( response == wxID_OK ) {
            wxString file = dialog.GetPath();
            make_dstorm->create_config( std::string(file.mb_str()) ).release();
        }
    }

    void make_rapidstorm_minimal(wxCommandEvent&) { 
        MainConfig::ui_managed( make_dstorm->create_config() );
    }

    void make_alignment_minimal(wxCommandEvent&) { 
        MainConfig::ui_managed( make_alignment_fitter->create_config() );
    }

    void make_rapidstorm_default(wxCommandEvent&) { 
        MainConfig::ui_managed( make_dstorm->create_config( dStorm::initialization_file().string() ) );
    }

public:
    static wxString get_title() {
        std::stringstream s;
        s << PACKAGE_NAME << " " << PACKAGE_MAJOR_VERSION << "." << PACKAGE_MINOR_VERSION;
        return wxString( s.str().c_str(), wxConvUTF8 );
    }
    RootFrame( boost::shared_ptr<Node> root_node, boost::shared_ptr< VisibilityControl > vc )
        : wxFrame( NULL, wxID_ANY, get_title(), wxDefaultPosition, wxSize(800,1000) ),
          root_node( root_node ), 
          ul_control( vc ),
          column( new wxBoxSizer(wxVERTICAL) )
    {
        wxMenuBar* menu = new wxMenuBar();

        wxMenu* menu_new = new wxMenu();

        wxMenu* rapidSTORM = new wxMenu();
        rapidSTORM->Append( RAPIDSTORM_MINIMAL, _("Minimal") );
        rapidSTORM->Append( RAPIDSTORM_DEFAULT, _("Default") );
        rapidSTORM->Append( RAPIDSTORM_LOAD_CONFIG, _("From &file ...") );
        menu_new->AppendSubMenu( rapidSTORM, _("&rapidSTORM") );

        wxMenu* alignment_fitter = new wxMenu();
        alignment_fitter->Append( ALIGNMENT_MINIMAL, _("Minimal") );
        menu_new->AppendSubMenu( alignment_fitter, _("&Alignment fitter") );

        menu_new->AppendSeparator();
        menu_new->Append( SAVE_CONFIG, _("&Save ...") );
        menu->Append( menu_new, _("&Job") );

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

    ~RootFrame() {}

    void add_window( wxWindow* window ) {
        column->Add( window, 1, wxEXPAND );
        Layout();
    }

    void set_main_config( dStorm::JobConfig* config, boost::shared_ptr<ScrolledTabNode> main_window ) {
        this->main_window = main_window;
        this->make_dstorm.reset( new ConfigTemplate( std::auto_ptr<dStorm::JobConfig>(config), main_window ) );
        this->make_alignment_fitter.reset( new ConfigTemplate( make_alignment_fitter_config(), main_window ) );
        MainConfig::ui_managed( make_dstorm->create_config( dStorm::initialization_file().string() ) );
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
    EVT_MENU(RAPIDSTORM_LOAD_CONFIG, RootFrame::load_config)
    EVT_MENU(SAVE_CONFIG, RootFrame::save_config)
    EVT_MENU(RAPIDSTORM_MINIMAL, RootFrame::make_rapidstorm_minimal)
    EVT_MENU(RAPIDSTORM_DEFAULT, RootFrame::make_rapidstorm_default)
    EVT_MENU(ALIGNMENT_MINIMAL, RootFrame::make_alignment_minimal)
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

RootNode::RootNode( ) 
: my_frame( new RootFrame*(NULL) ),
  window_view( new Window() ),
  vc( new VisibilityControl() ),
  protocol( NULL )
{
}

RootNode::~RootNode() {
    logfile.reset();
    if ( logfile_name ) {
        remove( *logfile_name );
    }
}

void RootNode::initialization_finished() {
    run_in_GUI_thread(
        boost::bind( &create_root_frame, shared_from_this(), my_frame, vc, window_view ) );
}

boost::shared_ptr<RootNode> RootNode::create( const dStorm::JobConfig& c, boost::optional<std::string> logfile ) {
    boost::shared_ptr<RootNode> rv( new RootNode() );
    if ( logfile ) {
        rv->logfile_name = *logfile;
        rv->logfile = boost::in_place( logfile->c_str() );
        rv->protocol = ProtocolNode( rv->logfile.get_ptr() );
    }
    rv->initialization_finished();
    boost::shared_ptr< ScrolledTabNode > main_part( new ScrolledTabNode( rv, "ScrolledTabNode" ) );
    main_part->initialization_finished();

    run_in_GUI_thread(
        bl::bind( &RootFrame::set_main_config, *bl::constant(rv->my_frame), 
            c.clone(), main_part ) );
    run_in_GUI_thread(
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
    run_in_GUI_thread(
         boost::bind( &register_main_window, my_frame, w.window ) );
}

NodeHandle RootNode::create_object( std::string name ) {
    return NodeHandle( new WindowNode( shared_from_this(), name ) );
}

NodeHandle RootNode::create_group( std::string name ) {
    return NodeHandle( new WindowNode( shared_from_this(), name ) );
}

Node::Relayout RootNode::get_relayout_function() {
    throw std::logic_error("Root node reached in relayout, not implemented here, expected scrolled window child");
}

void RootNode::attach_context_help( boost::shared_ptr<Window> window, std::string context_help_id ) {
    run_in_GUI_thread(
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
    run_in_GUI_thread( boost::bind(&show_message, m, my_frame) );
    return Message::OKYes;
}

}
}
