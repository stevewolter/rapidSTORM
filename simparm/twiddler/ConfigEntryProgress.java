
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

//package simparm.twiddler;

import java.io.*;
import javax.swing.JProgressBar;

class ConfigEntryProgress extends ConfigEntryDouble {
        private double value, min = 0, max = 1;
        private JProgressBar progress = new JProgressBar(0, 100);;

        ConfigEntryProgress() {
                textField = null;
                setField( progress );
        }

        protected void makeField() {}

	public void setValue(Double newvalue,
            boolean doprint, boolean updatefield, boolean updateslider) 
        {
            super.setValue( newvalue, doprint, false, false );
            if ( progress != null )
                if ( newvalue != null && updatefield )
                    progress.setValue((int)(newvalue * 100));
        }

        protected String getStringValue() { return String.valueOf(value); }
}

