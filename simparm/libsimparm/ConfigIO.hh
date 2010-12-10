
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
#include "ConfigObject.hh"

namespace simparm {

using std::istream;
using std::ostream;
class ConfigIO : public ConfigObject {
  private:
    istream *in;
    ostream *out;
    void *subthread_if_any, *mutex;
    bool detached;

    static void *processInputCallback(void *configIO) throw();
    virtual std::string getTypeDescriptor() const throw() { return "IO"; }
    std::string getPrefix() const throw() { return ""; }

    void define(ConfigNode *node) throw() { print( node->define() ); }
    void undefine(ConfigNode *node) throw() { print( node->undefine() ); }

  public:
    ConfigIO(istream* in, ostream* out) throw();
    ~ConfigIO() throw();
    ConfigIO* clone() const { return new ConfigIO(this->in, this->out); }

    ConfigAttribute<bool> remoteAttached;

    void processInput() throw();
    void forkInput() throw(std::runtime_error);
    void detachInput() throw(std::runtime_error);

    void processCommand(istream& in) throw();
    void print(const std::string& what) throw();
    void printHelp(std::ostream &) const throw() {}
};
}

#endif
