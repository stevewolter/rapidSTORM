
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


class ConfigEntryBool extends ConfigEntryCheckbox {

	public boolean value;

	public ConfigEntryBool() {
		super();
		value = false;
	}

	public void setValue(boolean newvalue, boolean doprint, boolean updatefield) {
		if (value != newvalue) {
			value = newvalue;
			if (doprint) {
				doPrint();
			}
		}
		if (updatefield) {
			displayState(value);
		}
	}

	public boolean setValueFromInputStream(String word, 
            BufferedReader stream) throws IOException 
        {
            return setFromString(word);
        }

        public boolean setFromString(String word) {
		if ( word.equalsIgnoreCase("1") || word.equalsIgnoreCase("true") || word.equalsIgnoreCase("t") || word.equalsIgnoreCase("yes") || word.equalsIgnoreCase("y") ) {
			setValue(true, false, true);
			return true;
		} else if ( word.equalsIgnoreCase("0") || word.equalsIgnoreCase("false") || word.equalsIgnoreCase("f") || word.equalsIgnoreCase("no") || word.equalsIgnoreCase("n") ) {
			setValue(false, false, true);
			return true;
		} else {
			return false;
		}
	}

	public void toggleValue(boolean doprint, boolean updatefield) {
		setValue( ! value, doprint, updatefield);
	}


        protected String getStringValue() {
                if (value) return "true"; else return "false";
	}


        public boolean processAttribute(String name, String value)
            throws IOException 
        {
            if (name.equals("value"))
                setFromString( value );
            else 
                return super.processAttribute(name, value);

            return true;
        }
}

