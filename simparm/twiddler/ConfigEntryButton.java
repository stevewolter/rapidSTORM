
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
import java.awt.event.*;

abstract class ConfigEntryButton extends ConfigEntry implements ActionListener {

	public JButton button;

	public ConfigEntryButton() {
		super();
		setField(new JButton());
	}

	public void setField(JComponent component) {
		super.setField(component);
		if (component instanceof JButton) {
			button = (JButton) component;
			button.setText(getDesc());
			button.addActionListener(this);
		}
	}

        public void setDesc(String name) {
		super.setDesc(name);
		button.setText(name);
        }

	public abstract void buttonPressed(boolean doprint);

	public void actionPerformed(ActionEvent e) {
		buttonPressed(true);
	}

	public void mouseClicked(MouseEvent e) {
		super.mouseClicked(e);
	}

}

