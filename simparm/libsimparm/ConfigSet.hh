
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

#if !defined(CONFIGSET_HH)
#define CONFIGSET_HH

#include "ConfigObject.hh"

namespace simparm {
class ConfigSet : public virtual ConfigObject {
  private:
    std::string getPrefix() const throw() { return "forSet "; }
    virtual std::string getTypeDescriptor() const throw() 
        { return "ConfigSet"; }
  protected:
    ConfigSet() throw();
  public:
    ConfigSet(std::string name, std::string desc) throw();
    ConfigSet(const ConfigSet &set);
    virtual ~ConfigSet();
    ConfigSet* clone() const { return new ConfigSet(*this); }

    void help(std::ostream &o) const throw();

    bool readConfig(int argc, char *argv[]) throw();
    void printHelp(std::ostream &) const throw() {}
};
}

#endif

