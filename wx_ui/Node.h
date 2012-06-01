#ifndef SIMPARM_WX_UI_NODE_H
#define SIMPARM_WX_UI_NODE_H

#include <simparm/Node.h>
#include <simparm/Message.h>
#include <boost/smart_ptr/enable_shared_from_this.hpp>

class wxSizer;
class wxWindow;
class wxTreebook;

namespace simparm {
namespace wx_ui {

class TreeRepresentation;
class VisibilityControl;

struct LineSpecification {
    boost::shared_ptr<wxWindow*> label;
    boost::shared_ptr<wxWindow*> contents;
    boost::shared_ptr<wxWindow*> adornment;

    LineSpecification() 
        : label( new wxWindow*(NULL) ), 
          contents( new wxWindow*(NULL) ), 
          adornment( new wxWindow*(NULL) ) {}
};

struct WindowSpecification {
    boost::shared_ptr<wxWindow*> window;
    std::string name;

    WindowSpecification()
        : window( new wxWindow*(NULL) ) {}
};

struct SizerSpecification {
    boost::shared_ptr<wxSizer*> sizer;

    SizerSpecification()
        : sizer( new wxSizer*(NULL) ) {}
};

struct Node 
: public simparm::Node, 
  public boost::enable_shared_from_this<Node>, 
  private boost::noncopyable
{
    boost::shared_ptr<Node> parent;
    bool is_visible;
    int user_level;

    virtual void set_visibility( bool b ) { is_visible = b; }
    virtual void set_user_level( UserLevel u ) { user_level = u; }
    virtual void set_description( std::string d ) {}
    virtual void set_help_id( std::string ) {}
    virtual void set_help( std::string ) {}
    virtual void set_editability( bool ) {}

    void add_to_visibility_control( boost::shared_ptr<wxWindow*> w );

protected:
    virtual void add_entry_line( const LineSpecification& line ) { 
        add_to_visibility_control( line.label );
        add_to_visibility_control( line.contents );
        add_to_visibility_control( line.adornment );
        parent->add_entry_line( line ); 
    }
    virtual void add_full_width_line( WindowSpecification w ) { 
        add_to_visibility_control( w.window );
        parent->add_full_width_line( w ); 
    }
    virtual void add_full_width_sizer( SizerSpecification w )
        { parent->add_full_width_sizer( w ); }
    virtual boost::shared_ptr< wxWindow* > get_parent_window() 
        { return parent->get_parent_window(); }
    virtual boost::shared_ptr< wxWindow* > get_treebook_widget() 
        { return parent->get_treebook_widget(); }
    virtual boost::shared_ptr< TreeRepresentation > get_treebook_parent() 
        { return parent->get_treebook_parent(); }
    virtual boost::shared_ptr< VisibilityControl > get_visibility_control() 
        { return parent->get_visibility_control(); }
    /* Create a function object that causes a relayout of the surrounding windows.
     * The function object must be called in the GUI thread. */
    typedef boost::function0<void> Relayout;
    virtual Relayout get_relayout_function() 
        { return parent->get_relayout_function(); }

    void create_static_text( boost::shared_ptr<wxWindow*> into, std::string text );

public:
    Node( boost::shared_ptr<Node> parent ) : parent(parent), is_visible(true), user_level(10) {}
    ~Node() {}

    NodeHandle create_object( std::string name );
    NodeHandle create_textfield( std::string name, std::string type );
    NodeHandle create_checkbox( std::string name );
    NodeHandle create_group( std::string name );
    NodeHandle create_tab_group( std::string name );
    NodeHandle create_choice( std::string name );
    NodeHandle create_file_entry( std::string name );
    NodeHandle create_progress_bar( std::string name );
    NodeHandle create_trigger( std::string name );
    NodeHandle create_tree_root();
    NodeHandle create_tree_object( std::string name );
    std::auto_ptr<dStorm::display::WindowHandle> get_image_window( 
        const dStorm::display::WindowProperties&, dStorm::display::DataSource& );

    void add_attribute( simparm::BaseAttribute& ) {}
    Message::Response send( Message& m ) const { return Message::OKYes; }
    void initialization_finished() {}
    void hide() {}
    /** TODO: Method is deprecated and should be removed on successful migration. */
    bool isActive() const { return true; }

    NodeHandle get_handle() { return shared_from_this(); }
    void run_in_GUI_thread( boost::function0<void> );
};

}
}

#endif
