#ifndef SIMPARM_TEXT_STREAM_FRONTENDNODE_H
#define SIMPARM_TEXT_STREAM_FRONTENDNODE_H

#include <string>
#include <iosfwd>

namespace simparm {
namespace text_stream {

class FrontendNode {
    virtual void process_attribute_command_( std::string name, std::istream& ) = 0;
    virtual void declare_( std::ostream& ) = 0;

public:
    void process_attribute_command( std::string name, std::istream& o )
        { process_attribute_command_(name, o ); }
    void declare( std::ostream& o ) { declare_(o); }
};

}
}

#endif
