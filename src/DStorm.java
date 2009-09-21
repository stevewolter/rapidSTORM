import  java.io.*;

class DStorm {
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
            String tempDir = System.getProperty("java.io.tmpdir");
            File tempFile = new File(tempDir, name);
        try {
            return extract_jar_file(name,tempFile);
        } catch (java.io.IOException e)  {
            /* File probably already present. Mark as temp anyway
             * so _someone_ will delete them. */
            tempFile.deleteOnExit();
            return tempFile;
        }
    }

    private static File extract_jar_file(String name, File tempFile)
        throws IOException
    {
        Object resource = Main.class.getResource(name).getContent();
        InputStream source = (InputStream)resource;
        FileOutputStream target = new FileOutputStream(tempFile);
        byte[] buffer = new byte[32767];
        while ( true ) {
            int read = source.read(buffer);
            if (read == -1 ) break;
            target.write(buffer, 0, read);
        }
        source.close();
        target.close();

        tempFile.deleteOnExit();

        return tempFile;
   }
   public static void main(String[] args) throws IOException {
        File executable = extract_to_temp("dstorm.exe");
        extract_with_exact_name("ATMCD32D.DLL");
        extract_with_exact_name("pthreadGCE2.dll");
        extract_with_exact_name("mingwm10.dll");
        File config = extract_to_temp("dstorm-config.txt");
        File help = extract_to_temp("rapidSTORM.chm");

        HelpManager.setManualFile(help);

        boolean have_arg = args.length > 0,stderrPipe = false;
        int argC = 4, twiddlerArgs = 0;
        if ( have_arg ) argC += 2;
        if ( stderrPipe ) { argC += 1; twiddlerArgs += 1; }
        try {
            executable.setExecutable(true);
        } catch (SecurityException e) {}
        String[] cmd = new String[argC];
        if ( stderrPipe ) cmd[0] = "--StderrPipe";
        cmd[0+twiddlerArgs] = executable.getPath();
        cmd[1+twiddlerArgs] = "--config";
        cmd[2+twiddlerArgs] = config.getPath();
        cmd[3+twiddlerArgs] = "--Twiddler";
        if ( have_arg ) {
            cmd[4+twiddlerArgs] = "--inputFile";
            cmd[5+twiddlerArgs] = args[0];
        }
        Main.main(cmd);
   }
}
