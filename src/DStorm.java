package de.uni_bielefeld.physik.rapidSTORM;

import  java.io.*;
import java.util.Set;
import java.util.Map;
import au.com.pulo.kev.simparm.*;
import javax.swing.JOptionPane;

import java.util.prefs.BackingStoreException;

class DStorm {
    static String install_dir_key = "install_dir";
    private static String get_install_directory() {
        if ( System.getenv("RAPIDSTORM_DIR") != null )
            return System.getenv("RAPIDSTORM_DIR");
        /* We cannot use *NodeForPackage here since it autocreates Nodes
         * that are potentially off-limits for our permissions. */
        String name = "/" + DStorm.class.getName().replace(".", "/");
        final java.util.prefs.Preferences 
            sys = java.util.prefs.Preferences.systemRoot(),
            usr = java.util.prefs.Preferences.userRoot();
        try {
            if ( usr.nodeExists(name) ) {
                System.err.println("Using usr node");
                return usr.node(name).get( install_dir_key, null );
            } else if ( sys.nodeExists(name) ) {
                System.err.println("Using sys node");
                return sys.node(name).get( install_dir_key, null );
            } else {
                System.err.println("Using no node");
                return null;
            }
        } catch (BackingStoreException e) {
            System.err.println("Had exception");
            return null;
        }
    }

    private static void set_install_directory(String value, boolean system){
        String name = "/" + DStorm.class.getName().replace(".", "/");
        final java.util.prefs.Preferences
            root = ((system) ? java.util.prefs.Preferences.systemRoot()
                             : java.util.prefs.Preferences.userRoot() ),
            p = root.node(name);
        p.put( install_dir_key, value );
        System.out.println("Set installation directory to " + value);
        
    }

    private static File extract_to_temp( String name) throws Exception
    {
        String[] splitname = name.split("\\.");
        if (splitname.length == 0) {
            splitname = new String[2];
            splitname[0] = name;
            splitname[1] = null;
        } else {
        }
        try {
            File tempFile = File.createTempFile
                (splitname[0], "." + splitname[splitname.length-1]);
            return extract_jar_file(name, tempFile);    
        } catch (java.io.IOException e) {
            throw new Exception("Error in extracting program archive:" +
                               e.getMessage());
        }
    }

    private static File extract_with_exact_name(File toDir, String name) {
        String[] split = name.split( "/" );
        String sep = String.valueOf(File.separatorChar);
        String winName = "";
        for ( int i = 0; i < split.length; i++ )
            winName = winName + ((i==0) ? "" : sep) + split[i];
        File tempFile = new File(toDir, winName);
        try {
            return extract_jar_file(name,tempFile);
        } catch (java.io.IOException e)  {
            /* File probably already present. Mark as temp anyway
             * so _someone_ will delete them. */
            //tempFile.deleteOnExit();
            return tempFile;
        }
    }

    private static File extract_jar_file(String name, File tempFile)
        throws IOException
    {
        java.net.URL resourceURL = Main.class.getResource("/" + name);
        if ( resourceURL == null ) return null;
        Object resource = resourceURL.getContent();
        InputStream source = (InputStream)resource;
        if ( source == null )
            throw new IOException("Resource file " + name + " not found");
        if ( tempFile.getParent() != null )
            tempFile.getParentFile().mkdirs();
        FileOutputStream target = new FileOutputStream(tempFile);
        byte[] buffer = new byte[32767];
        while ( true ) {
            int read = source.read(buffer);
            if (read == -1 ) break;
            target.write(buffer, 0, read);
        }
        source.close();
        target.close();

        //tempFile.deleteOnExit();

        return tempFile;
   }

   private static String[] build_environment(File tempdir) {
        Set<Map.Entry<String,String> > env = System.getenv().entrySet();
        String[] result_env = new String[ env.size() + 2 ];
        int i = 0;

        for ( Map.Entry<String,String> entry : env ) {
            result_env[i++] = entry.getKey() + "=" + entry.getValue();
        }
        result_env[i++] = "RAPIDSTORM_PLUGINDIR=" +
            tempdir.getPath() + File.separator + "lib" + File.separator + "rapidstorm"
            + File.separator + "plugins";
        result_env[i++] = "MAGICK_CONFIGURE_PATH=" +
            tempdir.getPath() 
                + File.separator + "share" 
                + File.separator + "GraphicsMagick-1.3.6"
                + File.separator + "config"
              + File.pathSeparator + 
              tempdir.getPath() 
                + File.separator + "lib" 
                + File.separator + "GraphicsMagick-1.3.6"
                + File.separator + "config";
        System.err.println("Using magick path " + result_env[i-1]);
        return result_env;
   }

   private static void extract_dll_list(File tempDir) 
    throws Exception, IOException 
   {
        InputStream dll_list = Main.class.getResourceAsStream("/dll_list");
        if ( dll_list == null )
            throw new Exception("jar does not contain executable files");
        BufferedReader reader = new BufferedReader
            ( new InputStreamReader(dll_list) );
        String line;
        while ( (line = reader.readLine()) != null ) {
            extract_with_exact_name(tempDir, line);
        }
   }

   public static void main(String[] args) throws IOException,Exception {
      try {
        if ( args.length > 0 && args[0].equals("--set-install-dir-key-system") ) {
            set_install_directory(args[1], true);
            return;
        } else if ( args.length > 0 && args[0].equals("--set-install-dir-key-user") ) {
            set_install_directory(args[1], false);
            return;
        }

        String install_dir = get_install_directory();
        File base_dir;
        if ( install_dir == null ) {
            File tempDir = new File( System.getProperty("java.io.tmpdir") );
            base_dir = new File(tempDir, "rapidSTORM");

            extract_dll_list(base_dir);
        } else {
            base_dir = new File( install_dir );
        }

        File executable = new File(base_dir, "bin" + File.separator + "dstorm");
        File config = new File(base_dir, "share" + File.separator + "rapidstorm" 
                                         + File.separator + "dstorm-config.txt");

        HelpManager.setHelpDirectory(
            new File(base_dir, "share" + File.separator + "doc" ) );

        String[] environment = build_environment( base_dir );
        boolean have_arg = args.length > 0,stderrPipe = (args.length > 0 && args[0].equals("--StderrPipe"));
        int argC = 4;
        if ( have_arg ) argC += 2;
        if ( stderrPipe ) argC += 1;
        try {
            executable.setExecutable(true);
        } catch (SecurityException e) {}
        int lastArg = 0;
        String[] cmd = new String[argC];
        if ( stderrPipe ) cmd[lastArg++] = "--StderrPipe";
        //cmd[lastArg++] = "RAPIDSTORM_PLUGINDIR=foo";
        cmd[lastArg++] = executable.getPath();
        cmd[lastArg++] = "--config";
        cmd[lastArg++] = config.getPath();
        cmd[lastArg++] = "--Twiddler";
        if ( have_arg ) {
            cmd[lastArg++] = "--inputFile";
            cmd[lastArg++] = args[0];
        }
        Main.StartWithEnv(cmd, environment);
     } catch (Exception e) {
        try {
            JOptionPane.showMessageDialog
                        (null, e.getMessage(), "Error while starting program",
                        JOptionPane.ERROR_MESSAGE);
        } catch (Exception e2) {
            System.err.println(e.getMessage());
        }
     }
   }
}
