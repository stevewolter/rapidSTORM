#ifndef SIMPARM_WX_UI_NODE_H
#define SIMPARM_WX_UI_NODE_H

#include <simparm/Node.h>
#include <simparm/Message.h>
#include <boost/smart_ptr/enable_shared_from_this.hpp>

class wxStaticText;
class wxSizer;
class wxWindow;

namespace simparm {
namespace wx_ui {

struct LineSpecification {
    boost::shared_ptr<wxStaticText*> label;
    boost::shared_ptr<wxWindow*> contents;
    boost::shared_ptr<wxWindow*> adornment;

    LineSpecification() 
        : label( new wxStaticText*(NULL) ), 
          contents( new wxWindow*(NULL) ), 
          adornment( new wxWindow*(NULL) ) {}
};

struct WindowSpecification {
    boost::shared_ptr<wxWindow*> window;
    std::string name;

    WindowSpecification()
        : window( new wxWindow*(NULL) ) {}
};

struct Node 
: public simparm::Node, 
  public boost::enable_shared_from_this<Node>, 
  private boost::noncopyable
{
    boost::shared_ptr<Node> parent;

    virtual void set_visibility( bool ) {}
    virtual void set_user_level( UserLevel ) {}
    virtual void set_description( std::string d ) {}
    virtual void set_help_id( std::string ) {}
    virtual void set_help( std::string ) {}
    virtual void set_editability( bool ) {}

protected:
    virtual void add_entry_line( const LineSpecification& line )
        { parent->add_entry_line( line ); }
    virtual void add_full_width_line( WindowSpecification w )
        { parent->add_full_width_line( w ); }
    virtual boost::shared_ptr< wxWindow* > get_parent_window() 
        { return parent->get_parent_window(); }

    void create_static_text( boost::shared_ptr<wxStaticText*> into, std::string text );
    void create_static_text( boost::shared_ptr<wxWindow*> into, std::string text );

public:
    Node( boost::shared_ptr<Node> parent ) : parent(parent) {}
    ~Node() {}

    NodeHandle create_object( std::string name ) { return NodeHandle( new Node(shared_from_this()) ); }
    NodeHandle create_textfield( std::string name, std::string type );
    NodeHandle create_checkbox( std::string name );
    NodeHandle create_group( std::string name ) { return NodeHandle( new Node(shared_from_this()) ); }
    NodeHandle create_tab_group( std::string name );
    NodeHandle create_choice( std::string name );
    NodeHandle create_file_entry( std::string name );
    NodeHandle create_progress_bar( std::string name );
    NodeHandle create_trigger( std::string name );
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
