
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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

abstract class ConfigEntryTextbox 
extends ConfigEntry implements ActionListener 
{
	protected JTextField textField;

	public ConfigEntryTextbox() {
		super();
                JTextField field;
		setField(new JTextField());
	}

	public void setField(JComponent component) {
		super.setField(component);
		if (component instanceof JTextField) {
			this.textField = (JTextField)component;
			textField.addActionListener(this);
		}
	}

	public abstract boolean setValue(String value, boolean doprint, boolean updatefield);

	public void actionPerformed(ActionEvent e) {
	    setValue(textField.getText(), true, false);
	}
}

