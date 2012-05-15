package au.com.pulo.kev.simparm;
import javax.swing.JOptionPane;
import java.io.IOException;
import java.io.File;

import java.awt.Desktop;
import java.util.Map;
import java.util.HashMap;
import java.io.BufferedReader;
import java.io.FileReader;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import javax.swing.JMenuItem;

public class HelpManager {
    private static HelpManager singleton = new HelpManager();
    private File manual = null, chm_manual = null;
    private Map<String,Integer> help_ids = new HashMap<String,Integer>();
    private static class HelpURI implements java.awt.event.ActionListener {
        private String name;
        private java.net.URI uri;
        public HelpURI( String name, java.net.URI uri ) 
            { this.name = name; this.uri = uri; }
        public void actionPerformed(java.awt.event.ActionEvent e) {
              try { Desktop.getDesktop().browse( uri ); }
              catch (java.io.IOException exc) { System.err.println(exc.getMessage()); }
        }
        public JMenuItem getMenuItem() {
            JMenuItem i = new JMenuItem( name );
            i.addActionListener(this);
            return i;
        }
    };
    private java.util.List<HelpURI> uris = new java.util.Vector<HelpURI>();

    public static HelpManager getSingleton() { 
        return singleton;
    }
    public void add_URI( String name, java.net.URI uri ) 
        { uris.add( new HelpURI(name, uri) ); }
    public JMenuItem[] getExtraURIs() {
        JMenuItem[] rv = new JMenuItem[ uris.size() ];
        int j = 0;
        java.util.Iterator<HelpURI> i = uris.iterator(); 
        while ( i.hasNext() ) {
            rv[j++] = i.next().getMenuItem();
        }
        return rv;
    }

    public void openManual() { openManual(""); }
    public void openManual(String section) {
        if ( manual == null )
            JOptionPane.showMessageDialog(null,
              "No manual is available.");
            
        if ( section.startsWith("#") )
            section = section.substring(1);

        try {
            if ( chm_manual != null ) {
                Integer help_id = help_ids.get(section);
                String[] args;
                if ( help_id != null ) {
                    String[] tmp = { "hh", "-mapid", help_id.toString(), chm_manual.getPath() };
                    args = tmp;
                } else {
                    String[] tmp = { "hh", chm_manual.getPath() };
                    args = tmp;
                }
                try {
                    Runtime.getRuntime().exec( args );
                    return;
                } catch (IOException e) { /* Ignore the exception and try to open with xchm */ }
                args[0] = "xchm";
                if ( args.length > 2 )
                    args[1] = "-c";
                try {
                    Runtime.getRuntime().exec( args );
                    return;
                } catch (IOException e) { /* Ignore the exception and try to open with browser */ }
            }

            java.net.URI manlink =  new java.net.URI(manual.toURI() + "#" + section);
            Desktop.getDesktop().browse(manlink);
        } catch (Exception e) {
                JOptionPane.showMessageDialog(null,
                "Opening manual failed: " + e.getMessage());
        }
    }

    public void forID( String id ) {
        if ( id.equals("") )
            JOptionPane.showMessageDialog(null,
              "No help is available for this item.");
        openManual( id );
    }

    public void setManual(File file) {
        manual = file;
    }

    private void read_context_table(File context) {
        try {
            BufferedReader alias = new BufferedReader(new FileReader(context));
            String line;
            Pattern p = Pattern.compile("^#define HELP_([^\t]*)\t([0-9]*)$");
            while ( (line = alias.readLine()) != null ) {
                Matcher m = p.matcher(line);
                if ( m.matches() ) {
                    help_ids.put( m.group(1), Integer.parseInt( m.group(2) ) );
                }
            }
        } catch (Exception e) {
            System.err.println("Unable to read context table: " + e.getMessage());
        }
    }

    public void setCHManual(File chm, File alias, File context) {
        read_context_table(context);
        chm_manual = chm;
    }
}
