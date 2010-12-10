//package simparm.twiddler;

import java.io.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import javax.swing.SwingUtilities;
import javax.swing.JOptionPane;
import javax.swing.JComponent;

class ConfigIO extends DescriptedPanel {
    private BufferedReader in;
    private PrintStream out;
    private boolean attached = false, closing = false;

    public ConfigIO( BufferedReader in, PrintStream out ) 
    {
        this.in = in;
        this.out = out;
    }

    public void detach() {
        if (in != null) { 
            try { in.close(); } catch (IOException e) {}
            in = null; 
        }
        if (out != null) { out.println("detach"); out.close(); out = null;}
    }

    public void print(String message) {
        out.println(message);
        out.flush();
    }

    public void attach() throws IOException {
        out.println("attach");
        out.flush();
        while ( ! attached ) {
            String line = in.readLine();
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
        else 
            return super.processCommand(line, rest);

        return true;

/*
        else if (line.startsWith("remove ")) {
            String[] split = line.split(" ", 2);
            String name = split[1];
            Tab moriturus = sets.get( name );
            sets.remove(name);
            ordered.remove(moriturus);
            if (tabPane != null) 
                tabPane.remove(moriturus.getComponent());
            displayRightElement();
        } else {
            String name = "", command = line;
            if (line.startsWith("forSet ")) {
                String[] split = line.split(" ", 3);
                name = split[1];
                command = split[2];
            }
            Tab newTab = null;
            if ( ! sets.containsKey( name ) ) {
                newTab = new Tab(name, new ConfigSet(name, in, out));
                sets.put( name, newTab );
                ordered.add( newTab );
            }
            sets.get( name ).processCommand(command, rest);
            if (tabPane != null && newTab != null) {
                tabPane.add(newTab.getComponent());
                tabPane.setSelectedIndex( ordered.size() - 1 );
            }
            displayRightElement();
        }
*/
    }

    public void saveConfigToFile(String filename) {
        out.println("save " + filename);
        out.flush();
    }

    public void loadConfigFromFile(String filename) {
        out.println("load " + filename); 
        out.flush();
    }

    private class InputMonitor implements Runnable {
        private String line = null;
        private Lock lock = new ReentrantLock();

        public void monitorInput() throws IOException {
            lock.lock();
            while ( attached && ! closing ) {
                if ( line != null || ! in.ready() ) { 
                    lock.unlock();
                    try { Thread.yield(); Thread.sleep(10); } 
                    catch (InterruptedException e) {
                        return;
                    }
                    lock.lock();
                } else {
                    line = in.readLine();
                    SwingUtilities.invokeLater(this);
                }
            }
            lock.unlock();
        }

        public void run() {
            lock.lock();
            if (line != null) {
                try {
                    processCommand(line, in); 
                } catch (IOException e) {
                    JOptionPane.showMessageDialog
                        (null, e.getMessage(), "Error",
                        JOptionPane.ERROR_MESSAGE);
                }
            }
            line = null;
            lock.unlock();
        }
    };

    public void monitorInput() throws IOException {
        new InputMonitor().monitorInput();
    }

    public void close() { 
        closing = true;
    }

};
