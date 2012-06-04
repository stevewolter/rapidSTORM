#ifndef SIMPARM_WX_UI_NODE_H
#define SIMPARM_WX_UI_NODE_H

#include <simparm/Node.h>
#include <simparm/Message.h>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include "VisibilityNode.h"

class wxSizer;
class wxTreebook;

namespace simparm {
namespace wx_ui {

class TreeRepresentation;

struct LineSpecification : public boost::noncopyable {
    boost::shared_ptr<Window> label;
    boost::shared_ptr<Window> contents;
    boost::shared_ptr<Window> adornment;
    std::vector< boost::function0<void> > removal_instructions;

    LineSpecification( boost::function0<void> redraw_function ) ;
};

struct WindowSpecification : public boost::noncopyable {
    boost::shared_ptr<Window> window;
    std::string name;
    int proportion;
    std::vector< boost::function0<void> > removal_instructions;

    WindowSpecification();
    ~WindowSpecification();
};

struct SizerSpecification : public boost::noncopyable {
    boost::shared_ptr<wxSizer*> sizer;
    int proportion;
    std::vector< boost::function0<void> > removal_instructions;

    SizerSpecification()
        : sizer( new wxSizer*(NULL) ), proportion(0) {}
};

struct Node
: public simparm::Node,
  public boost::enable_shared_from_this<Node>,
  private boost::noncopyable
{
    typedef boost::function0<void> Relayout;

    virtual void add_entry_line( LineSpecification& line ) = 0;
    virtual void add_full_width_line( WindowSpecification& w ) = 0;
    virtual void add_full_width_sizer( SizerSpecification& w ) = 0;
    virtual boost::shared_ptr< Window > get_parent_window() = 0;
    virtual boost::shared_ptr< Window > get_treebook_widget() = 0;
    virtual boost::shared_ptr< TreeRepresentation > get_treebook_parent() = 0;
    virtual boost::shared_ptr< VisibilityControl > get_visibility_control() = 0;
    virtual void bind_visibility_group( boost::shared_ptr<Window> vg ) = 0;
    /* Create a function object that causes a relayout of the surrounding windows.
     * The function object must be called in the GUI thread. */
    virtual Relayout get_relayout_function() = 0;

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

};

class InnerNode 
: public Node
{
    boost::shared_ptr<Node> parent;
    VisibilityNode visibility;

public:
    virtual void set_visibility( bool b ) { visibility.set_visibility( b ); }
    virtual void set_user_level( UserLevel u ) { visibility.set_user_level( u ); }
    virtual void set_description( std::string d ) {}
    virtual void set_help_id( std::string ) {}
    virtual void set_help( std::string ) {}
    virtual void set_editability( bool ) {}

protected:
    bool is_visible() const { return visibility.is_visible(); }
    void notify_on_visibility_change( boost::signals2::slot<void(bool)> s ) { visibility.add_listener(s); }
    void add_visibility_group( boost::shared_ptr<Window> vg ) 
        { visibility.add_group(vg); }

    virtual void add_entry_line( LineSpecification& line ) { 
        add_visibility_group( line.label );
        add_visibility_group( line.contents );
        add_visibility_group( line.adornment );
        parent->add_entry_line( line ); 
    }
    virtual void add_full_width_line( WindowSpecification& w ) { 
        add_visibility_group( w.window );
        parent->add_full_width_line( w ); 
    }
    virtual void add_full_width_sizer( SizerSpecification& w )
        { parent->add_full_width_sizer( w ); }
    virtual boost::shared_ptr< Window > get_parent_window() 
        { return parent->get_parent_window(); }
    virtual boost::shared_ptr< Window > get_treebook_widget() 
        { return parent->get_treebook_widget(); }
    virtual boost::shared_ptr< TreeRepresentation > get_treebook_parent() 
        { return parent->get_treebook_parent(); }
    virtual boost::shared_ptr< VisibilityControl > get_visibility_control() 
        { return parent->get_visibility_control(); }
    virtual void bind_visibility_group( boost::shared_ptr<Window> vg )
        { add_visibility_group(vg); parent->bind_visibility_group(vg); }
    /* Create a function object that causes a relayout of the surrounding windows.
     * The function object must be called in the GUI thread. */
    typedef boost::function0<void> Relayout;
    virtual Relayout get_relayout_function() 
        { return parent->get_relayout_function(); }

    void create_static_text( boost::shared_ptr<Window> into, std::string text );

public:
    InnerNode( boost::shared_ptr<Node> parent ) 
        : parent(parent), visibility( *parent->get_visibility_control() ) {}
    ~InnerNode() {}

    void add_attribute( simparm::BaseAttribute& ) {}
    Message::Response send( Message& m ) const { return Message::OKYes; }
    void initialization_finished() {}
    void hide() {}
    /** TODO: Method is deprecated and should be removed on successful migration. */
    bool isActive() const { return true; }

    NodeHandle get_handle() { return shared_from_this(); }
    static void run_in_GUI_thread( boost::function0<void> );
};

}
}

#endif