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
        boolean set_magick_path = new File( path.getPrefix(), 
            "share" + File.separator + "GraphicsMagick-1.3.12" ).exists();
        Set<Map.Entry<String,String> > env = System.getenv().entrySet();
        String[] result_env = new String[ env.size() + ( (set_magick_path) ? 1 : 0 ) ];
        int i = 0;

        for ( Map.Entry<String,String> entry : env ) {
            result_env[i++] = entry.getKey() + "=" + entry.getValue();
        }
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

    private static class DefaultsSaver implements au.com.pulo.kev.simparm.Twiddler.DefaultsSaver {
        private File file;
        private java.util.regex.Pattern is_car_line;
        public DefaultsSaver( File default_file ) {
            file = default_file;
            is_car_line = java.util.regex.Pattern.compile( "!at [0-9]+ in Car (.*)" );
        }

        public void save_defaults( java.util.List<String> commands ) {
            try {
                file.getParentFile().mkdirs();
                PrintWriter writer = new PrintWriter(file);
                for ( String s : commands ) {
                    java.util.regex.Matcher m = is_car_line.matcher(s);
                    if ( m.matches() && ! m.group(1).startsWith("in Run in value set ") ) {
                        writer.println(m.group(1));
                    }
                }
                writer.close();
            } catch ( FileNotFoundException err ) {
                JOptionPane.showMessageDialog
                (null, err, "Unable to save defaults",
                JOptionPane.ERROR_MESSAGE);
            }
        }
    }

   public static void main(String[] cmdline_args) throws IOException,Exception {
      try {
        DefaultPath path = new DefaultPath( get_install_directory() );

        File doc_path = path.getDocPath();
        if ( doc_path.exists() ) {
            HelpManager.getSingleton().setManual( new File(doc_path, "rapidstorm.html" ) );
            File doc_data_path = path.getDocDataPath();
            HelpManager.getSingleton().add_URI( "Report a bug", new java.net.URI("https://idefix.biozentrum.uni-wuerzburg.de/bugzilla3/") );
        }

        au.com.pulo.kev.simparm.Twiddler.setDefaultsSaver( new DefaultsSaver( path.getUserConfigFile() ) );

        String title = null;
        try {
            title = (new BufferedReader(new FileReader(path.getVersionFile()))).readLine();
        } catch (FileNotFoundException e) {
            /* Do nothing and read title from program */
        }

        String[] environment = build_environment( path );
        java.util.Vector<String> args = new java.util.Vector<String>();
        boolean gui = true, load_config = true, verbose = false;
        String exec_stem = "dstorm";
        for (int i = 0; i < cmdline_args.length; ++i) {
            if ( cmdline_args[i].equals("--StderrPipe") ) args.add("--StderrPipe");
            if ( cmdline_args[i].equals("--StdoutPipe") ) args.add("--StdoutPipe");
            if ( cmdline_args[i].equals("--no-gui") ) gui = false;
            if ( cmdline_args[i].equals("--no-load-config") ) load_config = false;
            if ( cmdline_args[i].equals("--verbose") ) verbose = true;
            if ( cmdline_args[i].equals("--executable") ) {
                ++i;
                if ( i < cmdline_args.length )
                    exec_stem = cmdline_args[i];
                else
                    throw new IllegalArgumentException("--executable requires an argument");
            }
        }
        args.add(path.getExecutable(exec_stem).getPath() );
        if ( load_config ) {
            args.add("--config");
            args.add( path.getSystemConfigFile().getPath() );
            if ( path.getUserConfigFile().exists() ) {
                args.add("--config");
                args.add( path.getUserConfigFile().getPath() );
            }
        }
        args.add("--Twiddler");

        String[] to_exec = args.toArray( new String[0] );
        if ( gui ) {
            Main.StartWithEnv(to_exec, environment, title);
        } else {
            if ( verbose ) {
                for (int i = 0; i < to_exec.length; ++i)
                    System.err.println("Arg " + i + " is " + to_exec[i]);
                for (int i = 0; i < environment.length; ++i)
                    System.err.println("Environment " + i + " is " + environment[i]);
            }
            Process p = Runtime.getRuntime().exec( to_exec, environment );
            Thread in = new au.com.pulo.kev.simparm.PipingThread( System.in, new PrintStream(p.getOutputStream()) );
            Thread out = new au.com.pulo.kev.simparm.PipingThread( p.getInputStream(), System.out );
            Thread err = new au.com.pulo.kev.simparm.PipingThread( p.getErrorStream(), System.err );
            out.start();
            err.start();
            in.start();
            int rv = p.waitFor();
            out.join();
            err.join();
            System.out.println("RAPIDSTORM EXITED WITH CODE " + rv);
        }
     } catch (Exception e) {
        e.printStackTrace();
        JOptionPane.showMessageDialog
                    (null, e.getMessage(), "Error while starting program",
                    JOptionPane.ERROR_MESSAGE);
     } finally {
     }
   }
}
