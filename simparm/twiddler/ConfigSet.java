
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

import java.util.*;
import java.io.*;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JComponent;
import javax.swing.border.TitledBorder;
import javax.swing.border.EtchedBorder;
import javax.swing.BorderFactory;
import javax.swing.JTabbedPane;
import javax.swing.JScrollPane;
import java.awt.GridBagLayout;
import javax.swing.BoxLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;

class ConfigSet extends ConfigObject {
    protected JTabbedPane tabPane = new JTabbedPane();
    private HashMap<JComponent,JScrollPane> scrollers
        = new HashMap<JComponent,JScrollPane>();

    protected boolean isDecorated = true, mayDecorate = true;

    ConfigSet() {
        super.transparent = false;
    }

    public void setDesc(String desc) {
        super.setDesc(desc);
        selfDescription();
    }

    public void setSelfDescribing(boolean is) {
        isDecorated = is;
        selfDescription();
    }

    private void selfDescription() {
        if (isDecorated && mayDecorate && getDesc() != null) {
            TitledBorder title;
            title = BorderFactory.createTitledBorder(
                BorderFactory.createEtchedBorder(EtchedBorder.RAISED),
                getDesc() );
            panel.setBorder( title );
        } else
            panel.setBorder( null );
    }

    public void add(ConfigObject object) {
        if (object instanceof ConfigSet) {
            ((ConfigSet)object).mayDecorate = !isTabbed;
            ((ConfigSet)object).selfDescription();
        }
        super.add(object);
    }
    protected void addToField(JComponent label, JComponent field,
         boolean shouldExpand, int pos)
    {
        if ( ! isTabbed) {
            super.addToField(label, field, shouldExpand, pos);
        } else if (field != null) {
            JScrollPane sp = new JScrollPane( field, 
                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED );
            scrollers.put( field, sp );
            if ( label == null || ! (label instanceof JLabel) ) {
                if ( field instanceof JPanel ) {
                    String name = ((JPanel)field).getName();
                    tabPane.add( name, sp );
                } else
                    tabPane.add( sp );
            } else
                tabPane.add( ((JLabel)label).getText(), sp );
            tabPane.setSelectedComponent( sp );
        }
    }

    protected void removeFromField(JComponent label, JComponent field)
    {
        if (!isTabbed) {
            super.removeFromField(label, field);
        } else {
            tabPane.remove( scrollers.get( field ) );
        }
    }

    public boolean processCommand(String line, BufferedReader rest)
        throws IOException 
    {
        if (line.equals("showTabbed true"))
            setTabbedness( true );
        else if (line.equals("showTabbed false"))
            setTabbedness( false );
        else
            return super.processCommand(line, rest);

        return true;
    }

    protected boolean isTabbed = false;
    protected void setTabbedness(boolean tabbed) { isTabbed = tabbed; }

    protected boolean requireOpaque() { return isTabbed; }
    protected void validate() { 
        super.validate();
        tabPane.validate(); 
        if (parent != null) parent.validate();
    }

    protected void declareDescriptedPanels(boolean inserted) {
        if ( isTabbed == false )
            super.declareDescriptedPanels(inserted);
        else {
            mayDeclare = inserted;

            for (ConfigObject o : children)
                o.declareDescriptedPanels(inserted);

            if (parent != null && 
                !(transparent && !parent.requireOpaque()) ) 
            {
                if (inserted && declaredSubPanel == null) {
                    declaredSubPanel = new SubPanel(null,tabPane,false);
                    parent.subPanelChange(null, declaredSubPanel, this, 0);
                } else if (!inserted && declaredSubPanel != null) {
                    parent.subPanelChange( declaredSubPanel, null, this, 0 );
                }
            }
        }
    }
}

