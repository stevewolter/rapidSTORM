#include "job/OutputTreeRoot.h"

namespace dStorm {
namespace job {

OutputTreeRoot::OutputTreeRoot()
: output::FilterSource(),
  name_object("EngineOutput", "dSTORM engine output"),
  cap( output::Capabilities()
            .set_source_image()
            .set_smoothed_image()
            .set_candidate_tree()
            .set_input_buffer() )
{
    {
        output::Config exemplar;
        this->set_output_factory( exemplar );
    }
    this->set_source_capabilities( cap );
    my_config = dynamic_cast<output::Config*>(getFactory());
    assert( my_config != NULL );
}

OutputTreeRoot::OutputTreeRoot( const OutputTreeRoot& other )
: output::FilterSource( other),
    tree_root(other.tree_root),
    name_object(other.name_object)
{
    this->set_output_factory( *other.my_config );
    my_config = dynamic_cast<output::Config*>(getFactory());
}

void OutputTreeRoot::set_trace_capability( const input::Traits<output::LocalizedImage>& t )
{
    cap.set_source_image( t.input_image_traits.get() != nullptr );
    cap.set_smoothed_image( t.smoothed_image_is_set );
    cap.set_candidate_tree( t.candidate_tree_is_set );
    cap.set_cluster_sources( ! t.source_traits.empty() );
    this->set_source_capabilities( cap );
}

void OutputTreeRoot::attach_full_ui( simparm::NodeHandle at ) {
    simparm::NodeHandle n = tree_root.attach_ui(at);
    simparm::NodeHandle r = name_object.attach_ui(n);
    attach_children_ui( r ); 
}

}
}
