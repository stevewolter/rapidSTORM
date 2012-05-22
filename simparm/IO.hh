
// SimParm: Simple and flexible C++ configuration framework
// Copyright (C) 2007 Australian National University
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// 
// Contact:
// Kevin Pulo
// kevin.pulo@anu.edu.au
// Leonard Huxley Bldg 56
// Australian National University, ACT, 0200, Australia

#if !defined(CONFIGIO_HH)
#define CONFIGIO_HH

#include <iostream>
#include "text_stream/Node.h"
#include "Attribute.hh"

namespace simparm {

using std::istream;
using std::ostream;
class IO : public simparm::text_stream::Node {
  private:
    istream *in;
    ostream *out;
    void *subthread_if_any, *mutex;
    bool should_quit;

    static void *processInputCallback(void *configIO);
    virtual std::string getTypeDescriptor() const { return "IO"; }

  protected:
    virtual void processCommand( 
        const std::string& cmd, std::istream& rest );

  public:
    /** Constructor setting used input and output streams. Either stream
     *  may be set to NULL, in which case the respective stream will not
     *  be used. */
    IO(istream* in, ostream* out);
    ~IO();
    IO* clone() const { return new IO(this->in, this->out); }

    void set_input_stream( istream *in );
    void set_output_stream( ostream *out );

    Attribute<bool> remoteAttached;
    Attribute<bool> showTabbed;

    void processInput();
    void forkInput();
    void detachInput();

    bool print(const std::string& what);
    bool print_on_top_level(const std::string& what);
    void send( Message &m );
    void printHelp(std::ostream &) const {}

    bool received_quit_command() const 
        { return should_quit; }
    void processCommand( std::istream& i ) { Node::processCommand(i); }
};
}

#endif
