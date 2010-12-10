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

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;

class Tab implements ConfigSet.Listener {
    private JScrollPane scrollbar;
    private JPanel panel;
    private GridBagLayout gridbag;
    ConfigSet config;
    private String description;

    public Tab(String name, ConfigSet config) throws IOException {
        panel = new JPanel();
        //panel.setPreferredSize(new Dimension(600,400));
        gridbag = new GridBagLayout();
        int[] widths = new int[2];
        widths[0] = 300; widths[1] = 0;
        gridbag.columnWidths = widths;
        panel.setLayout(gridbag);

        this.config = config;
        config.addListener(this);

        panel.setName(name);
        scrollbar = new JScrollPane( panel, 
              JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
              JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED );
    }

    public JComponent getComponent() { return scrollbar; }

    public void notifyOfAddition(ConfigObject src, ConfigObject o) {
        if ( ! (o instanceof ConfigEntry) ) return;
        ConfigEntry e = (ConfigEntry)o;
        GridBagConstraints c = new GridBagConstraints();
        c.fill = GridBagConstraints.HORIZONTAL;
        c.insets = new Insets(2,2,2,20);
        if (e.getLabel() != null) {
            Component l = e.getLabel();
            panel.add( l, c );
        }
        c.gridwidth = GridBagConstraints.REMAINDER;
        c.weightx = 1;
        c.insets = new Insets(2,2,2,2);
        if (e.fieldShouldExpand())
            c.weighty = 1;

        if (e.getField() != null)
            panel.add( e.getField(), c );
        
        panel.validate();
        scrollbar.validate();
    }

    public void notifyOfRemoval(ConfigObject src, ConfigObject o) {
        if ( ! (o instanceof ConfigEntry) ) return;
        ConfigEntry e = (ConfigEntry)o;
        panel.remove( e.getField() );
        if ( e.getLabel() != null )
            panel.remove( e.getLabel() );
    }

    public void notifyOfVisibilityChange(ConfigObject e) {
        panel.validate();
        scrollbar.validate();
    }

    public void notifyOfNewDescription(ConfigObject src, String desc) {
        panel.setName(desc);
        scrollbar.setName(desc);
        panel.validate();
        scrollbar.validate();
        description = desc;
    }

    public void processCommand(String line, BufferedReader rest)
    throws IOException
        { config.processCommand(line, rest); }

    public String getDescription() { return description; }
};

