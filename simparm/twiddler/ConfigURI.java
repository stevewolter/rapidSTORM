
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

import javax.swing.JMenu;
import javax.swing.JMenuItem;
import java.awt.event.*;
import java.awt.Desktop;

class ConfigURI
extends ConfigEntry implements ActionListener {
        public java.net.URI uri;
        public JMenu myMenu;
	public JMenuItem item;

        public void setDesc(String desc) {
		super.setDesc(desc);
		if (item == null && desc != null && !desc.equals("") ) {
                    item = new JMenuItem(desc);
                    item.addActionListener(this);
                    setField( item );
                }
        }

        public boolean processAttribute(String name, String value) throws java.io.IOException {
            if ( name.equals("uri") ) {
                try {
                    uri = new java.net.URI( value );
                } catch (Exception e) {}
            } else
                return super.processAttribute(name, value);
            return true;
        }
	public void actionPerformed(ActionEvent e) {
            if ( uri != null ) {
              try {
                Desktop.getDesktop().browse( uri );
              } catch (java.io.IOException exc) {
                System.err.println(exc.getMessage());
              }
            }
	}

        protected void declareDescriptedPanels(DeclarationInfo info) {
            if ( info != null && info.menu != null && item != null ) {
                myMenu = info.menu;
                myMenu.add( item );
            } else if ( info == null && myMenu != null ) {
                myMenu.remove( item );
                myMenu = null;
            }
        }
        void commitChanges() {}
}

