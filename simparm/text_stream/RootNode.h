#ifndef SIMPARM_TEXT_STREAM_ROOT_NODE_HH
#define SIMPARM_TEXT_STREAM_ROOT_NODE_HH

#include <iostream>
#include "Node.h"

namespace simparm {
namespace text_stream {

class RootNode : public Node {
  private:
    std::istream *in;
    std::ostream *out;
    void *mutex;
    bool is_attached, should_quit;

    void print_unconditionally( const std::string& what );

  protected:
    virtual void processCommand( 
        const std::string& cmd, std::istream& rest );

  public:
    /** Constructor setting used input and output streams. Either stream
     *  may be set to NULL, in which case the respective stream will not
     *  be used. */
    RootNode(std::istream* in, std::ostream* out);
    ~RootNode();

    void set_input_stream( std::istream *in );
    void set_output_stream( std::ostream *out );

    bool print(const std::string& what);
    bool print_on_top_level(const std::string& what);
    Message::Response send( Message &m ) const;

    bool received_quit_command() const 
        { return should_quit; }

    using Node::processCommand;
};

}
}

#endif
