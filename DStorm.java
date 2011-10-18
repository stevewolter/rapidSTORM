package de.uni_bielefeld.physik.rapidSTORM;

import  java.io.*;
import java.util.Set;
import java.util.Map;
import au.com.pulo.kev.simparm.*;
import javax.swing.JOptionPane;

import java.util.prefs.BackingStoreException;

class DStorm {
    static final String install_dir_key = "install_dir";
    private static File get_install_directory() {
        if ( System.getenv("RAPIDSTORM_DIR") != null )
            return new File(System.getenv("RAPIDSTORM_DIR"));
        /* We cannot use *NodeForPackage here since it autocreates Nodes
         * that are potentially off-limits for our permissions. */
        String name = "/" + DStorm.class.getName().replace(".", "/") + "2";
        final java.util.prefs.Preferences 
            sys = java.util.prefs.Preferences.systemRoot(),
            usr = java.util.prefs.Preferences.userRoot();
        try {
            if ( usr.nodeExists(name) ) {
                return new File(usr.node(name).get( install_dir_key, null ));
            } else if ( sys.nodeExists(name) ) {
                return new File(sys.node(name).get( install_dir_key, null ));
            } else {
                return null;
            }
        } catch (BackingStoreException e) {
            System.err.println("Had exception");
            return null;
        }
    }

   private static String[] build_environment(DefaultPath path) {
        boolean set_magick_path = false;
        Set<Map.Entry<String,String> > env = System.getenv().entrySet();
        String[] result_env = new String[ env.size() + ( (set_magick_path) ? 2 : 1 ) ];
        int i = 0;

        for ( Map.Entry<String,String> entry : env ) {
            result_env[i++] = entry.getKey() + "=" + entry.getValue();
        }
        result_env[i++] = "RAPIDSTORM_PLUGINDIR=" + path.getPluginDir();
        if ( set_magick_path ) {
            result_env[i++] = "MAGICK_CONFIGURE_PATH=" +
                path.getPrefix() 
                    + File.separator + "share" 
                    + File.separator + "GraphicsMagick-1.3.12"
                    + File.separator + "config"
                + File.pathSeparator + 
                path.getPrefix() 
                    + File.separator + "lib" 
                    + File.separator + "GraphicsMagick-1.3.12"
                    + File.separator + "config";
        }
        return result_env;
   }

   public static void main(String[] cmdline_args) throws IOException,Exception {
      try {
        DefaultPath path = new DefaultPath( get_install_directory() );

        File doc_path = path.getDocPath();
        if ( doc_path.exists() ) {
            HelpManager.getSingleton().setManual( new File(doc_path, "rapidstorm.html" ) );
            File doc_data_path = path.getDocDataPath();
            HelpManager.getSingleton().setCHManual( new File(doc_path, "rapidstorm.chm" ), new File(doc_data_path, "alias.h"), new File(doc_data_path, "context.h") );
        }

        String title = null;
        try {
            title = (new BufferedReader(new FileReader(path.getVersionFile()))).readLine();
        } catch (FileNotFoundException e) {
            /* Do nothing and read title from program */
        }

        String[] environment = build_environment( path );
        java.util.Vector<String> args = new java.util.Vector<String>();
        args.add(path.getExecutable().getPath() );
        if ( cmdline_args.length > 0 && cmdline_args[0].equals("--StderrPipe") ) args.add("--StderrPipe");
        if ( cmdline_args.length > 1 && cmdline_args[1].equals("--StdoutPipe") ) args.add("--StdoutPipe");
        args.add("--config");
        args.add( path.getSystemConfigFile().getPath() );
        if ( path.getUserConfigFile().exists() ) {
            args.add("--config");
            args.add( path.getUserConfigFile().getPath() );
        }
        args.add("--Twiddler");

        Main.StartWithEnv(args.toArray( new String[0] ), environment, title);
     } catch (Exception e) {
        try {
            JOptionPane.showMessageDialog
                        (null, e.getMessage(), "Error while starting program",
                        JOptionPane.ERROR_MESSAGE);
        } catch (Exception e2) {
            System.err.println(e.getMessage());
        }
     } finally {
     }
   }
}
