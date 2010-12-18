class ProcessWatcher {
    private class StreamCopy extends Thread {
        java.io.InputStream in;
        public java.io.PrintStream p;

        public StreamCopy(java.io.InputStream in, java.io.OutputStream out) 
        {
          this.in = in; 
          set(out);
        }

        public synchronized java.io.PrintStream get() {
            return p;
        }
        public synchronized void set( java.io.OutputStream out ) {
            if ( out == null )
                p = null;
            else
                p = new java.io.PrintStream(out);
        }

        public void run() {
          try {
            java.io.BufferedReader r = new java.io.BufferedReader(
                new java.io.InputStreamReader( in ) );
            while ( true ) {
                String s = r.readLine();
                if ( s != null ) {
                    if ( in == System.in && s.equals("START") ) {
                        set_run( true );
                    } else if ( in == System.in && s.equals("STOP") ) {
                        set_run( false );
                    }
                    java.io.PrintStream p = null;
                    while ( p == null ) p = get();
                    p.println( s );
                    p.flush();
                } else 
                    break;
            }
          } catch (Exception e) {
          }
        }
    };

    boolean run = true;
    private synchronized boolean should_run() { return run; }
    private synchronized void set_run(boolean do_run) { run = do_run; }

    private ProcessWatcher(String[] args) {
        StreamCopy c = new StreamCopy(System.in, null);
        c.start();
        System.out.println("PROCESS WATCHER ACTIVE");
        try {
          boolean success = false;
          while (true) 
          {
            while ( ! should_run() ) ;

            Process myProcess = Runtime.getRuntime().exec(args);
            Thread a, b;
            a = (new StreamCopy(myProcess.getInputStream(), System.out));
            a.start();
            b = (new StreamCopy(myProcess.getErrorStream(), System.out));
            b.start();
            c.set( myProcess.getOutputStream() );

            myProcess.waitFor();
            a.join();
            b.join();
            System.err.println("PROCESS TERMINATED WITH EXIT CODE " + myProcess.exitValue());
            c.set( null );
          }
        } catch (Exception e) {
            System.err.println("PROCESS WATCHER TERMINATED: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        new ProcessWatcher(args);
    }
};
