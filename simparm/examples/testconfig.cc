
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

#include <iostream>
#include "Set.hh"

class MySet : public Set {
	public:
		EntryString foobar;
		EntryDouble baz;
		EntryUnsignedLong moo;
		EntryLong fred;
		EntryBool barney;

		MySet()
		: Set()
		{
			foobar.setName("foobar");
			register_entry(&foobar);

			baz.setName("baz");
			register_entry(&baz);

			moo.setName("moo");
			register_entry(&moo);

			fred.setName("fred");
			register_entry(&fred);

			barney.setName("barney");
			register_entry(&barney);
		}

} config;

int main(int , char *[]) {
	cerr << "Initial parameter values:" << endl;
	cerr << config;

	cin >> config;
	if ( ! cin.eof()) {
		cerr << "Error reading config file" << endl;
	} else {
		cerr << "Config file read successfully:" << endl;
		cout << config;
	}

	return 0;
}

