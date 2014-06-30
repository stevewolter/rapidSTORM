#include "job/OutputTreeRoot.h"

namespace dStorm {
namespace job {

OutputTreeRoot::OutputTreeRoot()
: output::FilterSource(),
  name_object("EngineOutput", "dSTORM engine output")
{
    {
        output::Config exemplar;
        this->set_output_factory( exemplar );
    }
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

void OutputTreeRoot::attach_full_ui( simparm::NodeHandle at ) {
    simparm::NodeHandle n = tree_root.attach_ui(at);
    simparm::NodeHandle r = name_object.attach_ui(n);
    attach_children_ui( r ); 
}

}
}
