package au.com.pulo.kev.simparm;

import java.io.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.*;
import javax.swing.SwingUtilities;
import javax.swing.JOptionPane;
import javax.swing.JComponent;
import javax.swing.JScrollPane;
import javax.swing.JMenuBar;

class ConfigIO extends ConfigSet {
    private BufferedReader in;
    private PrintStream out;
    private boolean attached = false, closing = false, echo_to_stdout = false;

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

    public ConfigIO( BufferedReader in, PrintStream out, boolean echo_to_stdout, JMenuBar menu  ) 
    {
        this.in = in;
        this.out = out;
        this.echo_to_stdout = echo_to_stdout;
        super.isDecorated = false;
        super.declarationInfo = new DeclarationInfo();
        super.declarationInfo.menuBar = menu;

        setDesc("Child process");
    }

    public void detach() {
        if (out != null) { 
            attached = false;
            out.println("detach");
            out.close();
            out = null;
        }
    }

    private void print_acknowledged( String message, Acknowledgement ack ) {
            message = "cmd " + current_ack_id + " " + message;
            out.println(message);
            out.flush();
            outstanding_acks.put(new Integer(current_ack_id), ack);
            current_ack_id++;
    }
    public void print(String message, Acknowledgement ack) {
        commandHistory.add( new HistEl(message ,
                            (commandHistory.isEmpty()) ? null : commandHistory.get(0)) );
        if ( ack == null ) {
            if ( out != null ) {
                out.println(message);
                out.flush();
            }
        } else {
            print_acknowledged( message, ack );
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
            if (line == null) 
                throw new IOException(getDesc() + " terminated before it "
                                + "described all config elements");
            processCommand(line, in);
        }
    }

    public boolean processCommand(String line, BufferedReader rest)
    throws IOException 
    {
        if (line.equals("attach")) {
            attached = true;
        } else if (line.equals("clear")) {
            if ( attached ) {
                recursiveDisable( configComponent() );
                out.println("attach");
            }
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
        } else  {
            boolean rv = super.processCommand(line, rest);
            return rv;
        }

        return true;
    }

    private class LineAcknowledgement extends Acknowledgement {
        private boolean got_ack = false;
        public synchronized void gotAcknowledgement() { 
            got_ack = true; 
            notify(); 
        }
        public synchronized void waitForAcknowledgement() { 
            while ( ! got_ack ) 
                try {
                    wait(); 
                } catch ( InterruptedException e ) {}
        }
        public synchronized boolean acknowledged() { return got_ack; }
    }

    public void loadConfigFromFile(String filename) {
        try {
            BufferedReader reader
                = new BufferedReader( new FileReader(filename) );
            String line;
            while ( (line = reader.readLine()) != null ) {
                String[] split = line.split(" ", 3);
                if ( split[0].equals("!at") )
                    line = split[2];
                if ( line.startsWith("in Car ") && ! line.startsWith("in Car in Run ") && ! line.startsWith("in Car set Run") ) {
                    LineAcknowledgement ack = new LineAcknowledgement();
                    print_acknowledged( line, ack );
                    while ( ! ack.acknowledged() ) {
                        String l = in.readLine();
                        if ( l == null || l.equals("quit") )
                            break;
                        else
                            processCommand( l, in );
                    }
                    processCommand( line, in );
                }
            }
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
                if ( line != null && echo_to_stdout )
                    System.out.println( line );
                if ( line == null || line.equals("quit") )
                    break;
                else {
                    lock.lock();
                    command = line;
                    SwingUtilities.invokeLater(this);
                    while ( command != null )
                        string_processed.awaitUninterruptibly();
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
            } finally {
                validate();

                lock.lock();
                command = null;
                string_processed.signal();
                lock.unlock();
            }
        }
    };

    public void monitorInput() throws IOException {
        new InputMonitor().monitorInput();
    }

    private void recursiveDisable(java.awt.Component c) {
        if ( c != null && c instanceof java.awt.Container ) {
            java.awt.Component[] children =
                ((java.awt.Container)c).getComponents();
            for (int i= 0; i < children.length; i++)
                recursiveDisable(children[i]);
        }
        if ( c != null && c instanceof javax.swing.JComponent
                       && !( c instanceof javax.swing.JTabbedPane)
                       && !( c instanceof javax.swing.JTree) ) {
            ((javax.swing.JComponent)c).setEnabled(false);
        }
    }
    public void close() { 
        recursiveDisable( configComponent() );
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
