import java.io.*;

final class PipingThread extends Thread {
    private InputStream from;
    private OutputStream to;
    private byte buffer[];

    public PipingThread(InputStream from, OutputStream to) {
        this.from = from; this.to = to;
        buffer = new byte[64];
    }

    public void run() {
        try {
        while (true) {
            int bytes_read = from.read(buffer);
            if (bytes_read > 0)
                to.write(buffer, 0, bytes_read);
            else break;
        }
        } catch(IOException e) {
        }
    }
};
