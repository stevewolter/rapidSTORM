//package simparm.twiddler;

import java.io.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.*;
import javax.swing.SwingUtilities;
import javax.swing.JOptionPane;
import javax.swing.JComponent;
import javax.swing.JScrollPane;

class ConfigIO extends ConfigSet {
    private BufferedReader in;
    private PrintStream out;
    private boolean attached = false, closing = false;

    private int current_ack_id = 0;
    private HashMap<Integer,Acknowledgement> outstanding_acks = 
        new HashMap<Integer,Acknowledgement>();

    private class HistEl {
        private String command;
        private long time_captured, offset;

        public HistEl(String message, HistEl first) {
            command  = message;
            time_captured = System.currentTimeMillis();
            offset = (first != null ) ? 
                time_captured - first.time_captured : 0;
        }
        public String getLine() {
            return "!at " + Long.toString( offset ) + " " + command;
        }
    }

    private List<HistEl> commandHistory = new LinkedList<HistEl>();

    public JComponent configComponent() 
        { return (isTabbed) ? tabPane : panel; }

    public ConfigIO( BufferedReader in, PrintStream out ) 
    {
        this.in = in;
        this.out = out;
        super.isDecorated = false;
        super.mayDeclare = true;
    }

    public void detach() {
        if (out != null) { 
            attached = false;
            out.println("detach");
            out.close();
            out = null;
        }
    }

    public void print(String message, Acknowledgement ack) {
        commandHistory.add( new HistEl(message ,
                            (commandHistory.isEmpty()) ? null : commandHistory.get(0)) );
        if ( ack == null ) {
            out.println(message);
            out.flush();
        } else {
            message = "cmd " + current_ack_id + " " + message;
            out.println(message);
            out.flush();
            outstanding_acks.put(new Integer(current_ack_id), ack);
            current_ack_id++;
        }
    }

    public void setDesc(String desc) {
        super.setDesc(desc);
    }

    public void attach() throws IOException {
        out.println("attach");
        out.flush();
        while ( ! attached ) {
            String line = in.readLine();
            if (line == null) return;
            processCommand(line, in);
        }
    }

    public boolean processCommand(String line, BufferedReader rest)
    throws IOException 
    {
        if (line.equals("attach")) {
            attached = true;
        } else if (line.equals("detach")) {
            attached = false;
        } else if (line.equals(""))
            ;
        else if (line.startsWith("#"))
            ;
        else if (line.startsWith("ack ")) {
            String[] split = line.split(" ", 2);
            try {
                Integer ackId = new Integer(split[1]);
                outstanding_acks.get(ackId).gotAcknowledgement();
                outstanding_acks.remove(ackId);
            } catch (NumberFormatException e) {
                System.err.println("Invalid ack id: " + split[1]);
            }
        } else 
            return super.processCommand(line, rest);

        return true;
    }

    public void loadConfigFromFile(String filename) {
        try {
            BufferedReader reader
                = new BufferedReader( new FileReader(filename) );
            String line;
            while ( (line = reader.readLine()) != null )
                out.println(line);
            out.flush();
        } catch (IOException err) {
            JOptionPane.showMessageDialog
                (null, err.getMessage(), "Error",
                JOptionPane.ERROR_MESSAGE);
        }
    }

    private class InputMonitor implements Runnable {
        private String command = null;
        private ReentrantLock lock  = new ReentrantLock();
        private java.util.concurrent.locks.Condition string_processed
            = lock.newCondition();

        public void monitorInput() throws IOException {
            while ( attached && ! closing ) {
                String line = in.readLine();
                if ( line.equals("quit") )
                    break;
                else {
                    lock.lock();
                    command = line;
                    SwingUtilities.invokeLater(this);
                    string_processed.awaitUninterruptibly();
                    command = null;
                    lock.unlock();
                }
            }
            in.close();
        }

        public void run() {
            String line = command;
            try {
                processCommand(line, in); 
            } catch (IOException e) {
                JOptionPane.showMessageDialog
                    (null, e.getMessage(), "Error",
                    JOptionPane.ERROR_MESSAGE);
            }
            validate();

            lock.lock();
            string_processed.signal();
            lock.unlock();
        }
    };

    public void monitorInput() throws IOException {
        new InputMonitor().monitorInput();
    }

    public void close() { 
        //closing = true;
        out.println("quit");
        out.flush();
        out.close(); 
        out = null;
    }

    /** @return The current position in the command history. */
    public int getCommandHistoryPosition() {
      return commandHistory.size();
    }
    /** @return a list with all the commands sent since
      * getCommandHistoryPosition() returned @c from. */
    public List<String> getCommandHistory(int from) {
        List<String> result = new LinkedList<String>();
        for (HistEl e : commandHistory )
            result.add( e.getLine());
        return result;
    }
};
