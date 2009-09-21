
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
        public JComponent label;
        public JComponent field;
        public boolean fieldShouldExpand;
        public int tabOrder = 0;

        public SubPanel(JComponent l, JComponent f, boolean e) {
            label = l; field = f; fieldShouldExpand = e;
        }
    };

    private String name, desc;
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

    public boolean isViewable() {
            return (parent == null || parent.isViewable()) &&
                   viewable && (userLevel <= getRequiredUserLevel());
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
        panel.setVisible( isViewable() );
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
        if ( type.equals("ConfigEntryInt") ) type = "ConfigEntryLong";
        if ( type.equals("ConfigEntryShort") ) type = "ConfigEntryShort";
        if ( type.equals("ConfigEntryChar") ) type = "ConfigEntryChar";
        ConfigObject entry;
        try {
            entry = (ConfigObject) Class.forName(type).newInstance();
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

        if ( mayDeclare )
            entry.declareDescriptedPanels( true );
    }

    public void remove(ConfigObject entry) {
        if ( mayDeclare )
            entry.declareDescriptedPanels( false );

        lookup.remove(entry.getName());
        Iterator<Listener> l = listeners.iterator();
        while (l.hasNext()) 
            l.next().notifyOfRemoval(this, entry);
    }

    protected SubPanel declaredSubPanel = null;
    protected boolean mayDeclare = false;
    protected void declareDescriptedPanels(boolean inserted) {
        mayDeclare = inserted;

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
            o.declareDescriptedPanels(inserted);

        if ( isTreeNode && !isTreeRoot ) {
            if ( inserted ){
                component.addNode( node,
                    (treePa != this) ? treePa.node : null, focus );
            } else if (!inserted) {
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
                if (inserted && declaredSubPanel == null) {
                    declaredSubPanel = new SubPanel( null, myComp, false );
                    parent.subPanelChange(null, declaredSubPanel, this, 0);
                } else if (!inserted && declaredSubPanel != null) {
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

    public boolean processCommand(String line, BufferedReader rest)
        throws IOException 
    {
        String[] split = line.split(" ", 2);
        String command = split[0], 
                args = (split.length > 1) ? split[1] : "";

        if (command.equals("name"))
            setName(args);
        else if (command.equals("desc"))
            setDesc(args);
        else if (command.equals("viewable"))
            setViewable( Boolean.valueOf(args) );
        else if (command.equals("show_in_tree"))
            setTreeShow( Boolean.valueOf(args) );
        else if (command.equals("force_new_root"))
            isTreeRoot = Boolean.valueOf(args);
        else if (command.equals("focus_immediately"))
            focus = Boolean.valueOf(args);
        else if (command.equals("viewable"))
            setViewable( Boolean.valueOf(args) );
        else if (command.equals("userLevel"))
            setUserLevel( Integer.valueOf(args) );
        else if (command.equals("declare")) {
            add( ConfigObject.factory(args, rest) );
        } else if (command.equals("remove")) {
            ConfigObject e = lookup.get(args);
            if (e != null)
                remove(e);
        } else if (command.equals("set") || command.equals("forSet") ||
                   command.equals("in")) {
            split = args.split(" ", 2);
            if (split.length < 2) 
                throw new IOException("Badly formed argument to set.");

            ConfigObject e = lookup.get(split[0]);
            if (e != null) {
                e.processCommand(split[1], rest);
            } else
                System.err.println("Could not find config entry " + 
                    split[0] + " in object " + getName());
        } else if (command.equals("pulse")) {
                // just ignore heartbeats
        } else {
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
    protected JPanel panel = new JPanel();
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
        if ( to != null && to.field != null )
            l.add( to.field );
        if ( from != null && from.label != null )
            l.remove( from.label );
        if ( from != null && from.field != null )
            l.remove( from.field );

        if (transparent && parent != null && !parent.requireOpaque()) {
            parent.subPanelChange(from,to, this, offset); 
        } else {
            if ( from != null ) 
                removeFromField( from.label, from.field );
            if ( to != null )
                addToField( to.label, to.field, to.fieldShouldExpand, offset );
        }
    }


    protected void addToField(JComponent label, JComponent field,
         boolean shouldExpand, int pos)
    {
        GridBagConstraints c = new GridBagConstraints();
        c.fill = GridBagConstraints.HORIZONTAL;
        c.insets = new Insets(2,2,2,20);
        if (label != null) { panel.add( label, c, pos ); pos++; }
        c.gridwidth = GridBagConstraints.REMAINDER;
        c.weightx = 3;
        c.insets = new Insets(2,2,2,2);
        if (shouldExpand) c.weighty = 1;

        if ( field != null ) {
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
    protected void removeFromField(JComponent label, JComponent field) {
        if (label != null) panel.remove(label);
        if (field != null) panel.remove(field);
    }

    protected boolean requireOpaque() { return false; }
    protected void validate() { 
        panel.validate();
        if (parent != null) parent.validate();
    }

    public void wasShown() {
        for ( ConfigObject c : children ) c.wasShown();
    }
};
