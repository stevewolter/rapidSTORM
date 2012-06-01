#include "../Node.h"
#include "fwd.h"

namespace simparm {
namespace dummy_ui {

struct Node : public simparm::Node {
    virtual ~Node() {}
    virtual NodeHandle create_object( std::string ) { return make_node(); }
    virtual NodeHandle create_textfield( std::string, std::string ) { return make_node(); }
    virtual NodeHandle create_checkbox( std::string ) { return make_node(); }
    virtual NodeHandle create_group( std::string ) { return make_node(); }
    virtual NodeHandle create_tab_group( std::string ) { return make_node(); }
    virtual NodeHandle create_choice( std::string ) { return make_node(); }
    virtual NodeHandle create_file_entry( std::string ) { return make_node(); }
    virtual NodeHandle create_progress_bar( std::string ) { return make_node(); }
    virtual NodeHandle create_trigger( std::string ) { return make_node(); }
    virtual NodeHandle create_tree_root() { return make_node(); }
    virtual NodeHandle create_tree_object( std::string ) { return make_node(); }
    virtual std::auto_ptr<dStorm::display::WindowHandle> get_image_window( 
        const dStorm::display::WindowProperties&, dStorm::display::DataSource& )
    {
        throw std::runtime_error("Dummy user interface was asked to produce a display window");
    }

    virtual void add_attribute( simparm::BaseAttribute& ) {}
    virtual Message::Response send( Message& m ) const { return Message::OKYes; }
    virtual void initialization_finished() {}
    /** TODO: Method is deprecated and should be removed on successful migration. */
    virtual bool isActive() const { return true; }
    virtual void set_description( std::string ) {}
    virtual void set_visibility( bool ) {}
    virtual void set_user_level( UserLevel arg ) {}
    virtual void set_help_id( std::string ) {}
    virtual void set_help( std::string ) {}
    virtual void set_editability( bool ) {}
};

NodeHandle make_node() { return NodeHandle( new Node() ); }

}
}
