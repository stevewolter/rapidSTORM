#ifndef SIMPARM_NODE
#define SIMPARM_NODE

#include <string>
#include "simparm/Message.h"
#include "simparm/BaseAttribute.h"
#include "simparm/NodeHandle.h"
#include "simparm/UserLevel.h"

namespace dStorm { namespace display { class WindowProperties; class DataSource; class WindowHandle; } }
namespace dStorm { class Job; }

namespace simparm {

class Node {
  public:
    virtual ~Node() {}
    virtual NodeHandle create_object( std::string name ) = 0;
    virtual NodeHandle create_textfield( std::string name, std::string type ) = 0;
    virtual NodeHandle create_checkbox( std::string name ) = 0;
    virtual NodeHandle create_group( std::string name ) = 0;
    virtual NodeHandle create_tab_group( std::string name ) = 0;
    virtual NodeHandle create_choice( std::string name ) = 0;
    virtual NodeHandle create_file_entry( std::string name ) = 0;
    virtual NodeHandle create_progress_bar( std::string name ) = 0;
    virtual NodeHandle create_trigger( std::string name ) = 0;
    virtual NodeHandle create_tree_root() = 0;
    virtual NodeHandle create_tree_object( std::string name ) = 0;
    virtual NodeHandle create_job_node( std::string name ) { return create_group( name ); }
    virtual std::auto_ptr<dStorm::display::WindowHandle> get_image_window( 
        const dStorm::display::WindowProperties&, dStorm::display::DataSource& ) = 0;

    virtual void add_attribute( simparm::BaseAttribute& ) = 0;
    virtual Message::Response send( Message& m ) const = 0;
    virtual void initialization_finished() = 0;
    /** TODO: Method is deprecated and should be removed on successful migration. */
    virtual bool isActive() const = 0;
    virtual void set_description( std::string ) = 0;
    virtual void set_visibility( bool ) = 0;
    virtual void set_user_level( UserLevel arg ) = 0;
    virtual void set_help_id( std::string ) = 0;
    virtual void set_help( std::string ) = 0;
    virtual void set_editability( bool ) = 0;

    virtual void stop_job_on_ui_detachment( boost::shared_ptr<dStorm::Job> ) {}
};

}

#endif
