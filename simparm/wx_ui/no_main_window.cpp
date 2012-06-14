/* wxWidgets must be included first in Windows due to conflicts with Boost's
 * handling of windows.h. */
#include <wx/frame.h>

#include "no_main_window.h"
#include "Node.h"
#include "lambda.h"
#include "GUIHandle.h"
#include "gui_thread.h"

namespace simparm {
namespace wx_ui {

struct InvisibleDialog
: public Node {
    void add_entry_line( LineSpecification& line ) { throw std::logic_error("Invisible dialog used for non-window"); }
    virtual void add_full_width_line( WindowSpecification& w ) { throw std::logic_error("Invisible dialog used for non-window"); }
    virtual void add_full_width_sizer( SizerSpecification& w ) { throw std::logic_error("Invisible dialog used for non-window"); }
    virtual boost::shared_ptr< Window > get_parent_window() { throw std::logic_error("Invisible dialog used for non-window"); }
    virtual boost::shared_ptr< Window > get_treebook_widget() { throw std::logic_error("Invisible dialog used for non-window"); }
    virtual boost::shared_ptr< TreeRepresentation > get_treebook_parent() 
        { throw std::logic_error("Invisible dialog used for non-window"); }
    virtual boost::shared_ptr< VisibilityControl > get_visibility_control() 
        { throw std::logic_error("Invisible dialog used for non-window"); }
    virtual void bind_visibility_group( boost::shared_ptr<Window> vg ) 
        { throw std::logic_error("Invisible dialog used for non-window"); }
    virtual Relayout get_relayout_function() { throw std::logic_error("Invisible dialog used for non-window"); }
    virtual void attach_context_help( boost::shared_ptr<Window> window, std::string context_help_id ) 
        { throw std::logic_error("Invisible dialog used for non-window"); }

    void add_attribute( simparm::BaseAttribute& ) {}
    virtual Message::Response send( Message& ) const { return Message::OKYes; }
    virtual bool isActive() const { return true; }
    virtual void set_description( std::string ) {}
    virtual void set_visibility( bool ) {}
    virtual void set_user_level( UserLevel arg ) {}
    virtual void set_help_id( std::string ) {}
    virtual void set_help( std::string ) {}
    virtual void set_editability( bool ) {}
    virtual const ProtocolNode& get_protocol() const { return protocol; }

    InvisibleDialog() : protocol(NULL) {
        run_in_GUI_thread((
            *bl::constant( my_frame ) = 
            bl::bind( bl::new_ptr<wxFrame>(), static_cast<wxWindow*>(NULL), int(wxID_ANY), wxT("Nevershow") ),
            bl::bind( &wxFrame::Show, *bl::constant(my_frame), false ) ));
    }
    void initialization_finished() {}
    ~InvisibleDialog() {
        run_in_GUI_thread((
            bl::bind( &wxFrame::Destroy, *bl::constant(my_frame) ) ));
    }
private:
    GUIHandle<wxFrame> my_frame;
    ProtocolNode protocol;
};

NodeHandle no_main_window() {
    return NodeHandle( new InvisibleDialog() );
}

}
}
