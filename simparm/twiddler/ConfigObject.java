
import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;

abstract class ConfigObject {
    public interface Listener {
        public void notifyOfAddition(ConfigObject parent, 
                                     ConfigObject child);
        public void notifyOfRemoval(ConfigObject parent,
                                    ConfigObject child);
        public void notifyOfVisibilityChange(ConfigObject o);
        public void notifyOfNewDescription(ConfigObject o, String desc);
    }

    private String name, desc;
    protected LinkedList<ConfigObject> parents
        = new LinkedList<ConfigObject>();
    protected LinkedList<ConfigObject> children
        = new LinkedList<ConfigObject>();
    protected Map<String,ConfigObject> lookup 
        = new HashMap<String,ConfigObject>();
    private java.util.List<Listener> listeners
        = new LinkedList<Listener>();

    public String getPrefix() { return "in"; }

    public void addListener(Listener l) { listeners.add(l); }
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
            notifyOfNewDescription(desc);
    }
    public String getDesc() { return desc; }

    public void print(String message) {
        if (!message.equals(""))
            for ( ConfigObject p : parents )
                p.print(getPrefix() + " " + name + " " + message );
    }

    public static ConfigObject factory(String type, BufferedReader from)
    throws IOException 
    {
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
            if ( line.equals("end") )
                break;
            else
                processCommand(line, from);
        }
    }

    public void add(ConfigObject entry) {
        lookup.put(entry.getName(), entry);
        children.add( entry );
        entry.parents.add( this );
        Iterator<Listener> l =  listeners.iterator();
        while (l.hasNext()) 
            l.next().notifyOfAddition(this, entry);
    }

    public void remove(ConfigObject entry) {
        lookup.remove(entry.getName());
        Iterator<Listener> l = listeners.iterator();
        while (l.hasNext()) 
            l.next().notifyOfRemoval(this, entry);
    }

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
        else if (command.equals("declare")) {
            add( ConfigObject.factory(args, rest) );
        } else if (command.equals("remove")) {
            ConfigObject e = lookup.get(args);
            if (e != null)
                remove(e);
            else
                throw new IOException
                    ("Read undeclared entry " + args);
        } else if (command.equals("set")) {
            split = args.split(" ", 2);
            if (split.length < 2) 
                throw new IOException("Badly formed argument to set.");

            ConfigObject e = lookup.get(split[0]);
            if (e != null) {
                e.processCommand(split[1], rest);
            } else
                System.err.println("Could not find config entry " + 
                    split[0]);
        } else if (command.equals("pulse")) {
                // just ignore heartbeats
        } else {
            return false;
        }

        return true;
    }

    protected boolean hasLabel() { return false; }
    protected JComponent getLabel() { return null; }
    protected abstract JComponent getField();
    public boolean fieldShouldExpand() { return false; }
    public void setSelfDescribing(boolean is) {}

    private int _userLevel = 10;
    public int getRequiredUserLevel() { return _userLevel; }
    public void setRequiredUserLevel(int level) { 
        _userLevel = level; 
        Iterator<ConfigObject> entries = lookup.values().iterator();
        while (entries.hasNext())
            entries.next().setRequiredUserLevel(level);
    }
};
