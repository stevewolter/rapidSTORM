
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
import javax.swing.JMenuBar;
import java.awt.event.*;
import java.awt.Desktop;

public class ConfigMenu
extends ConfigEntry {
        public JMenuBar myBar;
	public JMenu menu;

        public ConfigMenu() {}
        public void setDesc(String desc) {
		super.setDesc(desc);
		if (desc != null && !desc.equals("") && menu == null) {
                    menu = new JMenu(desc);
                    //setField( menu );
                }
        }

        protected void declareDescriptedPanels(DeclarationInfo superInfo) {
            DeclarationInfo info = null;
            if ( superInfo != null ) info = new DeclarationInfo(superInfo);
            if ( info != null && info.menuBar != null && menu != null ) {
                myBar = info.menuBar;
                myBar.add( menu );
                myBar.validate();
            } else if ( info == null && myBar != null ) {
                myBar.remove( menu );
                myBar = null;
            }

            if ( info != null ) {
                info.menu = menu;
            }
            for (ConfigObject o : children)
                o.declareDescriptedPanels(info);
        }
        void commitChanges() {}

}

