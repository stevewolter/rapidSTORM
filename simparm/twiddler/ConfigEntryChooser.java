
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

//package simparm.twiddler;

import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;

abstract class ConfigEntryChooser extends ConfigEntry 
   implements ItemListener
{
        Object selected = null;
	private JComboBox chooser;
        private boolean expectChange = false;

	public ConfigEntryChooser() {
		super();
		setField(new JComboBox());
	}

	public void setField(JComponent component) {
		super.setField(component);
		if (component instanceof JComboBox) {
			chooser = (JComboBox) component;
			chooser.addItemListener(this);
		}
	}

	public abstract void setValue(Object newvalue,
                  boolean doprint, boolean updatefield);

	public void itemStateChanged(ItemEvent e) {
            if (!expectChange) {
                setValue(chooser.getSelectedItem(), true, false);
            }
	}

        protected void addChoice(Object o) { 
            expectChange = true;
            chooser.addItem(o); 
            chooser.setSelectedItem(selected);
            chooser.validate();
            expectChange = false;
        }
        protected void removeChoice(Object o) { 
            expectChange = true;
            chooser.removeItem(o); 
            if (selected == o) selected = null;
            chooser.setSelectedItem(selected);
            chooser.validate();
            expectChange = false;
        }
        protected void clearChoices() { 
            expectChange = true;
            chooser.removeAllItems(); 
            chooser.validate();
            expectChange = false;
        }

        protected void select(Object o) {
            expectChange = true;
            selected = o;
            chooser.setSelectedItem(selected);
            expectChange = false;
        }
}

