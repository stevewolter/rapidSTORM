
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

#include "Entry.h"
#include <stdlib.h>
#include <sstream>
#include "iostream.h"
#include "Node.h"
#include "Attribute.hpp"
#include "Entry.hpp"

#define INSTANTIATE(x) \
    template class Entry<x>; \
    template bool Attribute< boost::optional<x> >::valueChange(const boost::optional<x>&, bool); \
    template bool Attribute<x>::valueChange(const x&, bool);

namespace simparm {
    INSTANTIATE(string);
    INSTANTIATE(bool);
    INSTANTIATE(short int);
    INSTANTIATE(int);
    INSTANTIATE(long int);
    INSTANTIATE(unsigned short int);
    INSTANTIATE(unsigned int);
    INSTANTIATE(unsigned long int);
    INSTANTIATE(float);
    INSTANTIATE(double);
    INSTANTIATE(long double);
};
