
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
import java.util.*;

abstract class ConfigEntry extends DescriptedPanel {
	private String help;
	private boolean invalid;
	private boolean viewable;
	private boolean editable;
	private boolean outputOnChange;
        private int userLevel;
        private HashMap<Component,Color> normalBackground
            = new HashMap<Component,Color>();
        private String configSetIdent;

        public String getPrefix() { return "set"; }

	protected JComponent label, field;

	public ConfigEntry() {
                setName("");
                setDesc("");
		this.invalid = false;
		this.viewable = true;
		this.editable = true;
		this.outputOnChange = false;

                updateVisibility();
                updateEnabled();
	}

	public ConfigEntry(String name, String desc) {
		setName(name);
		setDesc(desc);
		this.invalid = false;
		this.viewable = true;
		this.editable = true;
		this.outputOnChange = false;

                updateVisibility();
                updateEnabled();
	}

	public ConfigEntry(String name, String desc, boolean invalid, boolean viewable, boolean editable) {
		setName(name);
		setDesc(desc);
		this.invalid = invalid;
		this.viewable = viewable;
		this.editable = editable;
		this.outputOnChange = false;

                updateVisibility();
                updateEnabled();
	}

        public void setDesc(String desc) {
            String ls = "<html><p>" + desc + ":</p></html>";
            JLabel nlabel = new JLabel(ls, Label.RIGHT);
            setLabel(nlabel);
            super.setDesc(desc);
        }

	public void setField(JComponent component) {
                if (this.field != null)
                    removeFromField( (JComponent)null, this.field );
		this.field = component;
		this.field.addMouseListener(this);
                if (this.field != null)
                    addToField( (JComponent)null,
                                this.field, fieldShouldExpand() );
	}

	public void setLabel(JComponent label) {
		this.label = label;
                if (this.label != null) {
                    label.setFont(label.getFont().deriveFont(Font.PLAIN));
                    this.label.addMouseListener(this);
                }
	}

        public void setHelp(String help) { 
            int width = 60;
            String orig = new String(help), umgebrochen= new String();
            while (orig.length() > 0) {
                if (orig.length() <= 60) {
                    umgebrochen += orig;
                    orig = "";
                } else {
                    int stelle = orig.lastIndexOf(' ', 60);
                    umgebrochen += orig.substring(0,stelle) + "<br/>";
                    orig = orig.substring(stelle+1);
                }
            }
            umgebrochen = "<html>" + umgebrochen + "</html>";
            if (this.label != null)
                label.setToolTipText(umgebrochen);
            if (this.field != null)
                field.setToolTipText(umgebrochen);
        }

        public void setInvalid(boolean isInvalid) {
            this.invalid = isInvalid;
        }

	public boolean isEditable() { return editable; }

        public void setEditable(boolean isEditable) {
            editable = isEditable;
            updateEnabled();
        }

	public boolean isOutputOnChange() { return outputOnChange; }
	public void setOutputOnChange(boolean doOutputOnChange)
            { this.outputOnChange = doOutputOnChange; }

	public boolean isViewable() {
		return viewable && (userLevel <= getRequiredUserLevel());
	}

        public void setViewable(boolean isViewable) {
            viewable = isViewable;
            updateVisibility();
        }

        protected void setUserLevel(int userLevel) {
            this.userLevel = userLevel;
            updateVisibility();
        }

        protected void updateVisibility() {
            if (field != null) field.setVisible( isViewable() );
            if (label != null) label.setVisible( isViewable() );
        }

        protected void updateEnabled() {
            if (field == null) return;

            boolean editable = isEditable();
            LinkedList<Component> myComponents 
                = new LinkedList<Component>();

            myComponents.add( field );
            if (field instanceof Container) {
                Container c = (Container)field;
                Component[] components = c.getComponents();
                for (Component comp : components)
                    myComponents.add( comp );
            }

            for (Component comp : myComponents) {
                if (normalBackground.get(comp) == null)
                    normalBackground.put(comp, comp.getBackground());
                comp.setEnabled( editable );
                /* This code would change the background color of
                 * elements that are disabled. 
                if ( ! editable) {
                    comp.setBackground(Color.white);
                } else
                    comp.setBackground(normalBackground.get(comp));
                */
            }
        }

        public void setRequiredUserLevel(int requiredLevel) {
            super.setRequiredUserLevel(requiredLevel);
            updateVisibility();
        }

        public boolean hasLabel() { return true; }
        public JComponent getLabel() { return label; }
            
        protected String getStringValue() { return ""; }

        public void doPrint() {
            print( "= " + getStringValue() );
        }

	public void readDefinition(BufferedReader input)
            throws IOException 
        {
            while (true) {
                String line = input.readLine();
                if (line.equals("end"))
                    break;
                else 
                    processCommand(line, input);
            }
	}

        public boolean processCommand(String line, BufferedReader stream)
            throws IOException 
        {
            String[] split = line.split(" ", 2);
            String command = split[0];
            String arg = (split.length == 2) ? split[1] : "";

            if (command.equals("desc"))
                setDesc(arg);
            else if (command.equals("help"))
                setHelp(arg);
            else if (command.equals("invalid"))
                setInvalid( Boolean.valueOf(arg) );
            else if (command.equals("viewable"))
                setViewable( Boolean.valueOf(arg) );
            else if (command.equals("editable"))
                setEditable( Boolean.valueOf(arg) );
            else if (command.equals("outputOnChange"))
                setOutputOnChange( Boolean.valueOf(arg) );
            else if (command.equals("userLevel"))
                setUserLevel( Integer.valueOf(arg) );
            else
                return super.processCommand(line, stream);

            return true;
        }

        protected boolean isValue(String str) 
            { return str.equals("value") || str.equals("="); }
}

