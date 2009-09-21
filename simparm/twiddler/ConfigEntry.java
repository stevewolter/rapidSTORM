
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

abstract class ConfigEntry extends ConfigObject 
    implements FocusListener, ActionListener, MouseListener
{
    private String help;
    private boolean invalid;
    private boolean editable;
    private boolean outputOnChange;
    private HashMap<Component,Color> normalBackground =
         new HashMap<Component,Color>();
    private String configSetIdent;
    private int helpID;

    public String getPrefix() { return "set"; }

    public void mousePressed(MouseEvent e) {}
    public void mouseReleased(MouseEvent e) {}
    public void mouseEntered(MouseEvent e) {}
    public void mouseExited(MouseEvent e) {}

    public void mouseClicked(MouseEvent e) {
       if (e.getButton() == MouseEvent.BUTTON3)
          HelpManager.forID( helpID );
    }

    private ConfigObject.SubPanel subpanel = null;
    private JComponent field = null, label = null;
    private boolean fieldMayExpand;

    public void focusGained(FocusEvent e) {} 
    public void focusLost(FocusEvent e) { commitChanges(); } 

    public void actionPerformed(ActionEvent e) { commitChanges(); }

    abstract void commitChanges();

    private void publishFields() {
        SubPanel to_declare;
        if ( !mayDeclare )
            to_declare = null;
        else if ( subpanel != null && subpanel.label == label 
                                   && subpanel.field == field )
            to_declare = subpanel;
        else
            to_declare = new SubPanel(label, field, fieldMayExpand);
        
        if (to_declare != subpanel) {
            SubPanel sp = subpanel;
            subpanel = to_declare;
            parent.subPanelChange( sp, to_declare, this, 0 );
        }
    }

    protected int determine_offset( ConfigObject fromNode, int offset_in_node ) {
        int rv =  super.determine_offset(fromNode, offset_in_node)
            + ( (subpanel != null && subpanel.label != null) ? 1 : 0)
            + ( (subpanel != null && subpanel.field != null) ? 1 : 0);
        return rv;
    }

    protected void declareDescriptedPanels(boolean inserted) {
        mayDeclare = inserted;
        publishFields();
        super.declareDescriptedPanels( inserted );
    }

    protected void setField(JComponent field) { setField(field, false); }
    protected void setField(JComponent field, boolean mayExpand) {
        this.field = field;
        field.addFocusListener(this);
        field.addMouseListener(this);
        updateEnabled();
        updateVisibility();
        publishFields();
    }

    protected void setLabel(JComponent label) {
        if (label != null) {
            label.setFont(label.getFont().deriveFont(Font.PLAIN));
            label.addMouseListener(this);
        }
        this.label = label;
        publishFields();
    }

    public ConfigEntry() {
            setName("");
            setDesc("");
            this.invalid = false;
            this.editable = true;
            this.outputOnChange = false;

            updateVisibility();
            updateEnabled();
    }

    public ConfigEntry(String name, String desc) {
            setName(name);
            setDesc(desc);
            this.invalid = false;
            this.editable = true;
            this.outputOnChange = false;

            updateVisibility();
            updateEnabled();
    }

    public ConfigEntry(String name, String desc, boolean invalid, boolean viewable, boolean editable) {
            setName(name);
            setDesc(desc);
            this.invalid = invalid;
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

    public void setHelp(String help) { 
        if (help == null || help.equals("") || help.equals(" ")) {
            if (this.label != null)
                label.setToolTipText(null);
            if (this.field != null)
                field.setToolTipText(null);
        } else {
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

    protected void updateVisibility() {
        super.updateVisibility();
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

    protected String getStringValue() { return ""; }

    public void doPrint() { doPrint(null); }
    public void doPrint(Acknowledgement ack_to) {
        print( "= " + getStringValue(), ack_to );
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
        else if (command.equals("editable"))
            setEditable( Boolean.valueOf(arg) );
        else if (command.equals("outputOnChange"))
            setOutputOnChange( Boolean.valueOf(arg) );
        else if (command.equals("helpID"))
            helpID = Integer.valueOf(arg);
        else
            return super.processCommand(line, stream);

        return true;
    }

    protected boolean isValue(String str) 
        { return str.equals("value") || str.equals("="); }
}

