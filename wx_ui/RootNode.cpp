#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "RootNode.h"
#include "ScrolledTabNode.h"
#include "wxDisplay/wxManager.h"
#include "wxDisplay/App.h"
#include "VisibilityControl.h"

namespace simparm {
namespace wx_ui {

class RootFrame
: public wxFrame {
    boost::shared_ptr< Node > root_node;
    boost::shared_ptr< VisibilityControl > ul_control;
    wxBoxSizer *column;

    enum EventID {
        UL_BEGINNER, UL_INTERMEDIATE, UL_EXPERT 
    };

    void user_level_beginner(wxCommandEvent&) { ul_control->set_user_level(10); FitInside(); }
    void user_level_intermediate(wxCommandEvent&) { ul_control->set_user_level(20); FitInside(); }
    void user_level_expert(wxCommandEvent&) { ul_control->set_user_level(30); FitInside(); }

public:
    RootFrame( boost::shared_ptr<Node> root_node )
        : wxFrame( NULL, wxID_ANY, wxT( PACKAGE_STRING ) ),
          root_node( root_node ), 
          column( new wxBoxSizer(wxVERTICAL) )
    {
        wxMenuBar* menu = new wxMenuBar();
        wxMenu* user_level = new wxMenu();
        user_level->AppendRadioItem( UL_BEGINNER, _("Beginner") );
        user_level->AppendRadioItem( UL_INTERMEDIATE, _("Intermediate") );
        user_level->AppendRadioItem( UL_EXPERT, _("Expert") );
        menu->Append( user_level, _("User level") );
        SetMenuBar( menu );

        SetSizer(column);
    }

    void add_window( wxWindow* window ) {
        column->Add( window, 1, wxEXPAND );
        Layout();
    }

    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(RootFrame, wxFrame)
    EVT_CHOICE(UL_BEGINNER, RootFrame::user_level_beginner)
    EVT_CHOICE(UL_INTERMEDIATE, RootFrame::user_level_intermediate)
    EVT_CHOICE(UL_EXPERT, RootFrame::user_level_expert)
END_EVENT_TABLE()

static void create_root_frame( 
    boost::shared_ptr<Node> root_node, 
    boost::shared_ptr<RootFrame*> frame_storage,
    boost::shared_ptr<wxWindow*> root_window_view
) {
    *frame_storage = new RootFrame(root_node);
    *root_window_view = *frame_storage;
}

static void show_root_frame( 
    boost::shared_ptr<RootFrame*> frame_storage
) {
    (*frame_storage)->Show(true);
}


RootNode::RootNode( dStorm::MainThread& main_thread, const dStorm::job::Config& c ) 
: Node( boost::shared_ptr<Node>() ),
  my_frame( new RootFrame*(NULL) ),
  window_view( new wxWindow*(NULL) ),
  vc( new VisibilityControl() ),
  main_thread( main_thread ),
  config(c),
  starter( &main_thread, boost::shared_ptr<simparm::Node>(), config )
{
}

RootNode::~RootNode() {
    main_thread.unregister_unstopable_job();
}

void RootNode::initialization_finished() {
    main_thread.register_unstopable_job();
    run_in_GUI_thread( boost::bind( &create_root_frame, shared_from_this(), my_frame, window_view ) );

    main_part.reset( new ScrolledTabNode( shared_from_this() ) );
    main_part->initialization_finished();

    starter.set_attachment_point( main_part );
    config.attach_ui( main_part );
    starter.attach_ui( config.user_interface_handle() );

    run_in_GUI_thread( boost::bind( &show_root_frame, my_frame ) );
}

boost::shared_ptr<RootNode> RootNode::create( dStorm::MainThread& m, const dStorm::job::Config& c ) {
    boost::shared_ptr<RootNode> rv( new RootNode(m,c) );
    rv->initialization_finished();
    return rv;
}

void RootNode::add_entry_line( const LineSpecification& ) { throw std::logic_error("Unexpected top level element"); }

static void register_main_window(
    boost::shared_ptr<RootFrame*> frame,
    boost::shared_ptr<wxWindow*> subwindow
) {
    (*frame)->add_window( *subwindow );
}

void RootNode::add_full_width_line( WindowSpecification w ) {
    run_in_GUI_thread( boost::bind( &register_main_window, my_frame, w.window ) );
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

}
}
