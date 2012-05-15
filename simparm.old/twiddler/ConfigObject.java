package au.com.pulo.kev.simparm;

import java.io.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import javax.swing.tree.*;

class ConfigObject {
    public interface Listener {
        public void notifyOfAddition(ConfigObject parent, 
                                     ConfigObject child);
        public void notifyOfRemoval(ConfigObject parent,
                                    ConfigObject child);
        public void notifyOfVisibilityChange(ConfigObject o);
        public void notifyOfNewDescription(ConfigObject o, String desc);
    }

    protected static class SubPanel {
        public JComponent label, checkbox, field;
        public boolean fieldShouldExpand;
        public int tabOrder = 0;

        public SubPanel(JComponent l, JComponent c, JComponent f, boolean e) {
            label = l; checkbox = c; field = f; fieldShouldExpand = e;
        }
    };

    private String name, desc;
    protected String help_file;
    protected ConfigObject parent = null;
    protected LinkedList<ConfigObject> children
        = new LinkedList<ConfigObject>();
    protected Map<String,ConfigObject> lookup 
        = new HashMap<String,ConfigObject>();
    private java.util.List<Listener> listeners
        = new LinkedList<Listener>();

    private boolean viewable;
    private int userLevel;

    ConfigObject() {
        this.viewable = true;
        GridBagLayout gridbag = new GridBagLayout();
        int[] widths = new int[2];
        widths[0] = 100; widths[1] = 0;
        gridbag.columnWidths = widths;
        panel.setLayout( gridbag );
    }

    protected boolean isDirectlyViewable() { 
        return viewable &&  (userLevel <= getRequiredUserLevel()); 
    }
    protected boolean shouldBeVisible() {
        return isViewable() && isDirectlyViewable();
    }
    private boolean isViewable() {
            return (parent == null || parent.isViewable()) && viewable;
    }

    public void setViewable(boolean isViewable) {
        viewable = isViewable;
        updateVisibility();
    }

    protected void setUserLevel(int userLevel) {
        this.userLevel = userLevel;
        updateVisibility();
    }
    protected int getUserLevel() { return userLevel; }

    protected void updateVisibility() {
        for ( ConfigObject c : children )
            c.updateVisibility();
        panel.setVisible( shouldBeVisible() );
    }

    public String getPrefix() { return "in"; }

    public void addListener(Listener l) { 
        if ( !listeners.contains(l) )
            listeners.add(l); 
    }
    public void removeListener(Listener l) { 
        if ( listeners.contains(l) )
            listeners.remove(l); 
    }
    protected void notifyOfVisibilityChange() {
            Iterator<Listener> l =  listeners.iterator();
            while (l.hasNext()) 
                l.next().notifyOfVisibilityChange(this);
    }
    protected void notifyOfNewDescription(String desc) {
            Iterator<Listener> l = listeners.iterator();
            while (l.hasNext()) 
                l.next().notifyOfNewDescription(this, desc);
    }

    protected void setName(String name) {
            this.name = name;
    }
    public String getName() { return name; }
    protected void setDesc(String desc) {
        this.desc = desc;
        panel.setName(desc);
        notifyOfNewDescription(desc);
        if ( component != null ) 
            component.notifyOfChangedDescription( node );
    }
    public String getDesc() { return desc; }

    public void print(String message) { print(message, null); }
    public void print(String message, Acknowledgement ack) {
        if (parent != null && !message.equals(""))
            parent.print(getPrefix() + " " + name + " " + message,
                                ack);
    }

    public static ConfigObject factory(String type, BufferedReader from)
    throws IOException 
    {
        /* Re-write type to Twiddler types */
        if (type.equals("choice")) type = "Object";
        if (type.endsWith("Entry")) type = "Entry" + type.substring(0, type.length()-5);
        type = "Config" + type;
        if ( type.equals("ConfigEntryUnsignedInt") ) type = "ConfigEntryInteger";
        if ( type.equals("ConfigEntryInt") ) type = "ConfigEntryInteger";
        if ( type.equals("ConfigEntryLong") ) type = "ConfigEntryInteger";
        if ( type.equals("ConfigEntryEntry<unsigned long>") ) type = "ConfigEntryInteger";
        if ( type.equals("ConfigEntryShort") ) type = "ConfigEntryInteger";
        if ( type.equals("ConfigEntryChar") ) type = "ConfigEntryInteger";
        if ( type.equals("ConfigEntryDouble") ) type = "ConfigEntryFloat";
        ConfigObject entry;
        try {
            Package pkg = null;
            try { pkg = ConfigObject.class.getPackage(); } catch(Exception e) {}
            String fqn = ((pkg != null) ? (pkg.getName()+".") : "") + type;
            entry = (ConfigObject) Class.forName(fqn).newInstance();
            entry.readDefinition(from);
            return entry;
        } catch (ClassNotFoundException e) {
            throw new IOException("No config item for class " + e );
        } catch (InstantiationException e) {
            throw new IOException(e.getMessage());
        } catch (IllegalAccessException e) {
            throw new IOException(e.getMessage());
        }
    }

    public void readDefinition(BufferedReader from) 
    throws IOException 
    {
        while ( true ) {
            String line = from.readLine();
            if ( line.equals("end") ) {
                break;
            } else
                processCommand(line, from);
        }
    }

    public void add(ConfigObject entry) {
        entry.setRequiredUserLevel(_userLevel);
        lookup.put(entry.getName(), entry);
        children.add( entry );
        entry.parent = this;
        for ( Listener l : listeners )
            l.notifyOfAddition(this, entry);
        for ( Listener l : entry.listeners )
            l.notifyOfAddition(this, entry);

        if ( declarationInfo != null ) {
            entry.declareDescriptedPanels( declarationInfo );
        }
    }

    public void remove(ConfigObject entry) {
        if ( declarationInfo != null ) {
            entry.declareDescriptedPanels( null );
        }

        lookup.remove(entry.getName());
        Iterator<Listener> l = listeners.iterator();
        while (l.hasNext()) 
            l.next().notifyOfRemoval(this, entry);
    }

    protected SubPanel declaredSubPanel = null;
    protected DeclarationInfo declarationInfo = null;

    protected class DeclarationInfo {
        public JMenu menu;
        public JMenuBar menuBar;
        public boolean is_in_scrolled_area = false;

        DeclarationInfo() {}
        DeclarationInfo(DeclarationInfo i) { menu = i.menu; menuBar = i.menuBar; is_in_scrolled_area = i.is_in_scrolled_area; }
    }

    protected void declareDescriptedPanels(DeclarationInfo info) {
        if ( info != null )
            declarationInfo = new DeclarationInfo(info);
        else
            declarationInfo = null;

        ConfigObject treePa = null;
        if ( isTreeNode ) {
            node.setUserObject( panel );

            for ( treePa = this; treePa != null; treePa = treePa.parent ) {
                if ( treePa.component != null || treePa.isTreeRoot ) {
                    component = treePa.component;
                    break;
                }
            }
            if ( component == null ) {
                component = new TreeComponent( node );
                isTreeRoot = true;
            } else {
                isTreeRoot = false;
            }
        }

        for (ConfigObject o : children)
            o.declareDescriptedPanels(info);

        if ( isTreeNode && !isTreeRoot ) {
            if ( info != null ){
                component.addNode( node,
                    (treePa != this) ? treePa.node : null, focus );
            } else {
                try {
                    component.removeNode( node );
                } catch (IllegalArgumentException e) {}
            }
        } else {
            JComponent myComp = 
                ( isTreeRoot ) ? component.getComponent() : panel;
            if (parent != null &&
                    !(transparent && !parent.requireOpaque()) ) 
            {
                if (info != null && declaredSubPanel == null) {
                    declaredSubPanel = new SubPanel( null, null, myComp, false );
                    parent.subPanelChange(null, declaredSubPanel, this, 0);
                } else if (info == null && declaredSubPanel != null) {
                    parent.subPanelChange( declaredSubPanel, null, this,0);
                }
            }
        }
    }

    protected void setTreeShow( boolean show ) {
        isTreeNode = show;
        if ( isTreeNode ) transparent = false;
    }

    private boolean isTreeNode, isTreeRoot, focus;
    private DefaultMutableTreeNode node = new DefaultMutableTreeNode();
    private TreeComponent component;

    protected boolean processAttribute(String name, String value)
        throws IOException 
    {
        if (name.equals("desc"))
            setDesc(value);
        else if (name.equals("viewable")) {
            setViewable( Boolean.valueOf(value) );
        } else if (name.equals("show_in_tree"))
            setTreeShow( Boolean.valueOf(value) );
        else if (name.equals("force_new_root"))
            isTreeRoot = Boolean.valueOf(value);
        else if (name.equals("focus_immediately"))
            focus = Boolean.valueOf(value);
        else if (name.equals("userLevel"))
            setUserLevel( Integer.valueOf(value) );
        else if (name.equals("help_file"))
            help_file = value;
        else 
            return true;

        return true;
    }

    java.util.TreeSet<String> attributes = new java.util.TreeSet<String>();
    public boolean processCommand(String line, BufferedReader rest)
        throws IOException 
    {
        String[] split = line.split(" ", 2);
        String command = split[0], 
                args = (split.length > 1) ? split[1] : "";
        if (command.equals("name")) {
            setName(args);
        } else if (command.equals("in") || command.equals("set") ) {
            split = args.split(" ", 2);
            if (split.length < 2) 
               throw new IOException("Badly formed argument to in.");

            if ( attributes.contains( split[0] ) )
                return processCommand( split[0] + " " + split[1], rest );
            
            ConfigObject e = lookup.get(split[0]);
            if (e != null) {
                e.processCommand(split[1], rest);
            } else
                throw new IOException("Unable to find node " + split[0]);
        } else if (command.equals("declare")) {
            add( ConfigObject.factory(args, rest) );
        } else if (command.equals("remove")) {
            ConfigObject e = lookup.get(args);
            if (e != null)
                remove(e);
        } else if (command.equals("pulse")) {
                // just ignore heartbeats
        } else if ( command.equals("!at") ) {
            split = args.split(" ", 2);
            return processCommand( split[1], rest );
        } else {
            String[] value_split = line.split(" ", 3);
            if ( value_split.length == 3 && value_split[1].equals("set") ) {
                boolean b = processAttribute(value_split[0], value_split[2]);
                if ( b )
                    attributes.add( value_split[0] );
                return b;
            } else if ( value_split.length == 2 && value_split[1].equals("unset") ) {
                boolean b = processAttribute(value_split[0], null);
                if ( b )
                    attributes.add( value_split[0] );
                return b;
            } else
                throw new IOException("Unknown command " + command);
        }

        return true;
    }

    public java.util.List<ConfigObject> getDisplayableMembers() {
        LinkedList<ConfigObject> result = new LinkedList<ConfigObject>();
        for ( ConfigObject n : children )
            result.addAll( n.getDisplayableMembers() );
        return result;
    }

    private int _userLevel = 10;
    public int getRequiredUserLevel() { return _userLevel; }
    public void setRequiredUserLevel(int level) { 
        _userLevel = level; 
        Iterator<ConfigObject> entries = lookup.values().iterator();
        while (entries.hasNext())
            entries.next().setRequiredUserLevel(level);
    }

    public java.util.List<ConfigObject> getChildren() { return children; }

    public boolean transparent = true;
    protected JPanel panel = new FixedWidthPanel();
    protected Map<ConfigObject, LinkedList<JComponent> > subpanel_contents
        = new HashMap<ConfigObject, LinkedList<JComponent> >();

    protected int determine_offset( ConfigObject fromNode, int offset_in_node ) {
        int offset = offset_in_node;
        for ( ConfigObject o : children ) {
            if ( o == fromNode )
                break;
            else {
                List<JComponent> pre = subpanel_contents.get( o );
                offset += ( (pre != null) ? pre.size() : 0 );
            }
        }
        return offset;
    }
    private List<JComponent> subpanel_contents_initialized(ConfigObject fromNode) {
        LinkedList<JComponent> l = subpanel_contents.get( fromNode );
        if ( l == null ) {
            l = new LinkedList<JComponent>();
            subpanel_contents.put(fromNode, l);
        }
        return l;
    }

    public void subPanelChange( ConfigObject.SubPanel from, 
                                ConfigObject.SubPanel to,
                                ConfigObject fromNode, int offset_in_node) 
    { 
        List<JComponent> l = subpanel_contents_initialized(fromNode);
        int offset = determine_offset( fromNode, offset_in_node );
        if ( to != null && to.label != null )
            l.add( to.label );
        if ( to != null && to.checkbox != null )
            l.add( to.checkbox );
        if ( to != null && to.field != null )
            l.add( to.field );
        if ( from != null && from.label != null )
            l.remove( from.label );
        if ( from != null && from.checkbox != null )
            l.remove( from.checkbox );
        if ( from != null && from.field != null )
            l.remove( from.field );

        if (transparent && parent != null && !parent.requireOpaque()) {
            parent.subPanelChange(from,to, this, offset); 
        } else {
            if ( from != null ) 
                removeFromField( from.label, from.checkbox, from.field );
            if ( to != null )
                addToField( to.label, to.checkbox, to.field, to.fieldShouldExpand, offset );
        }
    }


    protected void addToField(JComponent label, JComponent checkbox, JComponent field,
         boolean shouldExpand, int pos)
    {
        GridBagConstraints c = new GridBagConstraints();
        c.fill = GridBagConstraints.HORIZONTAL;
        c.weightx = 0;
        c.insets = new Insets(2,2,2,20);
        if (label != null) { panel.add( label, c, pos ); pos++; }
        c.fill = GridBagConstraints.HORIZONTAL;
        c.weightx = 0;
        c.insets = new Insets(2,2,2,20);
        if (checkbox != null) { panel.add( checkbox, c, pos ); pos++; }

        if ( field != null ) {
            c.gridwidth = GridBagConstraints.REMAINDER;
            c.weightx = 3;
            c.insets = new Insets(2,2,2,2);
            if (shouldExpand) c.weighty = 1;
            panel.add( field,c, pos );
            if ( field instanceof JPanel ) {
                JPanel addedPanel = (JPanel)field;
                if ( addedPanel.isVisible() )
                    addedPanel.setVisible( 
                        addedPanel.getComponentCount() > 0 );
                if (label != null && label.isVisible())
                    label.setVisible( addedPanel.getComponentCount() > 0 );
            }
        }
        validate();
    }
    protected void removeFromField(JComponent label, JComponent checkbox, JComponent field) {
        if (label != null) panel.remove(label);
        if (checkbox != null) panel.remove(checkbox);
        if (field != null) panel.remove(field);
    }

    protected boolean requireOpaque() { return false; }
    protected void validate() { 
        if (parent != null) parent.validate();
        panel.validate();
        panel.revalidate();
        if ( component != null ) component.validate();
        if (parent != null) parent.validate();
    }

    public void wasShown() {
        for ( ConfigObject c : children ) c.wasShown();
    }
};
