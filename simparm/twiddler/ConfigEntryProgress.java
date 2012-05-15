
// SimParm: Simple and flexible C++ configuration framework
// Copyright (C) 2008 Bielefeld University
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
// Steve Wolter
// twiddler@swolter.sdf1.org

package au.com.pulo.kev.simparm;

import java.io.*;
import javax.swing.JProgressBar;

class ConfigEntryProgress extends ConfigEntryFloat {
        private double value, min = 0, max = 1;
        private JProgressBar progress = new JProgressBar(0, 100);;
        private boolean isIndeterminate = false;

        ConfigEntryProgress() {
                setField( progress );
        }

        protected void makeField() {}

	public void setValue(double val)
        {
            if ( progress != null ) {
                if ( isIndeterminate ) {
                    if ( val >= 0.999 ) {
                        progress.setIndeterminate(false);
                        progress.setValue(100);
                    } else if ( val <= 0.001 ) {
                        progress.setIndeterminate(false);
                        progress.setValue(0);
                    } else {
                        progress.setIndeterminate(true);
                    }
                } else {
                    progress.setValue((int)(val * 100));
                }
            }
        }

        protected String getStringValue() { return null; }

	public boolean processAttribute(String name, String value)
            throws IOException
        {
            if ( name.equals("value") ) {
                double val = 0;
                val = Double.parseDouble(value);
                setValue(val);
                return true;
            } else if ( name.equals("indeterminate") && value.equals("true") ) {
                isIndeterminate = true;
                return true;
            } else 
                return super.processAttribute(name,value);
        }
}

