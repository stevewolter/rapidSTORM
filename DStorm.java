import  java.io.*;
import java.util.Set;
import java.util.Map;

class DStorm {
    private static String tempDir;
    private static File extract_to_temp( String name) 
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
            System.err.println("Error in extracting program archive:" +
                               e.getMessage());
            System.exit(1);
        }
        return null;
    }

    private static File extract_with_exact_name(String name) {
        String[] split = name.split( "/" );
        String sep = String.valueOf(File.separatorChar);
        String winName = "";
        for ( int i = 0; i < split.length; i++ )
            winName = winName + ((i==0) ? "" : sep) + split[i];
        File tempFile = new File(tempDir, winName);
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
        java.net.URL resourceURL = Main.class.getResource(name);
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
            tempdir.getPath() + File.separator + "lib" + File.separator + "rapidStorm"
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
        return result_env;
   }

   public static void main(String[] args) throws IOException {
        tempDir = System.getProperty("java.io.tmpdir");
        tempDir = tempDir + String.valueOf( File.separatorChar )
                  + "rapidSTORM";

        InputStream dll_list = Main.class.getResourceAsStream("dll_list");
        BufferedReader reader = new BufferedReader
            ( new InputStreamReader(dll_list) );
        String line;
        while ( (line = reader.readLine()) != null ) {
            extract_with_exact_name(line);
        }

        File executable = extract_with_exact_name("bin/dstorm.exe");
        File config = extract_with_exact_name("share/rapidstorm/dstorm-config.txt");
        File help = extract_with_exact_name("share/rapidstorm/rapidSTORM.chm");

        if ( help != null )
            HelpManager.setManualFile(help);

        String[] environment = 
            build_environment(new File(tempDir) );
        boolean have_arg = args.length > 0,stderrPipe = true;
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
   }
}
