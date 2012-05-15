package au.com.pulo.kev.simparm;
import java.io.*;

final public class PipingThread extends Thread {
    private BufferedReader from;
    private PrintStream to;

    public PipingThread(InputStream from, PrintStream to) {
        this.from = new BufferedReader(new InputStreamReader(from));
        this.to = to;
    }

    public void run() {
        String line;
        while (true) {
            try {
                line = from.readLine();
                if (line != null ) {
                    to.println(line);
                    to.flush();
                } else {
                    break;
                }
            } catch(IOException e) {
                e.printStackTrace();
            }
        }
    }
};
