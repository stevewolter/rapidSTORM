
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

package au.com.pulo.kev.simparm;

import java.io.*;

class ConfigEntryString extends ConfigEntryTextbox {

	protected String value;

	public ConfigEntryString() {
		super();
		value = "";
	}

	public boolean setValue(String newvalue, boolean doprint, boolean updatefield) {
		if (value != newvalue) {
			value = newvalue;
			if (doprint) {
				doPrint();
			}
		}
		if (updatefield) {
			textField.setText(value);
		}
		return true;
	}

	public boolean processAttribute(String name, String value)
            throws IOException
        {
            if (name.equals("value") || name.equals("=")) {
		setValue(value, false, true);
		return true;
            } else
                return super.processAttribute(name, value);
	}

        protected String getStringValue() { return value; }

        public void commitChanges() {
            if ( ! textField.getText().equals( value ) )
                setValue(textField.getText(), true, false);
        }
}

