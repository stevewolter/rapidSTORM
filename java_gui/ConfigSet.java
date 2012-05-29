
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

import java.util.*;
import java.io.*;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JComponent;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.border.Border;
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

    protected boolean isDecorated = true, mayDecorate = true, childrenScroll = true;

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
            //final JComponent text = new JLabel( getDesc() );
            //final JComponent button = new JLabel( "Foo" );
            //JPanel combined = new JPanel();
            //combined.add( text );
            //combined.add( button );
            Border b = BorderFactory.createTitledBorder( getDesc() );
            panel.setBorder( b );
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
    protected void addToField(JComponent label, JComponent checkbox, JComponent field,
         boolean shouldExpand, int pos)
    {
        if ( ! isTabbed) {
            super.addToField(label, checkbox, field, shouldExpand, pos);
        } else if (field != null && field.isVisible() ) {
            JComponent c = null;
            if ( ! declarationInfo.is_in_scrolled_area ) {
                JScrollPane sp = new JScrollPane( field, 
                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                    JScrollPane.HORIZONTAL_SCROLLBAR_NEVER );
                scrollers.put( field, sp );
                c = sp;
            } else {
                c = field;
            }
            if ( label == null || ! (label instanceof JLabel) ) {
                if ( field instanceof JPanel ) {
                    String name = ((JPanel)field).getName();
                    tabPane.addTab( name, c );
                } else
                    tabPane.addTab( "", c );
            } else
                tabPane.addTab( ((JLabel)label).getText(), c );
            tabPane.setSelectedComponent( c );
            validate();
        } 
    }

    protected void removeFromField(JComponent label, JComponent checkbox, JComponent field)
    {
        if (!isTabbed) {
            super.removeFromField(label, checkbox, field);
        } else if ( declarationInfo.is_in_scrolled_area ) {
            tabPane.remove( field );
        } else {
            tabPane.remove( scrollers.get( field ) );
        }
    }

    public boolean processAttribute(String name, String value)
        throws IOException 
    {
        if (name.equals("showTabbed"))
            setTabbedness( (value.equals("true")) ? true : false );
        else
            return super.processAttribute(name, value);

        return true;
    }

    protected boolean isTabbed = false;
    protected void setTabbedness(boolean tabbed) { isTabbed = tabbed; }

    protected boolean requireOpaque() { return isTabbed; }
    protected void validate() { 
        if (parent != null) parent.validate();
        tabPane.validate(); 
        for ( JScrollPane sp : scrollers.values() )
            sp.validate(); 
        super.validate();
        tabPane.validate(); 
        for ( JComponent comp : scrollers.keySet() ) {
            JScrollPane sp = scrollers.get(comp);
            sp.validate(); 
        }
        if (parent != null) parent.validate();
    }

    protected void declareDescriptedPanels(DeclarationInfo  info) {
        if ( ! isTabbed ) {
            super.declareDescriptedPanels(info);
        } else {
            if ( info != null ) {
                childrenScroll = ! info.is_in_scrolled_area;
                this.declarationInfo = new DeclarationInfo(info);
                declarationInfo.is_in_scrolled_area = true;

                for (ConfigObject o : children)
                    o.declareDescriptedPanels(declarationInfo);
            } else {
                for (ConfigObject o : children)
                    o.declareDescriptedPanels(null);
            }

            if (parent != null && 
                !(transparent && !parent.requireOpaque()) ) 
            {
                if (declarationInfo != null && declaredSubPanel == null) {
                    declaredSubPanel = new SubPanel(null,null,tabPane,false);
                    parent.subPanelChange(null, declaredSubPanel, this, 0);
                } else if (declarationInfo == null && declaredSubPanel != null) {
                    parent.subPanelChange( declaredSubPanel, null, this, 0 );
                }
            }
        }
    }
}

